#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include "./WebSocket/WebSocketServer.h"
#include <chrono>

class Server {
public:

	Server(char* version_hash);

	inline char* GetVersionHash() { return this->m_strVersionHash; }

	inline void AddWebSocketServer(WebSocketServer* wsServer) {
		this->m_WSServers.emplace_back(wsServer);
	}

	inline bool RemoveWebSocketServer(WebSocketServer* wsServer) {
		auto it = std::remove_if(
			this->m_WSServers.begin(),
			this->m_WSServers.end(),
			[wsServer](const WebSocketServer* pointer) { return pointer == wsServer; }
		);
		if (it == m_WSServers.end()) {
			return false;
		}
		this->m_WSServers.erase(it, this->m_WSServers.end());
		return true;
	}

	void Tick();

private:
	char* m_strVersionHash;

	std::vector<WebSocketServer*> m_WSServers;

	unsigned int m_uTPS = 0;
	std::chrono::steady_clock::time_point m_uLastTPSCheckedAt = std::chrono::high_resolution_clock::now();

};

#endif // SERVER_H