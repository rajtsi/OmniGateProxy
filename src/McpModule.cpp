#include "../include/McpModule.h"
#include <iostream>


McpModule::McpModule(int port) : port(port) {
    setup_routes(); 
}

// Store the definition and the executable function
void McpModule::register_tool(const ToolDefinition& def, ToolHandler handler) {
    registered_tools.push_back(def);
    tool_handlers[def.name] = handler;
}

// Define the API behavior
void McpModule::setup_routes() {
    
    // Tool Discovery Route Node.js MCP server will call this to build Zod schemasand register the tools
    CROW_ROUTE(app, "/proxy/tools")([this]() {
        crow::json::wvalue response;
          
        // preparing tools details to return for each tool
        for (size_t i = 0; i < registered_tools.size(); ++i) {  
            const auto& tool = registered_tools[i];
            response["tools"][i]["name"] = tool.name;
            response["tools"][i]["desc"] = tool.description;
            
            for (const auto& pair : tool.params) {
                std::string param_name = pair.first;
                response["tools"][i]["params"][param_name]["type"] = pair.second.type;
                response["tools"][i]["params"][param_name]["description"] = pair.second.description;
            }
        }
        return response;
    });

    //  Tool execuatioon route where evry request will contain tool name and required params
    CROW_ROUTE(app, "/proxy/execute").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON body");

        std::string action = body["action"].s();

        // if the requested tool exists in our dynamic registry
        if (tool_handlers.find(action) != tool_handlers.end()) {
            // Execute the mapped function and pass the arguments
            crow::json::wvalue res = tool_handlers[action](body["args"]);
            return crow::response(200, res);
        }

        return crow::response(404, "Tool not registered in C++");
    });
}

// Run the server
void McpModule::start() {
    std::cout << "[McpModule] Starting MCP Bridge on port " << port << "..." << std::endl;
    app.port(port).multithreaded().run();// api running using multiple threads
}