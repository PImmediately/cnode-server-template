#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <cstdint>
#include <functional>
#include <vector>
#include "./../../Shared/WebSocket/Binary.h"

class WebSocketServer; 

class WebSocketClient {
public:

	WebSocketClient(WebSocketServer* server);
	~WebSocketClient();

	inline WebSocketServer* GetWebSocketServer() { return this->m_WSServer; }
	inline unsigned int GetIndex() { return this->m_uIndex; }

	void PushCachedReceivedMessage(uint8_t* data, size_t length);
	void EmitCachedReceivedMessages();

	void Send(Binary* binary);
	void Kick();

	bool m_IsAccepted = false;

private:
	WebSocketServer* m_WSServer;
	std::vector<std::vector<uint8_t>> m_uCachedReceivedMessages;

	unsigned int m_uIndex;

};

#endif // WEBSOCKET_CLIENT_H