#include "LocationConfig.hpp"

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

LocationConfig::~LocationConfig(){}

