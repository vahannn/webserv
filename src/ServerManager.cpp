/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maharuty <maharuty@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 00:05:52 by dmartiro          #+#    #+#             */
/*   Updated: 2023/12/07 21:51:02 by maharuty         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include "EvManager.hpp"
#include "HelperFunctions.hpp"

bool ServerManager::newClient(int fd) {
    for (int i = 0; i < this->size(); ++i) {
        if ((*this)[i].getfd() == fd) {
            sock_t clientFd = accept((*this)[i].getfd(), 0, 0);
            fcntl(clientFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
            Client *client = new Client(clientFd, (*this)[i].getfd(), (*this)[i]);
            // if (client.getFd() == -1) {  // TODO is it needed
            //     throw std::runtime_error(std::string("accept: ") + strerror(errno));
            // }
            EvManager::addEvent(clientFd, EvManager::read);
            (*this)[i].push(clientFd, client);
            return (true);
        }
    }
    return (false);
}

void ServerManager::start() {
    EvManager::start();

    for (int i = 0; i < this->size(); ++i) {
        EvManager::addEvent((*this)[i].getfd(), EvManager::read);
    }

    while(true) {
        std::pair<EvManager::Flag, int> event;
        Client *client = NULL;
    
        event = EvManager::listen();
        if (newClient(event.second)) {
            continue ;
        }
        for (int i = 0; i < this->size(); ++i) {
            client = (*this)[i].getClient(event.second);
            if (client) {
                break;
            }
        }
        if (client == NULL) {
            continue ;
        }
        try
        {
            if (event.first == EvManager::eof) {
                closeConnetcion(client->getFd());
            } else if (event.first == EvManager::read || client->isRequestReady() == false) {
                if (client->getHttpRequest().empty()) {
                    EvManager::addEvent(client->getFd(), EvManager::write);
                }
                if (client->receiveRequest() == -1) {
                    closeConnetcion(client->getFd());
                }
                if (client->isRequestReady()) {
                    client->parse();
                    client->setResponse(generateResponse(*client));
                }
            } else if (client->isResponseReady() && event.first == EvManager::write) {
                if (client->sendResponse() == true) {
                    closeConnetcion(client->getFd());
                }
            }
        }
        catch(const ResponseError& e)
        {
            client->setResponse(generateErrorResponse(e, *client));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            exit(1);
        }
    }
};


std::string ServerManager::generateErrorResponse(const ResponseError& e, Client &client) {
    // TODO automate it   404, 405, 411, 412, 413, 414, 431, 500, 501, 505, 503, 507, 508
    std::string response;
    std::string resBody;

    if (e.getStatusCode() == 301) {
        client.addHeader(std::make_pair("Location", client.getRedirectPath()));
    }
    try
    {
        resBody = fileToString(client.getSrv().getErrPage(e.getStatusCode()));
    }
    catch(...)
    {
        resBody += "<html>";
		resBody += "<head><title>" + std::to_string(e.getStatusCode()) + " " + e.what() + "</title></head>";
		resBody += "<body>";
		resBody += "<center><h1>" + std::to_string(e.getStatusCode()) + " " + e.what() + "</h1></center><hr>";
		resBody += "</body>";
		resBody += "</html>";
    }

    response = HTTP_VERSION;
    response += " " + std::to_string(e.getStatusCode());
    response += "\r\n";
    client.addHeader(std::pair<std::string, std::string>("Content-Type", "text/html")); // TODO check actual type
    client.addHeader(std::pair<std::string, std::string>("Content-Length", std::to_string(resBody.size())));
    client.buildHeader();
    response += client.getResponse();
    response +=  resBody;
    return (response);
}

std::string ServerManager::generateResponse(Client &client) {
    std::string response;

    client.addHeader(std::make_pair("server", "webserv"));
    response = client.getVersion();
    response += " " + std::string("200") + " ";
    response += SUCCSSES_STATUS;
    response += "\r\n";
    try
    {
        std::vector<HTTPServer>::iterator it = std::find(this->begin(), this->end(), client.getFd());
        if (it == this->end()) {  // TODO never work
            throw std::runtime_error("std::find(this->begin(), this->end(), client.getFd());");
        }
        response += it->processing(client);
    }
    catch(const ResponseError& e)
    {
        response = generateErrorResponse(e, client);
    }
    return (response);
}

bool ServerManager::closeConnetcion(sock_t fd) {
    EvManager::delEvent(fd, EvManager::read);
    EvManager::delEvent(fd, EvManager::write);
    close(fd);
    getServerByClientSocket(fd)->removeClient(fd);
    return (true);
};

ServerManager::ServerManager(const char *configfile)
{
    Parser parser(configfile);
    parser.start(*this);
}

ServerManager::~ServerManager()
{
    
}



/*************************************************************
Finding correct HTTPServer funtions based on ::ServerManager::
**************************************************************/
HTTPServer *ServerManager::getServerBySocket(sock_t fd)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        if (fd == (*this)[i].getfd())
            return (&(*this)[i]);
    }
    return (NULL);
}

HTTPServer *ServerManager::getServerByClientSocket(sock_t fd)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        if ((*this)[i].getClient(fd))
            return (&(*this)[i]);
    }
    throw std::logic_error("getServerByClientSocket");
    return (NULL);
}

int ServerManager::used(HTTPServer &srv) const
{
    for(size_t i = 0; i < this->size(); i++)
        if (std::strcmp((*this)[i].getPort(), srv.getPort()) == 0)
        {
            std::cout << "return (-1);\n";
            return (-1);
        }
    return (0);
}



sock_t ServerManager::findServerBySocket(sock_t issetfd) 
{
    if (issetfd == -1)
        return (-1);
    for(size_t i = 0; i < this->size(); i++)
    {
        HTTPServer server = (*this)[i];
        if (issetfd == server.getfd())
            return (server.getfd());
    }
    return (-1);
}


sock_t ServerManager::findClientBySocket(sock_t issetfd)
{
    if (issetfd == -1)
        return (-1);
    for(size_t i = 0; i < this->size(); i++)
    {
        Client* client = (*this)[i].getClient(issetfd);
        if (client)
            return (client->getFd());
    }
    return (-1);
}



/*******************************************************************
Select Multiplexing  I/O Helper funtions based on ::ServerManager::
*******************************************************************/

int ServerManager::isServer(sock_t fd)
{
    return (0);
}

int ServerManager::isClient(sock_t fd)
{
    return (0);
}

// void ServerManager::push(HTTPServer const &srv)
// {
//     srvs.push_back(srv);
// }

// void ServerManager::push(HTTPServer const &srv)
// {
//     srvs.push_back(srv);
// }