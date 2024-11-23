import type WebSocketServer from "./WebSocketServer";
import type http from "node:http";
import type WebSocket from "ws";

export default class WebSocketClient {

	private readonly cachedReceivedMessages = new Array<Uint8Array>();

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

		this.server.application.wasmModule!._WebSocketClientHandler_EmitOnConnect(this.pointer);

	}

	private onDisconnect(event: WebSocket.CloseEvent): void {

		this.server.application.wasmModule!._WebSocketClientHandler_EmitOnDisconnect(this.pointer);

		delete this.server.clients[this.index];

	}

	private onError(event: WebSocket.ErrorEvent): void {

		this.server.application.wasmModule!._WebSocketClientHandler_EmitOnError(this.pointer);

	}

	private onMessage(data: WebSocket.Data): void {

		if (!(data instanceof ArrayBuffer)) {
			return;
		}

		const buffer = new Uint8Array(data);
		this.cachedReceivedMessages.push(buffer);

		this.emitOnMessage(buffer, false);

	}

	private emitOnMessage(buffer: Uint8Array, isCached: boolean): void {
		const pointer = this.server.application.wasmModule!._malloc(buffer.length);
		this.server.application.wasmModule!.HEAPU8.set(buffer, pointer);
		this.server.application.wasmModule![(!isCached) ? "_WebSocketClientHandler_EmitOnMessage" : "_WebSocketClientHandler_EmitOnMessageInTick"](this.pointer, pointer, buffer.length);
		this.server.application.wasmModule!._free(pointer);
	}

	public send(buffer: Uint8Array): void {
		this.socket.send(buffer);
	}

	public kick(): void {
		this.socket.close();
	}

	public tick(): void {

		for (let i: number = 0; i < this.cachedReceivedMessages.length; i++) {
			const message = this.cachedReceivedMessages.shift()!;
			this.emitOnMessage(message, true);
		}

	}

}