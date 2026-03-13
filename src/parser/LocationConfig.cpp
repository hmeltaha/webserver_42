#include "parser/LocationConfig.hpp"

LocationConfig::LocationConfig():
	root(""),
	path(""),
	index(""),
	redirect(""),
	upload_path(""),
	cgi_path(""),
	cgi_extension(""),
	allowed_methods(),
	autoindex(false)
{}

LocationConfig::LocationConfig(const LocationConfig& other):
	root(other.root),
	path(other.path),
	index(other.index),
	redirect(other.redirect),
	upload_path(other.upload_path),
	cgi_path(other.cgi_path),
	cgi_extension(other.cgi_extension),
	allowed_methods(other.allowed_methods),
	autoindex(other.autoindex)
{}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		root = other.root;
		path = other.path;
		index = other.index;
		redirect = other.redirect;
		upload_path = other.upload_path;
		cgi_path = other.cgi_path;
		cgi_extension = other.cgi_extension;
		allowed_methods = other.allowed_methods;
		autoindex = other.autoindex;
	}
	return *this;
}

LocationConfig::~LocationConfig(){}
