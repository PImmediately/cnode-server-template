#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <vector>
#include <cstdint>
#include "./../../Shared/WebSocket/Binary.h"

class WebSocketClient {
	typedef void (*OnConnectFn)(WebSocketClient *);
	typedef void (*OnDisconnectFn)(WebSocketClient *);
	typedef void (*OnErrorFn)(WebSocketClient *);
	typedef void (*OnMessageFn)(WebSocketClient *, uint8_t *, size_t);
public:

	WebSocketClient(const char* server_ip);
	~WebSocketClient();

	inline unsigned int GetIndex() { return this->m_uIndex; }

	static std::vector<WebSocketClient*> _s_Instances;

	void SetOnConnectCallback(OnConnectFn v){ this->m_fnOnConnect = v; }
	void SetOnDisconnectCallback(OnDisconnectFn v){ this->m_fnOnDisconnect = v; }
	void SetOnErrorCallback(OnErrorFn v){ this->m_fnOnError = v; }
	void SetOnMessageCallback(OnMessageFn v){ this->m_fnOnMessage = v; }

	void _PopEvent();

	bool IsOpen();
	bool Send(Binary* binary);

	void Ping();
	long long GetLastPingedAt();

private:
	unsigned int m_uIndex;

	OnConnectFn m_fnOnConnect = nullptr;
	OnDisconnectFn m_fnOnDisconnect = nullptr;
	OnErrorFn m_fnOnError = nullptr;
	OnMessageFn m_fnOnMessage = nullptr;

	long long m_llLastPingedAt;

};

#endif // WEBSOCKET_CLIENT_H