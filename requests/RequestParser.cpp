#include "RequestParser.hpp"
#include <sstream>
#include <cctype>
#include <cstdlib>

RequestParser::RequestParser()
{
}

RequestParser::RequestParser(const RequestParser& other)
{
    (void)other;
}

RequestParser& RequestParser::operator=(const RequestParser& other)
{
    (void)other;
    return *this;
}

RequestParser::~RequestParser()
{
}

// HELPERS

static std::string trim_whitespace(const std::string& s)
{
    size_t start = 0;
    size_t end   = s.size();

    while (start < end && (s[start] == ' ' || s[start] == '\t'))
        start++;

    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;

    return s.substr(start, end - start);
}


static std::string to_lower(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}

static void remove_cr(std::string& line)
{
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
}

// parsing

bool RequestParser::parse_request_line(const std::string& line, HttpRequest& request)
{
    std::istringstream ss(line);

    if (!(ss >> request.method >> request.path >> request.version))
        return false;

    if (request.version != "HTTP/1.0" && request.version != "HTTP/1.1")
        return false;

    return true;
}

bool RequestParser::parse_header(const std::string& line, HttpRequest& request)
{
    size_t colon = line.find(':');
    if (colon == std::string::npos)
        return false;

    std::string name  = to_lower(trim_whitespace(line.substr(0, colon)));
    std::string value = trim_whitespace(line.substr(colon + 1));

    if (name.empty())
        return false;

    if (request.headers.count(name))
        request.headers[name] += ", " + value;
    else
        request.headers[name] = value;

    return true;
}

void RequestParser::parse_body(const std::string& raw_request, HttpRequest& request)
{
    if (!request.headers.count("content-length"))
        return;

    size_t content_length = static_cast<size_t>(
        std::atoi(request.headers["content-length"].c_str())
    );

    if (content_length == 0)
        return;

    size_t body_start = raw_request.find("\r\n\r\n");
    if (body_start != std::string::npos)
        body_start += 4;
    else
    {
        body_start = raw_request.find("\n\n");
        if (body_start != std::string::npos)
            body_start += 2;
        else
            return;
    }

    if (raw_request.size() - body_start >= content_length)
        request.body = raw_request.substr(body_start, content_length);
}

// main
//cr : carriage return /n and /r
HttpRequest RequestParser::parse(const std::string& raw_request)
{
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;

    // 1. Request line
    if (!std::getline(stream, line))
        return request;
    remove_cr(line);

    if (line.empty() || !parse_request_line(line, request))
        return request;

    //store full URI before modifying anything
    request.uri = request.path;

    // query string parsing
    size_t qpos = request.path.find('?');
    if (qpos != std::string::npos)
    {
        request.query = request.path.substr(qpos + 1);
        request.path  = request.path.substr(0, qpos);
    }
    else
    {
        request.query = "";
    }

    // 2. Headers
    while (std::getline(stream, line))
    {
        remove_cr(line);

        if (line.empty())
            break;

        parse_header(line, request);
    }

    // 3. Body
    parse_body(raw_request, request);
    return request;
}


// GET /index.html HTTP/1.1\r\n          ← request line
// Host: localhost:8080\r\n              ← header
// Content-Type: application/json\r\n   ← header
// Content-Length: 27\r\n               ← header
// \r\n                                  ← blank line (separator)
// {"username": "hala", "age": 22}      ← body
