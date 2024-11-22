#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <iostream>

#include "./Client.h"

Client* client = new Client();

extern "C" {
	void EMSCRIPTEN_KEEPALIVE CreateWebSocket() {
		client->CreateWebSocket();
	}
}

void tick() {
	client->Tick();
}

int main() {
	std::cout << "main()" << std::endl;

	client->CreateWebSocket();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(tick, 60, true);
#endif
	return 0;
}