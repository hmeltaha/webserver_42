#ifndef FILERESPONSE_HPP
#define FILERESPONSE_HPP

#include <string>
#include <cstddef>

struct FileResponse
{
    int status_code;
    std::string mime_type;
    std::string body;
    size_t content_length;

    FileResponse() : status_code(500), content_length(0) {}
};

#endif