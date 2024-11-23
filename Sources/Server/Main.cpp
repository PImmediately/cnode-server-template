#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <vector>
#include <iostream>
#include <cstdint>

#include "./Server.h"
#include "./WebSocket/WebSocketServerHandler.h"
#include "./WebSocket/WebSocketClientHandler.h"
#include "./../Shared/WebSocket/Binary.h"
#include "./../Shared/WebSocket/Packet.h"

WebSocketServerHandler* create_websocket_server(Server* server) {
	WebSocketServerHandler* ws_server = new WebSocketServerHandler(server);
	server->AddWebSocketServerHandler(ws_server);
	std::cout << "[WebSocketServerHandler]" << " " << "servers[" << ws_server->GetIndex() << "]" << " " << "Created." << std::endl;

	ws_server->SetClientOnConnectCallback([](WebSocketClientHandler* client) {
		std::cout << "[WebSocketServerHandler]" << " " << "servers[" << client->GetWebSocketServerHandler()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnConnect()" << std::endl;
	});

	ws_server->SetClientOnDisconnectCallback([](WebSocketClientHandler* client) {
		std::cout << "[WebSocketServerHandler]" << " " << "servers[" << client->GetWebSocketServerHandler()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnDisconnect()" << std::endl;

		client->GetWebSocketServerHandler()->RemoveClient(client);
		client->GetWebSocketServerHandler()->SendClientCountToAllClients();

		delete client;
	});

	ws_server->SetClientOnErrorCallback([](WebSocketClientHandler* client) {
		std::cout << "[WebSocketServerHandler]" << " " << "servers[" << client->GetWebSocketServerHandler()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnError()" << std::endl;
	});

	ws_server->SetClientOnMessageCallback([](WebSocketClientHandler* client, uint8_t* data, size_t length) {
		// std::cout << "[WebSocketServerHandler]" << " " << "servers[" << client->GetWebSocketServerHandler()->GetIndex() << "]::m_Clients[" << client->GetIndex() << "]" << " " << "OnMessage()" << std::endl;

		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Serverbound header = static_cast<Serverbound>(binary->ReadUInt8());
		if (header == Serverbound::Ping) {
			Binary* binary = new Binary();
			binary->WriteUInt8(static_cast<uint8_t>(Clientbound::Ping));

			client->Send(binary);
			delete binary;
		}

		delete binary;
	});

	ws_server->SetClientOnMessageInTickCallback([](WebSocketClientHandler* client, uint8_t* data, size_t length) {
		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Serverbound header = static_cast<Serverbound>(binary->ReadUInt8());
		if (header == Serverbound::Connect) {
			const char* version_hash = binary->ReadString();

			const char* current_version_hash = client->GetWebSocketServerHandler()->GetServer()->GetVersionHash();
			if (memcmp(version_hash, current_version_hash, strlen(version_hash)) == 0) {
				Binary* binary = new Binary();
				binary->WriteUInt8(static_cast<uint8_t>(Clientbound::Accept));

				client->Send(binary);
				client->m_IsAccepted = true;

				client->GetWebSocketServerHandler()->SendClientCountToAllClients();
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
	uint32_t* EMSCRIPTEN_KEEPALIVE WebSocketServerHandler_Create(Server* server) {
		WebSocketServerHandler* ws_server = create_websocket_server(server);
		uint32_t result[2] = {
			ws_server->GetIndex(),
			static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ws_server))
		};
		return result;
	}
}

int main() {
	return 0;
}