#include "RequestParser.hpp"
#include <iostream>

int main()
{
    std::string rawRequest =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla\r\n"
        "\r\n";

    RequestParser parser;
    HttpRequest req = parser.parse(rawRequest);

    std::cout << "Method: " << req.method << std::endl;
    std::cout << "Path: " << req.path << std::endl;
    std::cout << "Version: " << req.version << std::endl;

    for (std::map<std::string, std::string>::iterator it = req.headers.begin();
         it != req.headers.end(); ++it)
    {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
}