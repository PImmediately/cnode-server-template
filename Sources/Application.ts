import HTTPServer from "./Server/HTTP/HTTPServer";
import WebSocketServer from "./Server/WebSocket/WebSocketServer";

export interface WebAssemblyModule {

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

	_WebSocketServer_Create: (server: number) => number;
	_WebSocketServer_CreateClient: (server: number) => number;

	_WebSocketClient_OnConnect: (client: number) => void;
	_WebSocketClient_OnDisconnect: (client: number) => void;
	_WebSocketClient_OnError: (client: number) => void;
	_WebSocketClient_OnMessage: (client: number, pointer: number, length: number) => void;

}

export default class Application {

	private _httpServer: (HTTPServer | undefined);
	public get httpServer(): (HTTPServer | undefined) {
		return this._httpServer;
	}

	private _webSocketServer: (WebSocketServer | undefined);
	public get webSocketServer(): (WebSocketServer | undefined) {
		return this._webSocketServer;
	}

	private _WebAssemblyModule: (WebAssemblyModule | undefined);
	public get WebAssemblyModule(): (WebAssemblyModule | undefined) {
		return this._WebAssemblyModule;
	}

	private _server: (number | undefined);
	public get server(): (number | undefined) {
		return this._server;
	}

	public constructor() {
	}

	public async executeServerWebAssembly(): Promise<void> {

		// DO NOT DELETE THIS
		"@server::server_cpp.bundle.js";

		this._WebAssemblyModule =
			// @ts-ignore	
			await Module() as WebAssemblyModule;
		// @ts-ignore

		this.WebAssemblyModule!.$WebSocketServers = new Array<WebSocketServer>();

		this._server = this.WebAssemblyModule!._Server_Create(this.WebAssemblyModule!._stringToNewUTF8("@server::version_hash"));

	}

	public async createHTTPServer(port: number): Promise<void> {

		this._httpServer = new HTTPServer(this, {
			port: port
		});

		await this.httpServer!.create();

	}

	public async createWebSocketServer(): Promise<void> {

		this._webSocketServer = new WebSocketServer(this, {
			httpServer: this.httpServer
		});

		this.webSocketServer!.create();

	}

	public tick(): void {
		if (this.server) {
			this.WebAssemblyModule?._Server_Tick(this.server);
		}
	}

}

(async () => {

	const port = (parseInt(process.env.PORT!) || 6430);

	const application = new Application();
	await application.executeServerWebAssembly();
	await application.createHTTPServer(port);
	await application.createWebSocketServer();

	setInterval(() => {
		application.tick();
	}, (1000 / 20));

})();