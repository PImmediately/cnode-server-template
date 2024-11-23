#ifndef WEBSOCKET_CLIENT_HANDLER_H
#define WEBSOCKET_CLIENT_HANDLER_H

#include "./../../Shared/WebSocket/Binary.h"

class WebSocketServerHandler; 

class WebSocketClientHandler {
public:

	WebSocketClientHandler(WebSocketServerHandler* server);
	~WebSocketClientHandler();

	inline WebSocketServerHandler* GetWebSocketServerHandler() { return this->m_WSServer; }
	inline unsigned int GetIndex() { return this->m_uIndex; }

	void Send(Binary* binary);
	void Kick();

	bool m_IsAccepted = false;

private:
	WebSocketServerHandler* m_WSServer;

	unsigned int m_uIndex;

};

#endif // WEBSOCKET_CLIENT_HANDLER_H