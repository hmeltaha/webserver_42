#include "Client.hpp"

Client::Client(/* args */)
{
	clientFd = -1;
	reqBuff = "";
	headersReceived = false;
	state = READING;
	len_body = 0;
	body = "";
}

Client::~Client()
{
}

Client::Client(int fd) : clientFd(fd)
{
	body = "";

	len_body = 0;
	reqBuff = "";
	headersReceived = false;
	state = READING;

}

Client::Client(const Client& other) 
{
	this->clientFd = other.clientFd;
	this->reqBuff = other.reqBuff;
	this->headersReceived = other.headersReceived;
	this->state = other.state;
}

Client& Client::operator=(const Client& other)
{
	if (this != &other)
	{
		this->clientFd = other.clientFd;
		this->reqBuff = other.reqBuff;
		this->headersReceived = other.headersReceived;
		this->state = other.state;
	}
	return *this;
}


int Client::getClientFd() const
{
	return clientFd;
}

void Client::setClientFd(int fd)
{
	this->clientFd = fd;
}

void Client::setReqBuff(const std::string& buff)
{
	this->reqBuff = buff;
}

std::string Client::getReqBuff() const
{
	return reqBuff;
}

// void Client::addToReqBuff(const std::string& buff)
// {
// 	if (state == READING)
// 	{
// 		this->reqBuff += buff;
// 		if (reqBuff.find("\r\n\r\n") != std::string::npos){
	
// 			headersReceived = true;
// 			state = PROCESSING;
// 		}
// 		if (reqBuff.find("Content-Length:") != std::string::npos)
// 		{
// 			size_t pos = reqBuff.find("Content-Length:");
// 			std::string lenStr = reqBuff.substr(pos + 15);
// 			size_t start = lenStr.find_first_not_of(" ");
// 			size_t end = lenStr.find("\r\n");

// 			if (start != std::string::npos && end != std::string::npos)
// 				lenStr = lenStr.substr(start, end - start);
// 			len_body = std::atoi(lenStr.c_str());
// 			state = READING_BODY;
// 		}
// 	}
// }

void Client::addToReqBuff(const std::string& buff)
{
	if (state != READING)
		return;
	reqBuff += buff;
	size_t headerEnd = reqBuff.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return;
	size_t pos = reqBuff.find("Content-Length:");
	if (pos != std::string::npos)
	{
		std::string lenStr = reqBuff.substr(pos + 15);
		size_t start = lenStr.find_first_not_of(" ");
		size_t end = lenStr.find("\r\n");
		if (start != std::string::npos)
			lenStr = lenStr.substr(start, end - start);
		len_body = std::atoi(lenStr.c_str());
		state = READING_BODY;
	}
	else
		state = PROCESSING;
}

void Client::addBodyToReq(const std::string& buff)
{
	body += buff;
	if (static_cast<int>(body.length()) > len_body)
		body.resize(len_body);
	if (static_cast<int>(body.length()) >= len_body)
	{
		this->reqBuff += body;
		state = PROCESSING;
		std::cout << "BBBOOOOODDYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY" << std::endl;
		std::cout << "\"" << buff << "\"" << std::endl;
	}
}

bool Client::getHeadersReceived() const 
{
	return headersReceived;
}

ClientState Client::getState() const
{
	return state;
}

void Client::setState(ClientState newState)
{
	this->state = newState;
}

void Client::setResBuff(const std::string& buff)
{
	this->resBuff = buff;
}

std::string Client::getResBuff() const
{
	return resBuff;
}

