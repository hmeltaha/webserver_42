#include "Client.hpp"

Client::Client(/* args */)
{
	clientFd = -1;
	reqBuff = "";
	headersReceived = false;
	state = NEW;
}

Client::~Client()
{
}

Client::Client(int fd) : clientFd(fd)
{
	reqBuff = "";
	headersReceived = false;
	state = NEW;

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

void Client::addToReqBuff(const std::string& buff)
{
	this->reqBuff += buff;
	//Get only
	if (reqBuff.find("\r\n\r\n") != std::string::npos){

		headersReceived = true;
		// state = PROCESSING;
	}
	//post only we must read the content length and wait until we get all the body

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