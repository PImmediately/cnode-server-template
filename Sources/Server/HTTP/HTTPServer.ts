import type Application from "./../../Application";

import http from "node:http";
import url from "node:url";

import type { DeepReadonly } from "./../../Shared/TypeScript/UtilityTypes";

export interface HTTPServerOptions {
	port: number;
}

const INDEX_HTML = Buffer.from("@client::index.html.base64", "base64").toString();
const CLIENT_CPP_JAVASCRIPT = Buffer.from("@client::client_cpp.js.base64", "base64").toString();
const CLIENT_CPP_WEBASSEMBLY = Buffer.from("@client::client_cpp.wasm.base64", "base64");

export default class HTTPServer {

	private _server: (http.Server | undefined);
	public get server(): (http.Server | undefined) {
		return this._server;
	}

	public constructor(public application: Application, public readonly options: DeepReadonly<HTTPServerOptions>) {
	}

	public async create(): Promise<void> {

		this._server = http.createServer(async (request, response) => {

			const pathname = url.parse(request.url!).pathname!;
			if (pathname === "/") {
				response.statusCode = 200;
				response.statusMessage = "OK";
				response.setHeader("Content-Type", "text/html; charset=utf-8");
				response.end(INDEX_HTML);
				return;
			} else if (pathname === "/@server::version_hash/client.js") {
				response.statusCode = 200;
				response.statusMessage = "OK";
				response.setHeader("Content-Type", "text/javascript; charset=utf-8");
				response.end(CLIENT_CPP_JAVASCRIPT);
				return;
			} else if (pathname === "/@server::version_hash/client.wasm") {
				response.statusCode = 200;
				response.statusMessage = "OK";
				response.setHeader("Content-Type", "application/wasm");
				response.end(CLIENT_CPP_WEBASSEMBLY);
				return;
			}

			response.statusCode = 404;
			response.statusMessage = "Not Found";
			response.setHeader("Content-Type", "text/html; charset=utf-8");
			response.end(
				`<html>` + "\n" +
				"\t" + `<head>` + "\n" +
				"\t" + "\t" + `<title>CNode-Server Template</title>` + "\n" +
				"\t" + `</head>` + "\n" +
				"\t" + `<body>` + "\n" +
				"\t" + "\t" + `<h1>` + response.statusCode + " " + response.statusMessage + `</h1>` + "\n" +
				"\t" + `</body>` + "\n" +
				`</html>`
			);

		});

		return new Promise<void>((resolve, reject) => {
			this.server!.listen(this.options.port, () => {
				resolve();
			});
		});

	}

}