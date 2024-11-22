import type WebSocketServer from "./WebSocket/WebSocketServer";

export default interface WebAssemblyModule {

	_malloc: (size: number) => number;
	_free: (pointer: number) => void;
	_stringToNewUTF8: (string: string) => number;

	HEAPU8: Uint8Array;
	HEAPU16: Uint16Array;
	HEAPU32: Uint32Array;
	HEAP8: Int8Array;
	HEAP16: Int16Array;
	HEAP32: Int32Array;
	HEAPF32: Float32Array;
	HEAPF64: Float64Array;

	$WebSocketServers: Array<WebSocketServer>;

	_Server_Create: (versionHash: number) => number;
	_Server_Delete: (server: number) => void;
	_Server_Tick: (server: number) => void;

	_WebSocketServerHandler_Create: (server: number) => number;
	_WebSocketServerHandler_CreateClientHandler: (server: number) => number;

	_WebSocketClientHandler_EmitOnConnect: (client: number) => void;
	_WebSocketClientHandler_EmitOnDisconnect: (client: number) => void;
	_WebSocketClientHandler_EmitOnError: (client: number) => void;
	_WebSocketClientHandler_EmitOnMessage: (client: number, pointer: number, length: number) => void;
	_WebSocketClientHandler_EmitOnMessageInTick: (client: number, pointer: number, length: number) => void;

}