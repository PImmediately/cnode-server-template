#ifndef PACKET_H
#define PACKET_H

#include <cstdint>

enum class Serverbound : uint8_t {
	Connect,
	Ping
};

enum class Clientbound : uint8_t {
	Accept,
	Disconnect,
	Ping,
	ClientCount
};

enum class ClientboundDisconnectReason : uint8_t {
	Outdated,
	ProtocolError
};

#endif // PACKET_H