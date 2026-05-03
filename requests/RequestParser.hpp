#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "HttpRequest.hpp"

class RequestParser
{
public:
    RequestParser();
    RequestParser(const RequestParser& other);
    RequestParser& operator=(const RequestParser& other);
    ~RequestParser();

    HttpRequest parse(const std::string& raw_request);

private:
    bool parse_request_line(const std::string& line, HttpRequest& request);
    bool parse_header(const std::string& line, HttpRequest& request);
    void parse_body(const std::string& raw_request, HttpRequest& request);
};

#endif