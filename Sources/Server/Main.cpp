#include <emscripten.h>
#include <vector>
#include <iostream>
#include <cstdint>

#include "./Server.h"
#include "./WebSocket/WebSocketServer.h"
#include "./WebSocket/WebSocketClient.h"
#include "./../Shared/WebSocket/Binary.h"
#include "./../Shared/WebSocket/Packet.h"

WebSocketServer* create_websocket_server(Server* server) {
	WebSocketServer* ws_server = new WebSocketServer(server);
	server->AddWebSocketServer(ws_server);
	std::cout << "[WebSocketServer]" << " " << "servers[" << ws_server->GetIndex() << "]" << " " << "Created." << std::endl;

	ws_server->SetClientOnConnectCallback([](WebSocketClient* client) {
		std::cout << "[WebSocketServer]" << " " << "servers[" << client->GetWebSocketServer()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnConnect()" << std::endl;
	});

	ws_server->SetClientOnDisconnectCallback([](WebSocketClient* client) {
		std::cout << "[WebSocketServer]" << " " << "servers[" << client->GetWebSocketServer()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnDisconnect()" << std::endl;

		client->GetWebSocketServer()->RemoveClient(client);
		client->GetWebSocketServer()->SendClientCountToAllClients();

		delete client;
	});

	ws_server->SetClientOnErrorCallback([](WebSocketClient* client) {
		std::cout << "[WebSocketServer]" << " " << "servers[" << client->GetWebSocketServer()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnError()" << std::endl;
	});

	ws_server->SetClientOnMessageCallback([](WebSocketClient* client, uint8_t* data, size_t length) {
		// std::cout << "[WebSocketServer]" << " " << "servers[" << client->GetWebSocketServer()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnMessage()" << std::endl;

		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Serverbound header = static_cast<Serverbound>(binary->ReadUInt8());
		if (header == Serverbound::Ping) {
			Binary* binary = new Binary();
			binary->WriteUInt8(static_cast<uint8_t>(Clientbound::Ping));

			client->Send(binary);
			delete binary;
		} else {
			client->PushCachedReceivedMessage(data, length);
		}

		delete binary;
	});

	ws_server->SetClientOnMessageInTickCallback([](WebSocketClient* client, uint8_t* data, size_t length) {
		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Serverbound header = static_cast<Serverbound>(binary->ReadUInt8());
		if (header == Serverbound::Connect) {
			const char* version_hash = binary->ReadString();

			const char* current_version_hash = client->GetWebSocketServer()->GetServer()->GetVersionHash();
			if (memcmp(version_hash, current_version_hash, strlen(version_hash)) == 0) {
				Binary* binary = new Binary();
				binary->WriteUInt8(static_cast<uint8_t>(Clientbound::Accept));

				client->Send(binary);
				client->m_IsAccepted = true;

				client->GetWebSocketServer()->SendClientCountToAllClients();
				delete binary;
			} else {
				Binary* binary = new Binary();
				binary->WriteUInt8(static_cast<uint8_t>(Clientbound::Disconnect));
				binary->WriteUInt8(static_cast<uint8_t>(ClientboundDisconnectReason::Outdated));

				client->Send(binary);
				client->Kick();
				delete binary;
			}
		}

		delete binary;
	});

	return ws_server;
}

extern "C" {
	Server* EMSCRIPTEN_KEEPALIVE Server_Create(char* version_hash) {
		Server* server = new Server(version_hash);
		return server;
	}
	void EMSCRIPTEN_KEEPALIVE Server_Delete(Server* server) {
		delete server;
	}
	void EMSCRIPTEN_KEEPALIVE Server_Tick(Server* server) {
		server->Tick();		
	}

	uint32_t* EMSCRIPTEN_KEEPALIVE WebSocketServer_Create(Server* server) {
		WebSocketServer* ws_server = create_websocket_server(server);
		uint32_t result[2] = {
			ws_server->GetIndex(),
			static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ws_server))
		};
		return result;
	}
	uint32_t* EMSCRIPTEN_KEEPALIVE WebSocketServer_CreateClient(WebSocketServer* ws_server) {
		WebSocketClient* ws_client = new WebSocketClient(ws_server);
		ws_server->AddClient(ws_client);

		uint32_t result[2] = {
			ws_client->GetIndex(),
			static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ws_client))
		};
		return result;
	}

	void EMSCRIPTEN_KEEPALIVE WebSocketClient_OnConnect(WebSocketClient* ws_client) {
		ws_client->GetWebSocketServer()->_m_fnClientOnConnect(ws_client);
	}
	void EMSCRIPTEN_KEEPALIVE WebSocketClient_OnDisconnect(WebSocketClient* ws_client) {
		ws_client->GetWebSocketServer()->_m_fnClientOnDisconnect(ws_client);
	}
	void EMSCRIPTEN_KEEPALIVE WebSocketClient_OnError(WebSocketClient* ws_client) {
		ws_client->GetWebSocketServer()->_m_fnClientOnError(ws_client);
	}
	void EMSCRIPTEN_KEEPALIVE WebSocketClient_OnMessage(WebSocketClient* ws_client, uint8_t* data, const size_t length) {
		ws_client->GetWebSocketServer()->_m_fnClientOnMessage(ws_client, data, length);
	}
}

int main() {
	return 0;
}