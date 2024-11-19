#include "./WebSocketServer.h"

#include <emscripten.h>
#include <memory>
#include <algorithm>
#include "./../../Shared/WebSocket/Packet.h"

WebSocketServer::WebSocketServer(Server* server) : m_Server(server) {
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
}

WebSocketServer::~WebSocketServer() {
	EM_ASM({
		servers[$0] = null;
	}, this->GetIndex());
}

void WebSocketServer::AddClient(WebSocketClient* client) {
	this->m_Clients.emplace_back(client);
}

bool WebSocketServer::RemoveClient(WebSocketClient* client) {
	auto it = std::remove_if(
		this->m_Clients.begin(),
		this->m_Clients.end(),
		[client](const WebSocketClient* p) { return p == client; }
	);
	if (it == m_Clients.end()) {
		return false;
	}
	this->m_Clients.erase(it, this->m_Clients.end());
	return true;
}

void WebSocketServer::SendToAllClients(Binary* binary) {
	for (WebSocketClient* client : this->m_Clients) {
		client->Send(binary);
	}
}

void WebSocketServer::SendClientCountToAllClients() {
	Binary* binary = new Binary();
	binary->WriteUInt8(static_cast<uint8_t>(Clientbound::ClientCount));
	binary->WriteVarUInt32(this->GetAcceptedClientCount());

	this->SendToAllClients(binary);
	delete binary;
}

void WebSocketServer::Tick() {
	for (WebSocketClient* client : this->m_Clients) {
		client->EmitCachedReceivedMessages();
	}
}