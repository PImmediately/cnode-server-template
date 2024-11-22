#include "./Client.h"

#include <iostream>
#include <regex>
#include <chrono>

#include "./WebSocket/WebSocketClientHandler.h"
#include "./../Shared/WebSocket/Binary.h"
#include "./../Shared/WebSocket/Packet.h"

void Client::CreateWebSocket() {
	std::string location_href = this->GetLocationHref();
	std::string server_ip_ = std::regex_replace(location_href, std::regex("http"), "ws");
	const char* server_ip = server_ip_.c_str();

	std::cout << "Connecting to " << server_ip << " (WebSocketClientHandler)..." << std::endl;
	WebSocketClientHandler* client = new WebSocketClientHandler(this, server_ip);
	client->SetOnConnectCallback([](WebSocketClientHandler* client) {
		std::cout << "WebSocketClientHandler::OnConnect()" << std::endl;

		Binary* binary = new Binary();
		binary->WriteUInt8(static_cast<uint8_t>(Serverbound::Connect));
		binary->WriteString(client->GetClient()->GetVersionHash());

		client->Send(binary);
		delete binary;
	});
	client->SetOnDisconnectCallback([](WebSocketClientHandler* client) {
		std::cout << "WebSocketClientHandler::OnDisconnect()" << std::endl;
		delete client;

		client->GetClient()->RecreateWebSocket();
	});
	client->SetOnErrorCallback([](WebSocketClientHandler* client) {
		std::cout << "WebSocketClientHandler::OnError()" << std::endl;
	});
	client->SetOnMessageCallback([](WebSocketClientHandler* client, uint8_t* data, size_t length) {
		// std::cout << "WebSocketClientHandler::OnMessage()" << std::endl;

		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Clientbound header = static_cast<Clientbound>(binary->ReadUInt8());
		if (header == Clientbound::Accept) {
			std::cout << "Accepted." << std::endl;

			client->Ping();
		} else if (header == Clientbound::Disconnect) {
			ClientboundDisconnectReason reason = static_cast<ClientboundDisconnectReason>(binary->ReadUInt8());

			if (reason == ClientboundDisconnectReason::Outdated) {
				client->GetClient()->ReloadPage();
			}
		} else if (header == Clientbound::Ping) {
			int ping = std::chrono::high_resolution_clock::now().time_since_epoch().count() - client->GetLastPingedAt();
			client->GetClient()->SetDisplayPing(ping);

			client->Ping();
		} else if (header == Clientbound::ClientCount) {
			uint32_t client_count = binary->ReadVarUInt32();

			client->GetClient()->SetDisplayClientCount(client_count);
		}

		delete binary;
		delete data;
	});
}

void Client::Tick() {
}