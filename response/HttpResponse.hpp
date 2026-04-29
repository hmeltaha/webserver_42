#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <string>
#include <cstddef>
#include <iostream>
#include <sstream>
#include "../handlers/FileResponse.hpp"

class HttpResponse
{
	public:
		FileResponse response;

		HttpResponse();
		HttpResponse(const HttpResponse& o);
		HttpResponse& operator=(const HttpResponse& o);
		~HttpResponse();
		void setResponse(const FileResponse& res); // remove it later
		std::string getMessage() const;
		std::string getHeaders() const;

};

#endif 
