#include "MethodValidator.hpp"

// Orthodox Canonical Form

MethodValidator::MethodValidator() {
}

MethodValidator::MethodValidator(const MethodValidator& other) {
    (void)other;
}

MethodValidator& MethodValidator::operator=(const MethodValidator& other) {
    if (this != &other) {
    }
    return *this;
}

MethodValidator::~MethodValidator() {
}

bool MethodValidator::isMethodAllowed(const std::string& method, const std::vector<std::string>& allowed_methods)
{
	if (allowed_methods.empty())
    	return (method == "GET");
	bool found = false;
	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (allowed_methods[i] == method)
		{
			found = true;
			break;
		}
	}
	if (found)
		return true;
	return false;
}
