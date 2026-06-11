import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js"; // for local transport
import * as z from 'zod';// for scheama verification
import dotenv from 'dotenv'; 
import path from 'path';
dotenv.config({ path: path.resolve(import.meta.dirname, '.env') });
const server = new McpServer({ name: 'OmnigateProxy_mcp', version: '1.0.0' });
const proxy_url = process.env.PROXY_URL;
console.error(proxy_url, "jajajajjajaj");
async function initializeDynamicTools() { // its a function taht will get all the tools from our c++ code using api call and will register it to our js mcp server
    let toolsFromCpp = [];
    // 1. Fetch the actual tools schema from your C++ OmniGate Proxy
    try {
        console.error("Fetching tool schemas from C++ OmniGate API (port 9090)...");//using error to print on consol because log will send to stdout which is used for conversation between ai and mcp 

        const response = await fetch(proxy_url + "proxy/tools");
        if (!response.ok) {
            throw new Error(`HTTP Error from C++ proxy API server: ${response.status}`);
        }

        const data = await response.json();
        toolsFromCpp = data.tools || [];
        console.error(`Successfully loaded ${toolsFromCpp.length} tools from proxy server!`);

    } catch (error) {
        console.error("Unable to connect to Proxy server ", error.message);
        process.exit(1); // Stop the Node.js server if it can't connect to proxy server
    }

    // 2. Looping through each tool fetched from the proxy mcp registry
    for (const tool of toolsFromCpp) {
        const schemaShape = {};
        // If the tool has parameters, dynamically mapped them to Zod schemas
        if (tool.params) {
            for (const [paramName, paramDetails] of Object.entries(tool.params)) {
                let zodType;
                switch (paramDetails.type) {
                    case "string":
                        zodType = z.string();
                        break;
                    case "number":
                        zodType = z.number();
                        break;
                    case "boolean":
                        zodType = z.boolean();
                        break;
                    default:
                        zodType = z.any();
                }

                if (paramDetails.description) {
                    zodType = zodType.describe(paramDetails.description);
                }

                schemaShape[paramName] = zodType;
            }
        }

        // 3. Dynamically registered the tool with MCP
        server.registerTool(
            tool.name,
            {
                title: tool.name,
                description: tool.desc,
                inputSchema: z.object(schemaShape)
            },
            async (args) => {
                console.error(`[Execution] Forwarding '${tool.name}' to to proxy server with args:`, args);

                try {
                    const response = await fetch(proxy_url + "proxy/execute", {
                        method: "POST",
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify({ action: tool.name, args })
                    });

                    const output = await response.json();

                    return {
                        content: [{ type: 'text', text: JSON.stringify(output) }]
                    };
                } catch (error) {
                    console.error(`Failed to execute '${tool.name}' via C++:`, error.message);
                    return {
                        isError: true,
                        content: [{ type: 'text', text: `Execution failed: ${error.message}` }]
                    };
                }
            }
        );
    }
}

//Registering all the tools before stream starts
await initializeDynamicTools();

// Setup the Standard I/O transport pipelines for the host client interface
const transport = new StdioServerTransport();
await server.connect(transport);
console.error("OmniGate MCP Server Bridge is live and ready!");