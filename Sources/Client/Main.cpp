#include <emscripten.h>

#include <iostream>
#include <regex>
#include <chrono>

#include "./WebSocket/WebSocketClient.h"
#include "./../Shared/WebSocket/Binary.h"
#include "./../Shared/WebSocket/Packet.h"

char* get_version_hash() {
	return (char*)EM_ASM_PTR({
		return stringToNewUTF8("@server::version_hash");
	});
}

char* get_location_href() {
	return (char*)EM_ASM_PTR({
		return stringToNewUTF8(window.location.href);
	});
}

void reload_page() {
	EM_ASM({
		window.onbeforeunload = null;
		window.location.reload();
	});
}

void recreate_websocket() {
	EM_ASM({
		setTimeout(() => {
			Module["_CreateWebSocket"]();
		}, 500);
	});
}

void set_ping(int ping) {
	EM_ASM({
		document.querySelector("#ping").textContent = ($0 / 1000 / 1000).toFixed(1);
	}, ping);
}

void set_client_count(unsigned int client_count) {
	EM_ASM({
		document.querySelector("#client-count").textContent = String($0);
	}, client_count);
}

void connect() {
	char* location_href = get_location_href();
	std::string server_ip_ = std::regex_replace(location_href, std::regex("http"), "ws");
	const char* server_ip = server_ip_.c_str();

	std::cout << "Connecting to " << server_ip << " (WebSocketClient)..." << std::endl;
	WebSocketClient* client = new WebSocketClient(server_ip);
	client->SetOnConnectCallback([](WebSocketClient* client) {
		std::cout << "WebSocketClient::OnConnect()" << std::endl;

		Binary* binary = new Binary();
		binary->WriteUInt8(static_cast<uint8_t>(Serverbound::Connect));
		binary->WriteString(get_version_hash());

		client->Send(binary);
		delete binary;
	});
	client->SetOnDisconnectCallback([](WebSocketClient* client) {
		std::cout << "WebSocketClient::OnDisconnect()" << std::endl;
		delete client;

		recreate_websocket();
	});
	client->SetOnErrorCallback([](WebSocketClient* client) {
		std::cout << "WebSocketClient::OnError()" << std::endl;
	});
	client->SetOnMessageCallback([](WebSocketClient* client, uint8_t* data, size_t length) {
		// std::cout << "WebSocketClient::OnMessage()" << std::endl;

		Binary* binary = new Binary();
		binary->SetBuffer(data, length);

		Clientbound header = static_cast<Clientbound>(binary->ReadUInt8());
		if (header == Clientbound::Accept) {
			std::cout << "Accepted." << std::endl;

			client->Ping();
		} else if (header == Clientbound::Disconnect) {
			ClientboundDisconnectReason reason = static_cast<ClientboundDisconnectReason>(binary->ReadUInt8());

			if (reason == ClientboundDisconnectReason::Outdated) {
				reload_page();
			}
		} else if (header == Clientbound::Ping) {
			int ping = std::chrono::high_resolution_clock::now().time_since_epoch().count() - client->GetLastPingedAt();
			set_ping(ping);

			client->Ping();
		} else if (header == Clientbound::ClientCount) {
			uint32_t client_count = binary->ReadVarUInt32();

			set_client_count(client_count);
		}

		delete binary;
		delete data;
	});
}
extern "C" {
	void EMSCRIPTEN_KEEPALIVE CreateWebSocket() {
		connect();
	}
}

void main_loop() {
}

int main() {
	std::cout << "main()" << std::endl;

	connect();

	emscripten_set_main_loop(main_loop, 60, true);
	return 0;
}