import type WebSocketServer from "./WebSocketServer";
import type http from "node:http";
import type WebSocket from "ws";

export default class WebSocketClient {

	public constructor(
		public readonly server: WebSocketServer,
		public readonly pointer: number,
		public readonly index: number,
		public readonly socket: WebSocket.WebSocket,
		public readonly request: http.IncomingMessage
	) {
		this.socket.binaryType = "arraybuffer";
	}

	public init(): void {
		this.onConnect();
		this.socket.addEventListener("close", (event) => {
			this.onDisconnect(event);
		});
		this.socket.addEventListener("error", (event) => {
			this.onError(event);
		});
		this.socket.addEventListener("message", (event) => {
			this.onMessage(event.data);
		});
	}

	private onConnect(): void {

		this.server.application.wasmModule!._WebSocketClientHandler_OnConnect(this.pointer);

	}

	private onDisconnect(event: WebSocket.CloseEvent): void {

		this.server.application.wasmModule!._WebSocketClientHandler_OnDisconnect(this.pointer);

		delete this.server.clients[this.index];

	}

	private onError(event: WebSocket.ErrorEvent): void {

		this.server.application.wasmModule!._WebSocketClientHandler_OnError(this.pointer);

	}

	private onMessage(data: WebSocket.Data): void {

		if (!(data instanceof ArrayBuffer)) {
			return;
		}

		const buffer = new Uint8Array(data);

		const pointer = this.server.application.wasmModule!._malloc(buffer.length);
		this.server.application.wasmModule!.HEAPU8.set(buffer, pointer);
		this.server.application.wasmModule!._WebSocketClientHandler_OnMessage(this.pointer, pointer, buffer.length);
		this.server.application.wasmModule!._free(pointer);

	}

	public send(buffer: Uint8Array): void {
		this.socket.send(buffer);
	}

	public kick(): void {
		this.socket.close();
	}

}