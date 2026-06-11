#include "../include/ToolRegistry.h"

void register_all_mcp_tools(McpModule& mcp) {
    
    mcp.register_tool(
        {"proxy-health-check", "Check Proxy Health", {}},
        [](const crow::json::rvalue& args) -> crow::json::wvalue {
            crow::json::wvalue res;
            res["status"] = "Success";
            res["is_healthy"] = true;
            return res;
        }
    );

    
}