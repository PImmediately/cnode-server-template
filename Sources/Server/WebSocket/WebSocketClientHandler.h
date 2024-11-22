#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <cstdint>
#include <functional>
#include <vector>
#include "./../../Shared/WebSocket/Binary.h"

class WebSocketServerHandler; 

class WebSocketClientHandler {
public:

	WebSocketClientHandler(WebSocketServerHandler* server);
	~WebSocketClientHandler();

	inline WebSocketServerHandler* GetWebSocketServerHandler() { return this->m_WSServer; }
	inline unsigned int GetIndex() { return this->m_uIndex; }

	void PushCachedReceivedMessage(uint8_t* data, size_t length);
	void EmitCachedReceivedMessages();

	void Send(Binary* binary);
	void Kick();

	bool m_IsAccepted = false;

private:
	WebSocketServerHandler* m_WSServer;
	std::vector<std::vector<uint8_t>> m_uCachedReceivedMessages;

	unsigned int m_uIndex;

};

#endif // WEBSOCKET_CLIENT_H