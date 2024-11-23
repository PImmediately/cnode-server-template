#include "./WebSocketServerHandler.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <memory>
#include <algorithm>
#include "./../../Shared/WebSocket/Packet.h"

WebSocketServerHandler::WebSocketServerHandler(Server* server) : m_Server(server) {
#ifdef __EMSCRIPTEN__
	this->m_uIndex = EM_ASM_INT({
		for (let i = 0; i < servers.length; i++) {
			if (!servers[i]) {
				servers[i] = new Array();
				return i;
			}
		}
		servers.push(new Array());
		return (servers.length - 1);
	});
#endif
}

WebSocketServerHandler::~WebSocketServerHandler() {
#ifdef __EMSCRIPTEN__
	EM_ASM({
		servers[$0] = null;
	}, this->GetIndex());
#endif
}

void WebSocketServerHandler::AddClient(WebSocketClientHandler* client) {
	this->m_Clients.emplace_back(client);
}

bool WebSocketServerHandler::RemoveClient(WebSocketClientHandler* client) {
	auto it = std::remove_if(
		this->m_Clients.begin(),
		this->m_Clients.end(),
		[client](const WebSocketClientHandler* p) { return p == client; }
	);
	if (it == m_Clients.end()) {
		return false;
	}
	this->m_Clients.erase(it, this->m_Clients.end());
	return true;
}

void WebSocketServerHandler::SendToAllClients(Binary* binary) {
	for (WebSocketClientHandler* client : this->m_Clients) {
		client->Send(binary);
	}
}

void WebSocketServerHandler::SendClientCountToAllClients() {
	Binary* binary = new Binary();
	binary->WriteUInt8(static_cast<uint8_t>(Clientbound::ClientCount));
	binary->WriteVarUInt32(this->GetAcceptedClientCount());

	this->SendToAllClients(binary);
	delete binary;
}

extern "C" {
	uint32_t* EMSCRIPTEN_KEEPALIVE WebSocketServerHandler_CreateClientHandler(WebSocketServerHandler* ws_server) {
		WebSocketClientHandler* ws_client = new WebSocketClientHandler(ws_server);
		ws_server->AddClient(ws_client);

		uint32_t result[2] = {
			ws_client->GetIndex(),
			static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ws_client))
		};
		return result;
	}
}