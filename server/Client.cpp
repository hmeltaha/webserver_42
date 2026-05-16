#include "Client.hpp"

Client::Client(/* args */)
{
	clientFd = -1;
	reqBuff = "";
	state = READING;
	len_body = 0;
	body = "";
	bytes_send = 0;
	server_to_connect = -1;
	payload_too_large = false;
}

Client::~Client()
{
}

Client::Client(int fd, int server_index) : clientFd(fd)
{
	body = "";
	len_body = 0;
	reqBuff = "";
	state = READING;
	bytes_send = 0;
	server_to_connect = server_index;
	payload_too_large = false;
	start_time = time(NULL);
}

int Client::getServerToConnect() const
{
	return server_to_connect;
}

Client::Client(const Client& other)
{
	this->clientFd = other.clientFd;
	this->reqBuff = other.reqBuff;
	this->state = other.state;
	this->bytes_send = 0;
	this->server_to_connect = other.server_to_connect;
	this->payload_too_large = other.payload_too_large;
	start_time = other.start_time;
}

Client& Client::operator=(const Client& other)
{
	if (this != &other)
	{
		this->clientFd = other.clientFd;
		this->reqBuff = other.reqBuff;
		this->state = other.state;
		this->bytes_send = 0;
		this->server_to_connect = other.server_to_connect;
		this->payload_too_large = other.payload_too_large;
		start_time = other.start_time;
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

size_t Client::getBytesSend() const
{
	return bytes_send;
}

void Client::setBytesSend(size_t size)
{
	bytes_send = size;
}


void Client::setReqBuff(const std::string& buff)
{
	this->reqBuff = buff;
}

std::string Client::getReqBuff() const
{
	return reqBuff;
}

bool Client::getPayloadTooLarge() const
{
	return payload_too_large;
}

void Client::addToReqBuff(const std::string& buff, const ServerConfig& config)
{
	if (state != READING && state != READING_BODY)
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
		body = reqBuff.substr(headerEnd + 4);
		if (start != std::string::npos)
			lenStr = lenStr.substr(start, end - start);
		len_body = std::atoi(lenStr.c_str());
		if (len_body > config.client_max_body_size)
		{
			payload_too_large = true;
			state = PROCESSING;
			return;
		}
		size_t len_of_recv_body = reqBuff.length() - (headerEnd + 4);
		if (len_of_recv_body >= len_body)
			state = PROCESSING;
		else
			state = READING_BODY;
	}
	else
		state = PROCESSING;
}

time_t Client::getStartTime() const
{
	return start_time;
}

void Client::setStartTime(time_t time)
{
	this->start_time = time;
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

std::string& Client::getResBuff() const
{
	return const_cast<std::string&>(resBuff);
}

