#ifndef FILERESPONSE_HPP
#define FILERESPONSE_HPP

#include <string>
#include <cstddef>
/**
 * 
 * response information:
 * status_code: HTTP status code (e.g., 200, 404, 500)
 * mime_type: MIME type of the response (e.g., "text/html", "application, json")
 * body: The content of the response, which can be an HTML page, JSON data, or any other type of content depending on the request and the server's response.
 * content_length: The size of the response body in bytes. This is important for the client to know how much data to expect in the response body.
 */
struct FileResponse
{
	int status_code;
	std::string mime_type;
	std::string body;
	size_t content_length;

	FileResponse() : status_code(500), content_length(0) {}
};

#endif