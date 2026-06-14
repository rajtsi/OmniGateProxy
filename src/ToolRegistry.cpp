#include "../include/MetricsTracker.hpp"
#include "../include/RpsTracker.hpp"
#include "../include/ToolRegistry.h"

void register_all_mcp_tools(McpModule &mcp)
{

    mcp.register_tool(
        {"proxy-health-check", "Check Proxy Health", {}},
        [](const crow::json::rvalue &args) -> crow::json::wvalue
        {
            crow::json::wvalue response;

            response["status"] = "Healthy";
            response["mcp_port"] = 9090;
            response["message"] = "OmniGate proxy is actively running and the MCP bridge is fully functional";
            return response;
        });

    mcp.register_tool(
        {"proxy-metrics", "Get real-time proxy metrics", {}},
        [](const crow::json::rvalue &args) -> crow::json::wvalue
        {
            crow::json::wvalue response;

            response["rps"] = RpsTracker::getInstance().getRps();
            response["status_2xx"] = MetricsTracker::getInstance().status_2xx.load();
            response["status_4xx"] = MetricsTracker::getInstance().status_4xx.load();
            response["status_5xx"] = MetricsTracker::getInstance().status_5xx.load();
            response["total_requests"] = MetricsTracker::getInstance().total_requests.load();
            response["active_connections"] = MetricsTracker::getInstance().active_connections.load();

            return response;
        });
}