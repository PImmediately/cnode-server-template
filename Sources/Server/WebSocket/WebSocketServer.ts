import type HTTPServer from "./../HTTP/HTTPServer";

import WebSocket from "ws";
import WebSocketClient from "./WebSocketClient";

import type Application from "./../../Application";

import http from "node:http";

import type { DeepReadonly } from "./../../Shared/TypeScript/UtilityTypes";

export interface WebSocketServerOptions {
	httpServer?: HTTPServer;
}

export default class WebSocketServer {

	private _server: (WebSocket.Server | undefined);
	public get server(): (WebSocket.Server | undefined) {
		return this._server;
	}

	public clients = new Array<WebSocketClient>();

	private pointer: (number | undefined);

	public constructor(public readonly application: Application, public readonly options: DeepReadonly<WebSocketServerOptions>) {
	}

	public create(): void {

		if ((!this.application.wasmModule) || (typeof this.application.server !== "number")) {
			throw new Error("WebAssembly module does not exist.");
		}

		this._server = new WebSocket.Server({
			server: this.options.httpServer?.server
		});

		this.server!.on("connection", (client, request) => {
			this.onConnection(client, request);
		});

		const pointer = this.application.wasmModule._WebSocketServerHandler_Create(this.application.server);
		const serverIndex = this.application.wasmModule.HEAPU32[(pointer + (0 << 2)) >> 2]!;
		const serverPointer = this.application.wasmModule.HEAPU32[(pointer + (1 << 2)) >> 2]!;

		this.pointer = serverPointer;
		this.application.wasmModule.$WebSocketServers[serverIndex] = this;

	}

	private onConnection(socket: WebSocket.WebSocket, request: http.IncomingMessage): void {

		if (!this.application.wasmModule) {
			throw new Error("WebAssembly module does not exist.");
		}
		if (typeof this.pointer !== "number") {
			throw new Error("The server has not been created.");
		}

		const pointer = this.application.wasmModule._WebSocketServerHandler_CreateClientHandler(this.pointer);
		const clientIndex = this.application.wasmModule.HEAPU32[(pointer + (0 << 2)) >> 2]!;
		const clientPointer = this.application.wasmModule.HEAPU32[(pointer + (1 << 2)) >> 2]!;

		const client = new WebSocketClient(this, clientPointer, clientIndex, socket, request);
		this.clients[clientIndex] = client;
		client.init();

	}

	public tick(): void {

		this.clients.forEach((client) => {
			client.tick();
		});

		if (this.application.server) {
			this.application.wasmModule?._Server_Tick(this.application.server);
		}

	}

}