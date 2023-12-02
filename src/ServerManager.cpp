/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmartiro <dmartiro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/14 00:05:52 by dmartiro          #+#    #+#             */
/*   Updated: 2023/11/18 15:19:40 by dmartiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include "EvManager.hpp"

bool ServerManager::newClient(int fd) {
    for (int i = 0; i < this->size(); ++i) {
        if ((*this)[i].getfd() == fd) {
            sock_t clientFd = accept((*this)[i].getfd(), 0, 0);
            fcntl(clientFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
            Client *client = new Client(clientFd, (*this)[i].getfd());  // TODO delete in destructor
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
        std::pair<EvManager::Flag, int> event = EvManager::listen();
        std::cout << "event.second = " << event.second << std::endl;
        std::cout << "event.first = " << event.first << std::endl;
        if (newClient(event.second)) {
            continue ;
        }
        // // std::cout << "else\n";
        Client *client;
        std::cout << "event.second = " << event.second << std::endl;
        for (int i = 0; i < this->size(); ++i) {
            client = (*this)[i].getClient(event.second);
            if (client) {
                break;
            }
        }
        if (client == NULL) {  //TODO probably it never works
            throw std::runtime_error("client == NULL");
        }
        std::cout << "client->isResponseReady() = " << client->isResponseReady() << std::endl;
        if (event.first == EvManager::eof) {
            // std::cout << "\nEV_EOF\n" << std::endl;
            closeConnetcion(client->getFd());
        } else if (event.first == EvManager::read) {
            // std::cout << "\nEVFILT_READ\n" << std::endl;
            if (client->getHttpRequest().empty()) {
                EvManager::addEvent(client->getFd(), EvManager::write);
            }
            if (client->receiveRequest() == -1) {
                closeConnetcion(client->getFd());
            }
            if (client->isRequestReady()) {
                client->parse();
                client->setResponse(generateResponse(client);
            }
        } else if (client->isResponseReady() && event.first == EvManager::write) {
            std::cout << "event.first == EvManager::write\n";
            // std::cout << "\nEVFILT_WRITE\n" << std::endl;
            // TODO send response little by little
            if (client->sendResponse() == true) {
                closeConnetcion(client->getFd());
            }
        } else if (client->isResponseReady() == false) {
            if (client->receiveRequest() == -1) {
                closeConnetcion(client->getFd());
            }
            // std::cout << client->getHttpRequest() << std::endl;
            if (client->isRequestReady()) {
                client->parse();
                client->setResponse(generateResponse(client));
            }
        }
    }
};

std::string ServerManager::generateResponse(Client &client) {
    const std::string &httpRequest = clie;
    const std::string &body;
    std::map<std::string, std::string> const &headers;
    std::string response;
    
    std::unordered_map<std::string, std::string> headerContent;

    headerContent.insert(std::make_pair("server", "webserv"));
    response = version;
    response += " " + reqStatus + " ";
    response += SUCCSSES_STATUS;
    response += "\r\n";
    try
    {
        if (method == "POST") {
            response += post(client);
        } else if (method == "GET") {
            response += get(client);
        } else if (method == "DELETE") {
            response += del(client);
        }
    }
    catch(const ServerMangaer::Error& e)
    {
        std::cout << "stex\n";
         // TODO automate it   404, 405, 411, 412, 413, 414, 431, 500, 501, 505, 503, 507, 508
        std::string fileContent;
        std::ifstream ifs("./error_pages/404.html");
        if (ifs.is_open() == false) {
            throw std::logic_error("can not open file");
        }
        std::getline(ifs, fileContent, '\0');
        size_t pos = fileContent.find("statusCode");
        if (pos != std::string::npos) {
            fileContent.replace(pos, strlen("statusCode"), std::to_string(e.getStatusCode()) + " " + e.what());
        } else {
            fileContent = "Error" + std::to_string(e.getStatusCode());
        };
        response = version + " ";
        headerContent["Content-Length"] = std::to_string(fileContent.size());
        response += std::to_string(e.getStatusCode());
        // std::cout << response << std::endl;
        response += "\r\n";

        for (std::unordered_map<std::string, std::string>::iterator it = headerContent.begin();
            it != headerContent.end(); ++it) {
                response += it->first;
                response += ": ";
                response += it->second;
                response += "\r\n";
        }
        response +=  "\n";
        response +=  fileContent;
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

ServerManager::ServerManager(std::string const &configfile)
{
    (void)configfile;
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

int ServerManager::used(HTTPServer *srv) const
{
    if (!this->empty())
    {
        for(size_t i = 0; i < this->size(); i++)
            if (std::strcmp((*this)[i].getPort(), srv->getPort()) == 0)
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