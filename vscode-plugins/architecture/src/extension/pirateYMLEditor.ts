import * as path from 'path'
import * as vscode from 'vscode'
import { getNonce } from './util'
import * as fs from "fs"
import * as R from "../shared/viewRequests"
import * as U from "../shared/modelUpdates"
import { parseArchitectureFile } from './parser'
import { rawListeners } from 'process'

/**
 * Bring up a window to show the text document beside this window.
 */
function showDocument(loc: R.VisitURI) {
	const uri = vscode.Uri.file(loc.filename)
	const pos = new vscode.Position(loc.line, loc.column)
	const range = new vscode.Range(pos, pos)
	vscode.window.showTextDocument(uri, {viewColumn: vscode.ViewColumn.Beside, selection: range})
}

/**
 * Provider for PIRATE project files.
 */
export class ProjectEditorProvider implements vscode.CustomTextEditorProvider {

	public static register(context: vscode.ExtensionContext): vscode.Disposable {
		const provider = new ProjectEditorProvider(context)	
        const providerRegistration = vscode.window.registerCustomEditorProvider('pirate.graph', provider)
		return providerRegistration
	}

	constructor(private readonly context: vscode.ExtensionContext) {

	}

	/**
	 * Called when our custom editor is opened.
	 */
	public async resolveCustomTextEditor(
		document: vscode.TextDocument,
		webviewPanel: vscode.WebviewPanel,
		_token: vscode.CancellationToken
	): Promise<void> {
		// Setup initial content for the webview
		webviewPanel.webview.options = {
			// Enable javascript
			enableScripts: true,
			// And restrict the webview to only loading content.
			localResourceRoots: [
				vscode.Uri.file(path.join(this.context.extensionPath, 'webview-static')),
				vscode.Uri.file(path.join(this.context.extensionPath, 'out', 'webview'))
			]
		}

		const c = vscode.window.createOutputChannel('pirate')

		webviewPanel.webview.html = this.getHtmlForWebview(webviewPanel.webview)

		// Receive message from the webview.
		webviewPanel.webview.onDidReceiveMessage((e:R.ViewRequest) => {
			switch (e.tag) {
			case R.Tag.VisitURI:
				showDocument(e)
				break
			}
		})

		const changeDocumentSubscription = vscode.workspace.onDidChangeTextDocument(e => {
			if (e.document.uri.toString() === document.uri.toString()) {
				//updateWebview();
			}
		})

		// Make sure we get rid of the listener when our editor is closed.
		webviewPanel.onDidDispose(() => {
			changeDocumentSubscription.dispose()
		})

		const bytes = document.getText()
		try {
			const res = parseArchitectureFile(bytes, {tabstopWidth: 8})
			for (const e of res.errors)
				c.appendLine(e.start.line + ':' + e.start.column + ': ' + e.message)
			if (res.value) {
				const  m : U.SetSystemLayout = { tag: U.Tag.SetSystemLayout, system: res.value }
				webviewPanel.webview.postMessage(m)
			}
		} catch (e) {
			c.appendLine('Exception thrown: ' + e)
		}
		
	}

	/**
	 * Get the static html used for the editor webviews.
	 */
	private getHtmlForWebview(webview: vscode.Webview): string {
		// Local path to script and css for the webview
		const cssDiskPath    = path.join(this.context.extensionPath, 'webview-static', 'pirateYMLEditor.css')
		const cssResource    = webview.asWebviewUri(vscode.Uri.file(cssDiskPath))
		const scriptDiskPath = path.join(this.context.extensionPath, 'out', 'webview', 'webview', 'pirateYMLEditor.js')
		const scriptResource = webview.asWebviewUri(vscode.Uri.file(scriptDiskPath))
		const htmlBodyDiskPath    = path.join(this.context.extensionPath, 'webview-static', 'pirateYMLEditor.html')
		const htmlBodyContents    = fs.readFileSync(htmlBodyDiskPath, 'utf8')
		// Use a nonce to whitelist which scripts can be run
		const nonce = getNonce()

		return /* html */`
			<!DOCTYPE html>
			<html lang="en">
			<head>
				<meta charset="UTF-8">
				<meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src ${webview.cspSource} 'unsafe-inline'; script-src 'nonce-${nonce}'; "/>
				<meta name="viewport" content="width=device-width, initial-scale=1.0">
				<title>PIRATE Project Viewer</title>
				<link nonce="${nonce}" href="${cssResource}" rel="stylesheet">
			</head>
			<body nonce="${nonce}">
			<div id="lasterrormsg">No error message</div>
			${htmlBodyContents}
			<script type="module" nonce="${nonce}" src="${scriptResource}" />
			</body>
			</html>`
	}
}