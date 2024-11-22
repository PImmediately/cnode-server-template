#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <iostream>
#include <regex>
#include <chrono>

#include "./WebSocket/WebSocketClientHandler.h"
#include "./../Shared/WebSocket/Binary.h"
#include "./../Shared/WebSocket/Packet.h"

char* get_version_hash() {
#ifdef __EMSCRIPTEN__
	return (char*)EM_ASM_PTR({
		return stringToNewUTF8("@server::version_hash");
	});
#endif
}

char* get_location_href() {
#ifdef __EMSCRIPTEN__
	return (char*)EM_ASM_PTR({
		return stringToNewUTF8(window.location.href);
	});
#endif
}

void reload_page() {
#ifdef __EMSCRIPTEN__
	EM_ASM({
		window.onbeforeunload = null;
		window.location.reload();
	});
#endif
}

void recreate_websocket() {
#ifdef __EMSCRIPTEN__
	EM_ASM({
		setTimeout(() => {
			Module["_CreateWebSocket"]();
		}, 500);
	});
#endif
}

void set_ping(int ping) {
#ifdef __EMSCRIPTEN__
	EM_ASM({
		document.querySelector("#ping").textContent = ($0 / 1000 / 1000).toFixed(1);
	}, ping);
#endif
}

void set_client_count(unsigned int client_count) {
#ifdef __EMSCRIPTEN__
	EM_ASM({
		document.querySelector("#client-count").textContent = String($0);
	}, client_count);
#endif
}

void connect() {
	char* location_href = get_location_href();
	std::string server_ip_ = std::regex_replace(location_href, std::regex("http"), "ws");
	const char* server_ip = server_ip_.c_str();

	std::cout << "Connecting to " << server_ip << " (WebSocketClientHandler)..." << std::endl;
	WebSocketClientHandler* client = new WebSocketClientHandler(server_ip);
	client->SetOnConnectCallback([](WebSocketClientHandler* client) {
		std::cout << "WebSocketClientHandler::OnConnect()" << std::endl;

		Binary* binary = new Binary();
		binary->WriteUInt8(static_cast<uint8_t>(Serverbound::Connect));
		binary->WriteString(get_version_hash());

		client->Send(binary);
		delete binary;
	});
	client->SetOnDisconnectCallback([](WebSocketClientHandler* client) {
		std::cout << "WebSocketClientHandler::OnDisconnect()" << std::endl;
		delete client;

		recreate_websocket();
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

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 60, true);
#endif
	return 0;
}