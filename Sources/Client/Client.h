#ifndef CLIENT_H
#define CLIENT_H

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class Client {
public:

	inline char* GetVersionHash() {
#ifdef __EMSCRIPTEN__
		return (char*)EM_ASM_PTR({
			return stringToNewUTF8("@server::version_hash");
		});
#endif
	}

	inline char* GetLocationHref() {
#ifdef __EMSCRIPTEN__
		return (char*)EM_ASM_PTR({
			return stringToNewUTF8(window.location.href);
		});
#endif
	}

	inline char* ReloadPage() {
#ifdef __EMSCRIPTEN__
		EM_ASM({
			window.onbeforeunload = null;
			window.location.reload();
		});
#endif
	}

	void CreateWebSocket();
	inline void RecreateWebSocket() {
#ifdef __EMSCRIPTEN__
		EM_ASM({
			setTimeout(() => {
				Module["_CreateWebSocket"]();
			}, 500);
		});
#endif
	}

	inline void SetDisplayPing(int ping) {
#ifdef __EMSCRIPTEN__
		EM_ASM({
			document.querySelector("#ping").textContent = ($0 / 1000 / 1000).toFixed(1);
		}, ping);
#endif
	}

	inline void SetDisplayClientCount(unsigned int client_count) {
#ifdef __EMSCRIPTEN__
		EM_ASM({
			document.querySelector("#client-count").textContent = String($0);
		}, client_count);
#endif
	}

	void Tick();

};

#endif // CLIENT_H