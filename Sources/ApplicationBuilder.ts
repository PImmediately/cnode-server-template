import esbuild from "esbuild";
import path from "node:path";
import fs from "node:fs";

import CleanCSS from "clean-css";
import htmlMinifier from "html-minifier";

import * as crypto from "node:crypto";

function sha1(data: string): string {
	return crypto.createHash("sha1").update(data).digest("hex");
}

class ApplicationBuilder {

	private hash: (string | undefined);

	private readonly sourceDirectory = path.resolve(__dirname, "./../Sources");
	private readonly buildDirectory = (this.sourceDirectory + "/../build");

	private readonly serverCPPJavaScriptPath = (this.buildDirectory + "/server_cpp.js");
	private readonly serverCPPWebAssemblyPath = (this.buildDirectory + "/server_cpp.wasm");
	private readonly serverCPPBundleJavaScriptPath = (this.buildDirectory + "/server_cpp.bundle.js");

	private readonly serverNodeJavaScriptPath = (this.buildDirectory + "/server_node.js");
	private readonly serverNodeBundleJavaScriptPath = (this.buildDirectory + "/server_node.bundle.js");

	public constructor() {
	}

	public async build(): Promise<void> {

		this.hash = sha1(new Date().getTime().toString());

		this.buildServerCPPBundleJavaScript();
		await this.buildServerNodeJavaScript();

	}

	private applyVersionHash(code: string): string {
		return code.replaceAll("@server::version_hash", this.hash!);
	}

	private buildServerCPPBundleJavaScript(): void {

		const serverCPPWebAssemblyBase64Encoded = Buffer.from(
			fs.readFileSync(this.serverCPPWebAssemblyPath)
		).toString("base64");

		let code: string = fs.readFileSync(this.serverCPPJavaScriptPath, "utf-8");

		{

			const targetCode = `function getBinaryPromise(binaryFile) {`;
			if (code.indexOf(targetCode) === -1) {
				throw new Error("Couldn't find 'getBinaryPromise'.");
			}

			const addedCode = (
				`const buffer = Buffer.from("${serverCPPWebAssemblyBase64Encoded}", "base64");` + "\n" +
				`return new Promise((resolve) => resolve(buffer));`
			);

			code = code.replace(
				targetCode,
				(targetCode + "\n" + addedCode)
			);

		}

		{

			const targetCode = `var stringToNewUTF8`;
			const replacementCode = `Module["_stringToNewUTF8"]`;

			const at = code.indexOf(targetCode);
			if (at === -1) {
				throw new Error("Couldn't find 'stringToNewUTF8'.");
			}

			code = code.replace(targetCode, replacementCode);

		}

		{

			const targetCode = `if (typeof exports === 'object' && typeof module === 'object')`;

			const at = code.indexOf(targetCode);
			if (at === -1) {
				throw new Error("Couldn't find exporter.");
			}

			code = code.slice(0, at);

		}

		fs.writeFileSync(this.serverCPPBundleJavaScriptPath, code, "utf-8");

	}

	private async buildServerNodeJavaScript(): Promise<void> {

		await esbuild.build({
			bundle: true,
			entryPoints: [
				(this.sourceDirectory + "/Application.ts")
			],
			external: [
				"ws"
			],
			logLevel: "debug",
			outbase: this.sourceDirectory,
			outfile: this.serverNodeJavaScriptPath,
			platform: "node",
			plugins: [
				{
					name: "version-hash",
					setup: (build) => {
						build.onLoad({ filter: /\.ts$/ }, async (args) => {
							let code: string = fs.readFileSync(args.path, "utf8");
							code = this.applyVersionHash(code);
							return {
								contents: code,
								loader: "ts"
							};
						});
					}
				}
			]
		});

		let code: string = fs.readFileSync(this.serverNodeJavaScriptPath, "utf-8");

		{

			const targetCode = `"@server::server_cpp.bundle.js";`;
			const replacementCode = fs.readFileSync(this.serverCPPBundleJavaScriptPath, "utf-8");

			const at = code.indexOf(targetCode);
			if (at === -1) {
				throw new Error("Couldn't find '@server::server_cpp.bundle.js'");
			}

			code = (
				code.slice(0, at) +
				replacementCode +
				code.slice(at + targetCode.length)
			);

		}

		{

			const targetCode = `@client::index.html.base64`;
			const replacementCode = Buffer.from(
				this.getIndexHTML()
			).toString("base64");

			if (code.indexOf(targetCode) === -1) {
				throw new Error("Couldn't find '@client::index.html.base64'");
			}

			code = code.replace(targetCode, replacementCode);

		}

		{

			const targetCode = `@client::client_cpp.js.base64`;
			const replacementCode = Buffer.from(
				this.applyVersionHash(
					fs.readFileSync((this.buildDirectory + "/client_cpp.js"), "utf-8")
						.replaceAll(`client_cpp.wasm`, `client.wasm`)
				)
			).toString("base64");

			if (code.indexOf(targetCode) === -1) {
				throw new Error("Couldn't find '@client::client_cpp.js.base64'");
			}

			code = code.replace(targetCode, replacementCode);

		}

		{

			const targetCode = `@client::client_cpp.wasm.base64`;
			const replacementCode = Buffer.from(
				fs.readFileSync(this.buildDirectory + "/client_cpp.wasm")
			).toString("base64");

			if (code.indexOf(targetCode) === -1) {
				throw new Error("Couldn't find '@client::client_cpp.wasm.base64'");
			}

			code = code.replace(targetCode, replacementCode);

		}

		fs.writeFileSync(this.serverNodeBundleJavaScriptPath, code, "utf-8");

	}

	private getIndexHTML(): string {

		const cleanCSS = new CleanCSS({
			level: 2
		});
		const style = fs.readFileSync((this.sourceDirectory + "/Server/HTTP/Public/style.css"), "utf-8");

		let code: string = fs.readFileSync((this.sourceDirectory + "/Server/HTTP/Public/index.html"), "utf-8");
		code = code.replace(`<style></style>`, `<style>${cleanCSS.minify(style).styles}</style>`);
		code = htmlMinifier.minify(code, {
			collapseWhitespace: true,
			preserveLineBreaks: true,
			removeComments: true,
			removeTagWhitespace: true
		});

		code = code.replace(/^\s*[\r\n]/gm, "");

		code = this.applyVersionHash(code);

		return code;

	}

}

(async () => {

	const builder = new ApplicationBuilder();
	await builder.build();

})();