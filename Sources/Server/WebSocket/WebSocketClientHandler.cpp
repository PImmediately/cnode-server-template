#include "./WebSocketClientHandler.h"

#include "./WebSocketServerHandler.h"
#include <emscripten.h>
#include <iostream>

WebSocketClientHandler::WebSocketClientHandler(WebSocketServerHandler* ws_server) : m_WSServer(ws_server) {
	this->m_uIndex = EM_ASM_INT({
		const sockets = servers[$0];
		for (let i = 0; i < sockets.length; i++) {
			if (!sockets[i]) {
				sockets[i] = true;
				return i;
			}
		}
		sockets.push(true);
		return (sockets.length - 1);
	}, this->GetWebSocketServerHandler()->GetIndex());
}

WebSocketClientHandler::~WebSocketClientHandler() {
	EM_ASM({
		servers[$0][$1] = null;
	}, this->GetWebSocketServerHandler()->GetIndex(), this->GetIndex());
}

void WebSocketClientHandler::PushCachedReceivedMessage(uint8_t* data, size_t length) {
	this->m_uCachedReceivedMessages.emplace_back(data, (data + length));
}

void WebSocketClientHandler::EmitCachedReceivedMessages() {
	for (auto message = this->m_uCachedReceivedMessages.begin(); message != this->m_uCachedReceivedMessages.end(); ) {
		this->GetWebSocketServerHandler()->_m_fnClientOnMessageInTick(this, message->data(), message->size());
		message = this->m_uCachedReceivedMessages.erase(message);
	}
}

void WebSocketClientHandler::Send(Binary* binary) {
	uint8_t* buffer = binary->GetBuffer();
	EM_ASM({
		const buffer = new Uint8Array(Module.HEAPU8.buffer, $2, $3);
		Module["$WebSocketServers"][$0].clients[$1].send(buffer);
	}, this->GetWebSocketServerHandler()->GetIndex(), this->GetIndex(), buffer, binary->GetBufferLength());

	delete buffer;
}

void WebSocketClientHandler::Kick() {
	EM_ASM({
		Module["$WebSocketServers"][$0].clients[$1].kick();
	}, this->GetWebSocketServerHandler()->GetIndex(), this->GetIndex());
}