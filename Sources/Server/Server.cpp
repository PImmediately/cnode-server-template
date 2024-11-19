#include "./Server.h"

#include <iostream>

Server::Server(char* version_hash) : m_strVersionHash(version_hash) {
}

void Server::Tick() {
	auto tick_started_at = std::chrono::high_resolution_clock::now();

	// tick
	{

		for (WebSocketServer* wsServer : this->m_WSServers) {
			wsServer->Tick();
		}

	}

	auto current_time = std::chrono::high_resolution_clock::now();
	++this->m_uTPS;

	int delta_time_in_tick = std::chrono::duration_cast<std::chrono::microseconds>(current_time - tick_started_at).count();
	if (delta_time_in_tick > 1000) {
		std::cout << "[Server]" << " " << "WARN:" << " " << "tick took " << delta_time_in_tick << " μs" << std::endl;
	}

	int elapsed_since_last_tps_check = std::chrono::duration_cast<std::chrono::microseconds>(current_time - this->m_uLastTPSCheckedAt).count();
	if (elapsed_since_last_tps_check >= 1000 * 1000) {
		std::cout << "[Server]" << " " << "tps:" << " " << this->m_uTPS << std::endl;
		this->m_uTPS = 0;
		this->m_uLastTPSCheckedAt = std::chrono::high_resolution_clock::now();
	}
}