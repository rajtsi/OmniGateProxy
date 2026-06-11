#include "../include/ProxyServer.h"
#include <iostream>
#include "../include/McpModule.h"
#include "../include/ToolRegistry.h"
#include <thread>

int main(int argc, char *argv[])
{
    std::string config_path = (argc > 1) ? argv[1] : "../config.json";
    
  
    McpModule mcp(9090);
    register_all_mcp_tools(mcp);

    // Start the MCP Module in a background thread
    std::thread mcp_thread(&McpModule::start, &mcp);
    mcp_thread.detach(); // Detach allows it to run independently in the background

    // 4. Initialized and start the main proxy server 
    OmniGate &myserver = OmniGate::getInstance();
    myserver.load_config(config_path);
    
    std::cout << "[Main] Starting OmniGate Proxy loop..." << std::endl;
    myserver.start_server(); 

    return 0;
}