#pragma once

#include "crow_all.h"
#include <string>
#include <vector>
#include <map>
#include <functional> // c++ library to hold functions 

// Structure to hold parameter metadata for the AI schema

struct ToolParam {
    std::string type;
    std::string description;
};

// ToolDefinition holds full definition of a Tool
// Example 
// tool name: BMI
// tool Description: Its for BMI calculation
// params : {"height":{parameter type:number, parameter: details about parameter liek its height of person / or time ect}...}

struct ToolDefinition {
    std::string name;
    std::string description;
    std::map<std::string, ToolParam> params;
};

class McpModule {
private:
    int port;
    crow::SimpleApp app;
    std::vector<ToolDefinition> registered_tools;
    using ToolHandler = std::function<crow::json::wvalue(const crow::json::rvalue&)>;
    // This map will store tool's string name and corresponding tool function
    std::map<std::string, ToolHandler> tool_handlers;

    // Private method to initialize the API endpoints
    void setup_routes();

public:
    McpModule(int port = 9090);
    //function to dynamically add tools
    void register_tool(const ToolDefinition& def, ToolHandler handler);

    // Starts MCP serving Proxy server
    void start();
};

