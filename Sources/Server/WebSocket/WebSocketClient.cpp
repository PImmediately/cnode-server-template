#include "./WebSocketClient.h"

#include "./WebSocketServer.h"
#include <emscripten.h>
#include <iostream>

WebSocketClient::WebSocketClient(WebSocketServer* ws_server) : m_WSServer(ws_server) {
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
	}, this->GetWebSocketServer()->GetIndex());
}

WebSocketClient::~WebSocketClient() {
	EM_ASM({
		servers[$0][$1] = null;
	}, this->GetWebSocketServer()->GetIndex(), this->GetIndex());
}

void WebSocketClient::PushCachedReceivedMessage(uint8_t* data, size_t length) {
	this->m_uCachedReceivedMessages.emplace_back(data, (data + length));
}

void WebSocketClient::EmitCachedReceivedMessages() {
	for (auto message = this->m_uCachedReceivedMessages.begin(); message != this->m_uCachedReceivedMessages.end(); ) {
		this->GetWebSocketServer()->_m_fnClientOnMessageInTick(this, message->data(), message->size());
		message = this->m_uCachedReceivedMessages.erase(message);
	}
}

void WebSocketClient::Send(Binary* binary) {
	uint8_t* buffer = binary->GetBuffer();
	EM_ASM({
		const buffer = new Uint8Array(Module.HEAPU8.buffer, $2, $3);
		Module["$WebSocketServers"][$0].clients[$1].send(buffer);
	}, this->GetWebSocketServer()->GetIndex(), this->GetIndex(), buffer, binary->GetBufferLength());

	delete buffer;
}

void WebSocketClient::Kick() {
	EM_ASM({
		Module["$WebSocketServers"][$0].clients[$1].kick();
	}, this->GetWebSocketServer()->GetIndex(), this->GetIndex());
}