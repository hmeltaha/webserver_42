#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : response() {}
HttpResponse::HttpResponse(const HttpResponse& o) : response(o.response) {}
HttpResponse& HttpResponse::operator=(const HttpResponse& o)
{
	if (this != &o)
		response = o.response;
	return *this;
}

HttpResponse::~HttpResponse() {}


void HttpResponse::setResponse(const FileResponse& res)
{
	response = res;
}

std::string HttpResponse::getMessage() const
{
	if (response.status_code == 200)
		return "OK";
	else if (response.status_code == 201)
		return "Created";
	else if (response.status_code == 301)
		return "Moved Permanently";
	else if (response.status_code == 403)
		return "Forbidden";
	else if (response.status_code == 404)
		return "Not Found";
	else if (response.status_code == 405)
		return "Method Not Allowed";
	else if (response.status_code == 413)
			return "Payload Too Large";
	else if (response.status_code == 500)
		return "Internal Server Error";
	return "Unknown";
}

std::string HttpResponse::getHeaders() const
{
	std::ostringstream response_message;
	response_message << "HTTP/1.0 " << response.status_code <<
		" "<< getMessage() << "\r\n";
	response_message << "Content-Type: " << response.mime_type << "\r\n";
	response_message << "Content-Length: " << response.body.size();
	response_message << "\r\nConnection: close\r\n\r\n";
	response_message << response.body;

	return response_message.str();
}
