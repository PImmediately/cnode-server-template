import HTTPServer from "./Server/HTTP/HTTPServer";
import WebSocketServer from "./Server/WebSocket/WebSocketServer";

import type WebAssemblyModule from "./Server/WebAssemblyModule";

export default class Application {

	private _httpServer: (HTTPServer | undefined);
	public get httpServer(): (HTTPServer | undefined) {
		return this._httpServer;
	}

	private _wsServer: (WebSocketServer | undefined);
	public get wsServer(): (WebSocketServer | undefined) {
		return this._wsServer;
	}

	private _wasmModule: (WebAssemblyModule | undefined);
	public get wasmModule(): (WebAssemblyModule | undefined) {
		return this._wasmModule;
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

		this._wasmModule =
			// @ts-ignore	
			await Module() as WebAssemblyModule;
		// @ts-ignore

		this.wasmModule!.$WebSocketServers = new Array<WebSocketServer>();

		this._server = this.wasmModule!._Server_Create(this.wasmModule!._stringToNewUTF8("@server::version_hash"));

	}

	public async createHTTPServer(port: number): Promise<void> {

		this._httpServer = new HTTPServer(this, {
			port: port
		});

		await this.httpServer!.create();

	}

	public async createWebSocketServer(): Promise<void> {

		this._wsServer = new WebSocketServer(this, {
			httpServer: this.httpServer
		});

		this.wsServer!.create();

	}

	public tick(): void {

		this.wsServer?.tick();

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