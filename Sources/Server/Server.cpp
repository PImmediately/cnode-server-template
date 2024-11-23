#include "./Server.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <iostream>

Server::Server(char* version_hash) : m_strVersionHash(version_hash) {
}

void Server::Tick() {
	auto tick_started_at = std::chrono::high_resolution_clock::now();

	// tick
	{

		// do something

	}

	auto current_time = std::chrono::high_resolution_clock::now();
	++this->m_u_TPS;

	this->m_uDeltaTimeInTick = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::microseconds>(current_time - tick_started_at).count());
	if (this->m_uDeltaTimeInTick > 1000) {
		std::cout << "[Server]" << " " << "WARN:" << " " << "tick took " << this->GetDeltaTimeInTick() << " μs" << std::endl;
	}

	unsigned int elapsed_since_last_tps_check = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::microseconds>(current_time - this->m_uLastTPSCheckedAt).count());
	if (elapsed_since_last_tps_check >= 1000 * 1000) {
		this->m_uTPS = this->m_u_TPS;
		std::cout << "[Server]" << " " << "tps:" << " " << this->GetTPS() << std::endl;
		this->m_u_TPS = 0;
		this->m_uLastTPSCheckedAt = std::chrono::high_resolution_clock::now();
	}
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
}