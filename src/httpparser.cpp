
#include "../include/httpparser.h"

HttpRequest::HttpRequest(const char *raw_buffer)
{
    std::stringstream buffer_stream(raw_buffer);
    std::string line;

    if (std::getline(buffer_stream, line))
    {

        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        std::stringstream first_line_stream(line);

        first_line_stream >> this->method;  // "GET"
        first_line_stream >> this->path;    // "/home"
        first_line_stream >> this->version; // "HTTP/1.1"
    }

    while (std::getline(buffer_stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty())
        {
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 2);
            this->headers[key] = value;
        }
    }
};