/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maharuty <maharuty@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 10:29:55 by dmartiro          #+#    #+#             */
/*   Updated: 2023/12/12 21:55:22 by maharuty         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(sock_t clfd, sock_t srfd, HTTPServer &srv) : _defaultSrv(srv)
{
    this->fd = clfd;
    this->serverFd = srfd;
    this->rd = 0;
    _subSrv = NULL;
}

Client::~Client()
{
}

sock_t Client::getFd( void ) const
{
    return (this->fd);
}

sock_t Client::getServerFd( void ) const
{
    return (this->serverFd);
}

std::string Client::getServerPort( void ) const {
    return (_defaultSrv.getPort());
};

int Client::receiveRequest() {
    char buf[READ_BUFFER];
    errno = 0;
    int rdSize = recv(fd, buf, sizeof(buf), 0);
    if (rdSize == -1) { 
        return (0);
    }
    if (rdSize == 0) {
        return (-1);
    }
    if (_isHeaderReady == false) {
        httpRequest.append(buf, rdSize);
        size_t headerEndPos = httpRequest.find("\r\n\r\n");
        if (headerEndPos == std::string::npos) {
            return 0;
        }
        _isHeaderReady = true;

        std::string tmpBody = httpRequest.substr(headerEndPos + strlen("\r\n\r\n"));
        httpRequest.erase(headerEndPos);
        this->parseHeader();
        std::map<std::string, std::string>::const_iterator it = httpHeaders.find("Content-Length");
        size_t pos = httpRequest.find("Content-Length: ");
        if (it == httpHeaders.end()) {
            _bodySize = 0;
        } else {
            char *ptr;
            _bodySize = std::strtoul(it->second.c_str(), &ptr, 10);
            if (_bodySize > this->getSrv().getClientBodySize()) { // TODO cant be done here still not determined wich server will serve for client
                throw ResponseError(413, "Content Too Large");
            }
        }
        if (_bodySize != 0) {
            _body = tmpBody;
            if (_bodySize <= _body.size()) {
                _body.erase(_bodySize);
                _isBodyReady = true;
                _isRequestReady = true;
            }
            
        } else {
            _isBodyReady = true;
            _isRequestReady = true;
        }
        return 0;
    }
    _body.append(buf, rdSize);
    if (_bodySize <= _body.size()) {
        _body.erase(_bodySize);
        _isBodyReady = true;
        _isRequestReady = true;
    }
    return 0;
}

void Client::parseHeader()
{
    size_t space = 0;
    size_t pos = httpRequest.find("\r\n");
    request = httpRequest.substr(0, pos);
    httpRequest.erase(0, pos + 2);

    for (size_t i = 0; i < request.size(); i++)
        if (std::isspace(request[i]))
            space++;
    if (space == 2)
    {
        method = trim(request.substr(0, request.find_first_of(" ")));
        request.erase(0, request.find_first_of(" ") + 1);
        _path = trim(request.substr(0, request.find_first_of(" ")));// TODO handle ? var cases in _path
        request.erase(0, request.find_first_of(" ") + 1);
        version = trim(request.substr(0, request.find("\r\n")));
    }
    std::stringstream iss(httpRequest);
    std::string get_next_line;

    while (std::getline(iss, get_next_line) && get_next_line != "\r\n")
    {
        size_t colon;
        if ((colon = get_next_line.find_first_of(":")) != std::string::npos && std::isspace(get_next_line[colon+1]))
        {
            std::string key = trim(get_next_line.substr(0, colon));
            std::string value = trim(get_next_line.substr(colon+2, get_next_line.find("\r\n")));
            httpHeaders.insert(std::make_pair(key, value));
        }
    }
    httpRequest.clear();
    std::map<std::string, std::string>::iterator it = httpHeaders.find("Host");
    if (it != httpHeaders.end()) {
        _subSrv = _defaultSrv.getSubServerByName(it->second);
    }
    HTTPRequest::checkPath(this->getSrv());
}

void Client::parseBody()
{
    if (method == "POST") {
        if (_isCgi == true) {

        }
        multipart();
    }
}


bool Client::sendResponse() {
    if (_responseLine.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _responseLine.size() ? WRITE_BUFFER : _responseLine.size();
        if (send(fd, _responseLine.c_str(), sendSize, 0) == -1) {
            return (false); // TODO is send function return -1 seting EAGAIN in errno
        }
        _responseLine.erase(0, sendSize);

    }
    else if (_header.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _header.size() ? WRITE_BUFFER : _header.size();
        if (send(fd, _header.c_str(), sendSize, 0) == -1) {
            return (false); // TODO is send function return -1 seting EAGAIN in errno
        }
        _header.erase(0, sendSize);
    } else if (_responseBody.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _responseBody.size() ? WRITE_BUFFER : _responseBody.size();
        if (send(fd, _responseBody.c_str(), sendSize, 0) == -1) {
            return (false); // TODO is send function return -1 seting EAGAIN in errno
        }
        _responseBody.erase(0, sendSize);
    }
    if (_responseBody.empty() == true) {
        _isResponseReady = false;
    }
    // _response.clear();
    return (_responseBody.empty() && _header.empty() && _responseLine.empty());
}



const HTTPServer &Client::getSrv( void ) const {
    if (_subSrv) {
        return (*_subSrv);
    }
    return (_defaultSrv);
};

HTTPServer &Client::getSrv( void ) {
    if (_subSrv) {
        return (*_subSrv);
    }
    return (_defaultSrv);
};

void Client::setResponseLine(std::string const &line) {
    _responseLine = line;
};