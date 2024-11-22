#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include "./WebSocketClientHandler.h"
#include <vector>
#include <memory>
#include "./../../Shared/WebSocket/Binary.h"

class Server;

class WebSocketServerHandler {
	typedef void (*ClientOnConnectFn)(WebSocketClientHandler *);
	typedef void (*ClientOnDisconnectFn)(WebSocketClientHandler *);
	typedef void (*ClientOnErrorFn)(WebSocketClientHandler *);
	typedef void (*ClientOnMessageFn)(WebSocketClientHandler *, uint8_t *, size_t);
public:

	WebSocketServerHandler(Server* server);
	~WebSocketServerHandler();

	inline Server* GetServer() { return this->m_Server; }
	inline unsigned int GetIndex() { return this->m_uIndex; }

	ClientOnConnectFn _m_fnClientOnConnect = nullptr;
	ClientOnDisconnectFn _m_fnClientOnDisconnect = nullptr;
	ClientOnErrorFn _m_fnClientOnError = nullptr;
	ClientOnMessageFn _m_fnClientOnMessage = nullptr;
	ClientOnMessageFn _m_fnClientOnMessageInTick = nullptr;
	void SetClientOnConnectCallback(ClientOnConnectFn v){ this->_m_fnClientOnConnect = v; }
	void SetClientOnDisconnectCallback(ClientOnDisconnectFn v){ this->_m_fnClientOnDisconnect = v; }
	void SetClientOnErrorCallback(ClientOnErrorFn v){ this->_m_fnClientOnError = v; }
	void SetClientOnMessageCallback(ClientOnMessageFn v){ this->_m_fnClientOnMessage = v; }
	void SetClientOnMessageInTickCallback(ClientOnMessageFn v){ this->_m_fnClientOnMessageInTick = v; }

	inline size_t GetClientCount() {
		return this->m_Clients.size();
	}
	inline size_t GetAcceptedClientCount() {
		size_t count = 0;
		for (WebSocketClientHandler* client : this->m_Clients) {
			if (client->m_IsAccepted) {
				++count;
			}
		}
    	return count;
	}

	void AddClient(WebSocketClientHandler* client);
	bool RemoveClient(WebSocketClientHandler* client);

	void SendToAllClients(Binary* binary);

	void SendClientCountToAllClients();

	void Tick();

private:
	Server* m_Server;
	std::vector<WebSocketClientHandler*> m_Clients;

	unsigned int m_uIndex;

};

#endif // WEBSOCKET_SERVER_H