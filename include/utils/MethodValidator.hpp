#ifndef METHODVALIDATOR_HPP
#define METHODVALIDATOR_HPP

#include <string>
#include <vector>

class MethodValidator
{
	public:

    	MethodValidator();
    	MethodValidator(const MethodValidator& other);
    	MethodValidator& operator=(const MethodValidator& other);
    	~MethodValidator();

	bool isMethodAllowed(const std::string& method, const std::vector<std::string>& allowed_methods);

	private:
};

#endif
