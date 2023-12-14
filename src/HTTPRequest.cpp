/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maharuty <maharuty@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/12 22:14:54 by dmartiro          #+#    #+#             */
/*   Updated: 2023/12/07 21:47:39 by maharuty         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include "HTTPServer.hpp"


// class HTTPServer;
HTTPRequest::HTTPRequest(void)
{
    reqLineEnd = 0;
    bodyEnd = 0;
    _bodySize = 0;
    statusCode = 0;
    location = NULL;
    _maxSizeRequest = 0;
    _bodySize = 0;
    _isHeaderReady = false;
    _isBodyReady = false;
    _isRequestReady = false;
    _isOpenConnection = false;
    _isResponseReady = false;
    //boundary = "&"; // !IMPORTANT: if GET request: the boundary is (&) else if POST request: boundary is read from (Headers)
}

HTTPRequest::~HTTPRequest()
{
}

std::string const &HTTPRequest::getMethod( void ) const
{
    return (method);
}

std::string HTTPRequest::getBody() const
{
    return (_body);
}

std::string const &HTTPRequest::getPath( void ) const
{
    return (absolutePath);
}

std::string const &HTTPRequest::getVersion( void ) const
{
    return (version);
}

std::string HTTPRequest::getHttpRequest() const {
    return (httpRequest);
}

const std::unordered_map<std::string, std::string> &HTTPRequest::getUploadedFiles() const {
    return(_uploadedFiles);
}

bool HTTPRequest::isRequestReady() const {
    return (_isRequestReady);
}

bool HTTPRequest::isResponseReady() const {
    return (_isResponseReady);
}

std::string HTTPRequest::rtrim(const std::string &str)
{
    size_t end = str.find_last_not_of(" \r\n\t\f\v");
    return (end == std::string::npos ? "" : str.substr(0, end + 1));
}

std::string HTTPRequest::ltrim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \r\n\t\f\v");
    return (start == std::string::npos ? str : str.substr(start));
}

std::string HTTPRequest::trim(const std::string &str)
{
    return (ltrim(rtrim(str)));
}

// void HTTPRequest::processing(sock_t fd)
// {
// }

void HTTPRequest::charChange(std::string &str, char s, char d)
{
    for(size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == s)
            str[i] = d;
    }
}

std::string HTTPRequest::findInMap(std::string key)
{
    std::map<std::string, std::string>::iterator in = httpHeaders.find(key);
    if (in != httpHeaders.end())
        return (in->second);
    std::string nill;
    return (nill);
}

// void HTTPRequest::get(HTTPServer &srv)
// {
//     // std::cout << "GET method" << std::endl;
// }

// void HTTPRequest::post(HTTPServer &srv)
// {
//     if (!(contentType = findInMap("Content-Type")).empty())
//     {
//         type = trim(contentType.substr(0, contentType.find(";")));
//         // HTTPRequest::contentReceiveMethod(fd);
//     }
//     else if (!(transferEncoding = findInMap("Transfer-Encoding")).empty())
//     {
//         std::cout << "Data transfers chunck by chunk" << std::endl;
//     }
// }

// void HTTPRequest::delet(HTTPServer &srv)
// {
//     std::cout << "method is DELETE" << std::endl;
// }


void HTTPRequest::multipart(void)
{
    std::map<std::string, std::string>::iterator it =  httpHeaders.find("Content-Type");
    std::cout << "multipart function " << std::endl;

    if(it == httpHeaders.end())
    {
        throw ResponseError(428, "Precondition Required");
    }
    std::string contentType = it->second;
    contentType.erase(0, contentType.find(";")+1);
    size_t posEqualsign = contentType.find("=");
    if (posEqualsign == std::string::npos) {
        throw ResponseError(428, "Precondition Required");
    }
    boundary = "--" + contentType.substr(posEqualsign + 1);
    boundaryEnd = boundary + "--";

    size_t boundaryPos = _body.find(boundary);
    size_t endPos = _body.find(boundaryEnd);

    do {
        size_t filenameStart = _body.find("filename", boundaryPos);

        if(filenameStart == std::string::npos) {
            throw ResponseError(428, "Precondition Required");
        }
        filenameStart += strlen("filename") + 2;
        std::string filename = _body.substr(filenameStart, _body.find("\"", filenameStart) - filenameStart);
        boundaryPos = _body.find(boundary, filenameStart);
        size_t contentStart = _body.find("\r\n\r\n", filenameStart) + strlen("\r\n\r\n");
        std::string fileContent = _body.substr(contentStart, boundaryPos - contentStart - strlen("\r\n"));
        _uploadedFiles[filename] = fileContent;
        // std::cout << "fileContent = " << fileContent << "$" << std::endl;
    } while (boundaryPos != endPos);

    // std::unordered_map<std::string, std::string>::iterator it1 = _uploadedFiles.begin();
    // while (it1 != _uploadedFiles.end()) {
    //     std::cout << it1->first << "$" <<std::endl;
    //     std::cout << it1->second  << "$" <<std::endl;
    //     ++it1;
    // }
}

void HTTPRequest::showHeaders( void )
{
    std::map<std::string, std::string>::iterator it;
    for(it = httpHeaders.begin(); it != httpHeaders.end(); it++)
    {
        std::cout << it->first << " = " << it->second << std::endl;
    }
}

std::string HTTPRequest::lastChar(std::string const &str, char s)
{
    std::string newString = str;
    if (!newString.empty())
    {
        size_t lst = newString.size() - 1;
        if (newString[lst] != s)
            newString += s;
    }
    return (newString);
}

void HTTPRequest::firstChar(std::string &str, char s)
{
    if (!str.empty())
        if (str[0] != s)
            str = s + str;
}

std::string HTTPRequest::middle_slash(std::string const &s1, char s, std::string const &s2)
{
    std::string newString;
    if (s1[s1.size()-1] == s && s2[0] != s)
        newString = s1 + s2;
    else if (s1[s1.size()-1] != s && s2[0] == s)
        newString = s1 + s2;
    else if (s1[s1.size()-1] != s && s2[0] != s)
        newString = s1 + s + s2;
    else if (s1[s1.size()-1] == s && s2[0] == s)
    {
        std::string ss2 = s2;
        ss2.erase(0, 1);
        newString = s1 + ss2;
    }
    return (newString);
}

int HTTPRequest::in(std::string const &method)
{
    std::vector<std::string>::iterator it = std::find(methods.begin(), methods.end(), method);
    if (it != methods.end())
        return (1);
    return (0);
}

std::string HTTPRequest::dir_content(std::string const &realPath)
{
    DIR* odir = opendir(realPath.c_str());
    std::string directoryContent;
    if (odir)
    {
        struct dirent* each;
        while ((each = readdir(odir)) != NULL)
        {
            std::string d_f_name = "<a href=\"" + std::string(each->d_name) + "\">" + std::string(each->d_name) + "</a><br>";
            directoryContent.insert(0, d_f_name);
        }
        return (directoryContent);
    }
    return (directoryContent);
}

void HTTPRequest::checkPath(HTTPServer const &srv)
{
    size_t use = 0;
    if ((use = path.find_first_of("?")) != std::string::npos)
    {
        queryString = path.substr(use+1); // TODO determine the 
        path = path.substr(0, use);
    }
    location = srv.find(path);
    if (location)
    {
        pathChunks = pathChunking(path);
        absolutePath = middle_slash(location->getRoot(), '/', pathChunks[pathChunks.size() - 1]);
    }
    else
        absolutePath = middle_slash(srv.getRoot(), '/', path);
    //TODO  check indexs
    // TODO extension absolutePath 
}

std::vector<std::string> HTTPRequest::pathChunking(std::string const &rPath)
{
    std::string pathPrefix = rPath;
    std::vector<std::string> chunks;
    std::string pathChunk;
    for(size_t i = 0; i <= pathPrefix.size(); i++)
    {
        if ((pathPrefix[i] == '/' ||  i == pathPrefix.size()))
        {
            if (!pathChunk.empty())
            {
                chunks.push_back(pathChunk);
                pathChunk.clear();
            }
        }
        else
            pathChunk += pathPrefix[i];
    }
    return (chunks);
}

HTTPRequest::PathStatus HTTPRequest::path_status(bool autoindex, std::string const &checkPath)
{
    if (isExist(checkPath))
    {
        if (isDir(checkPath))
        {
            if (autoindex == true)
                return (ISDIR);
            else
                return (DIROFF);
        }
        else if (isFile(checkPath))
            return (ISFILE);
    }
    return (NOTFOUND);
}

size_t HTTPRequest::slashes(std::string const &pathtosplit)
{
    size_t count = 0;
    for(size_t i = 0; i < pathtosplit.size(); i++)
        if (pathtosplit[i] == '/')
            count++;
    return (count);
}


// HTTPRequest::PathStatus HTTPRequest::path_status(HTTPServer const &srv, std::string const &checkPath)
// {
//     if (path_status(checkPath) == PathStatus::ISDIR)
//         if (srv.getAutoindex() == true)
//             return (PathStatus::DIRON);
//     return (PathStatus::DIROFF);
// }

// HTTPRequest::PathStatus HTTPRequest::path_status(const Location* location, std::string const &checkPath)
// {
//     if (path_status(checkPath) == PathStatus::ISDIR)
//         if (location->getAutoindex() == true)
//             return (PathStatus::DIRON);
//     return (PathStatus::DIROFF);
// }

// HTTPRequest::PathStatus HTTPRequest::path_status(std::string const &checkPath)
// {
//     if (isExist(checkPath))
//     {
//         if (isDir(checkPath))
//             return (PathStatus::ISDIR);
//         else if (isFile(checkPath))
//             return (PathStatus::ISFILE);
//     }
//     return (PathStatus::NOTFOUND);
// }


bool HTTPRequest::isDir(const std::string& filePath) {
    struct stat file;
    if (stat(filePath.c_str(), &file) != 0)
        return false;
    return S_ISDIR(file.st_mode);
}

bool HTTPRequest::isFile(const std::string& filePath) {
    struct stat file;
    if (stat(filePath.c_str(), &file) != 0)
        return false;
    return S_ISREG(file.st_mode);
}

bool HTTPRequest::isExist(std::string const &filePath)
{
    struct stat existing;
    return (stat(filePath.c_str(), &existing) == 0);
}
