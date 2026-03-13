#include "server/Client.hpp"
#include <cstdlib>

Client::Client()
{
        clientFd = -1;
        reqBuff = "";
        headersReceived = false;
        state = READING;
}

Client::~Client()
{
}

Client::Client(int fd) : clientFd(fd)
{
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
        this->resBuff = other.resBuff;
}

Client& Client::operator=(const Client& other)
{
        if (this != &other)
        {
                this->clientFd = other.clientFd;
                this->reqBuff = other.reqBuff;
                this->headersReceived = other.headersReceived;
                this->state = other.state;
                this->resBuff = other.resBuff;
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
        if (state == READING || state == READING_BODY)
        {
                if (reqBuff.find("\r\n\r\n") != std::string::npos)
                {
                        headersReceived = true;
                        state = PROCESSING;
                }
                if (reqBuff.find("Content-Length:") != std::string::npos)
                        state = READING_BODY;
        }
}

void Client::addBodyToReq()
{
        size_t pos;
        size_t end_pos;
        int body_len;

        pos = reqBuff.find("Content-Length:") + 15;
        end_pos = reqBuff.find("\r\n", pos);
        // atoi used instead of std::stoi for C++98 compliance
        body_len = atoi(reqBuff.substr(pos, end_pos - pos).c_str());
        size_t total_len = reqBuff.find("\r\n\r\n") + 4 + body_len;
        if (reqBuff.length() >= total_len)
        {
                state = PROCESSING;
                reqBuff.resize(total_len);
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
