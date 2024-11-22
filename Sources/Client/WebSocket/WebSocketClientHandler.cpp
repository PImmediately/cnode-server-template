#include "./WebSocketClientHandler.h"

#include <emscripten.h>
#include "./../../Shared/WebSocket/Binary.h"
#include "./../../Shared/WebSocket/Packet.h"

#include <iostream>
#include <chrono>

enum kWebSocketEvent {
	Open = 1,
	Close,
	Error,
	Message
};

std::vector<WebSocketClientHandler*> WebSocketClientHandler::_s_Instances;

WebSocketClientHandler::WebSocketClientHandler(const char* server_ip) {
	this->_s_Instances.push_back(this);

	this->m_uIndex = EM_ASM_INT({
		const socket = new WebSocket(UTF8ToString($0));
		socket.binaryType = "arraybuffer";

		socket.events = new Array();			
		socket.onopen = (event) => {
			socket.events.push([1/*kWebSocketEvent::Open*/, undefined, undefined]);
			Module["__WebSocketClientHandler_CheckEvent"]();
		};
		socket.onclose = (event) => {
			socket.events.push([2/*kWebSocketEvent::Close*/, undefined, undefined]);
			Module["__WebSocketClientHandler_CheckEvent"]();
		};
		socket.onerror = (event) => {
			socket.events.push([3/*kWebSocketEvent::Error*/, undefined, undefined]);
			Module["__WebSocketClientHandler_CheckEvent"]();
		};
		socket.onmessage = (event) => {
			const buffer = new Uint8Array(event.data);
			const pointer = _malloc(buffer.length);
			Module["HEAPU8"].set(buffer, pointer);
			socket.events.push([4/*kWebSocketEvent::Message*/, pointer, buffer.byteLength]);
			Module["__WebSocketClientHandler_CheckEvent"]();
		};

		for (let i = 0; i < sockets.length; i++) {
			if (!sockets[i]) {
				sockets[i] = socket;
				return i;
			}
		}
		sockets.push(socket);
		return (sockets.length - 1);
	}, server_ip);
}

WebSocketClientHandler::~WebSocketClientHandler() {
	this->_s_Instances.erase(
		std::remove(this->_s_Instances.begin(), this->_s_Instances.end(), this),
		this->_s_Instances.end()
	);

	EM_ASM({
		const socket = sockets[$0];
		socket.onopen = socket.onerror = socket.onclose = socket.onmessage = () => {};
		socket.events = null;

		try {
			socket.close();
		} catch (error) {
		}
		sockets[$0] = null;
	}, this->GetIndex());
}

void WebSocketClientHandler::_PopEvent() {

	uint32_t pointer = NULL;
	size_t length = NULL;

	kWebSocketEvent event = static_cast<kWebSocketEvent>(EM_ASM_INT({
		const socket = sockets[$0];
		if (socket.events.length === 0) {
			return 0;
		}
		const event = socket.events.shift();
		Module.HEAPU32[$1 >> 2] = (event[1] || 0);
		Module.HEAPU32[$2 >> 2] = (event[2] || 0);
		return event[0];
	}, this->GetIndex(), &pointer, &length));

	if (event == kWebSocketEvent::Open) {
		this->m_fnOnConnect(this);
	} else if (event == kWebSocketEvent::Close) {
		this->m_fnOnDisconnect(this);
	} else if (event == kWebSocketEvent::Error) {
		this->m_fnOnError(this);
	} else if (event == kWebSocketEvent::Message) {
		this->m_fnOnMessage(this, reinterpret_cast<uint8_t*>(pointer), length);
	}

}

bool WebSocketClientHandler::IsOpen() {
	return static_cast<bool>(EM_ASM_INT({
		return (sockets[$0].readyState === WebSocket.OPEN);
	}), this->GetIndex());
}

bool WebSocketClientHandler::Send(Binary* binary) {
	uint8_t* buffer = binary->GetBuffer();
	bool result = EM_ASM_INT({
		const socket = sockets[$0];
		if (socket.readyState !== WebSocket.OPEN) {
			return false;
		}
		try {
			socket.send(new Uint8Array(HEAPU8.buffer, $1, $2));
		} catch (error) {
			return false;
		}
		return true;
	}, this->GetIndex(), buffer, binary->GetBufferLength());
	
	delete buffer;
	return result;
}

void WebSocketClientHandler::Ping() {
	Binary* binary = new Binary();
	binary->WriteUInt8(static_cast<uint8_t>(Serverbound::Ping));

	if (this->Send(binary)) {
		this->m_llLastPingedAt = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	}
	delete binary;
}

long long WebSocketClientHandler::GetLastPingedAt() {
	return this->m_llLastPingedAt;
}

extern "C" {
	void EMSCRIPTEN_KEEPALIVE _WebSocketClientHandler_CheckEvent() {
		for (WebSocketClientHandler* instance : WebSocketClientHandler::_s_Instances) {
			instance->_PopEvent();
		}
	}
}