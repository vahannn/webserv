#include "Cgi.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <ctime>

std::map<std::string, std::string> Cgi::_env;

int Cgi::execute(Client &client) {
    signal(SIGPIPE, SIG_IGN); // TODO
    char *argv[3];
    const std::string &argv1 = client.getCurrentLoc().getCgi(client.getExtension()).second;
    argv[0] = const_cast<char *>(argv1.c_str());
    const std::string &argv2 =  client.getPath();
    argv[1] = const_cast<char *>(argv2.c_str());
    argv[2] = NULL;
    int pipe_from_child[2];
    int pipe_to_child[2];

    if (pipe(pipe_from_child) == -1 || pipe(pipe_to_child) == -1) {
        throw ResponseError(500, "Internal Server Error");
    }
    fcntl(pipe_to_child[1], F_SETFL, O_NONBLOCK, O_CLOEXEC);
	fcntl(pipe_from_child[0], F_SETFL, O_NONBLOCK, O_CLOEXEC);
    int pid = fork();

    if (pid == -1) {
        throw ResponseError(500, "Internal Server Error");
    }
    std::ofstream osf("log.log");
    osf << client.getRequestBody();
    // std::cout << "client.getRequestBody().c_str()" << std::endl;
    if (pid == 0) {
        char **envp = Cgi::initEnv(client);
        dup2(pipe_from_child[1], 1);
        close(pipe_from_child[0]);
        close(pipe_from_child[1]);

        if (client.getMethod() == "POST") {
            // write(pipe_to_child[1], client.getRequestBody().c_str(), client.getRequestBody().size());
            dup2(pipe_to_child[0], 0);
            close(pipe_to_child[1]);
            close(pipe_to_child[0]);
        }
        int res = execve(argv[0], argv, envp);
        perror("execve: ");
        exit(res);
    }
    EvManager::addEvent(pipe_to_child[1], EvManager::write);
    client.addInnerFd(new InnerFd(pipe_to_child[1], client, client.getRequestBody(), EvManager::write));
    close(pipe_from_child[1]);
    // close(pipe_to_child[1]);
    close(pipe_to_child[0]);
    client.setCgiPID(pid);
    client.setCgiStartTime();
    return (pipe_from_child[0]);
};

char **Cgi::initEnv(Client const &client)
{
    char *pwd;
    const ServerCore &srv = client.getCurrentLoc();
    char *clientIp = client.inet_ntoa(client.getSocketAddress()->sin_addr);

    client.showHeaders();
    pwd = getcwd(NULL, 0);
    _env["AUTH_TYPE"] = "Basic";
    _env["CONTENT_LENGTH"] = my_to_string(client.getRequestBody().size());
    _env["CONTENT_TYPE"] = client.findInMap("Content-Type");
    // _env["TRANSFER-ENCODING"] = client.findInMap("Transfer-Encoding");
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["PATH_INFO"] = client.getPath();
    _env["PATH_TRANSLATED"] = pwd + std::string("/") + client.getPath();
    _env["QUERY_STRING"] = client.getQueryString();
    _env["REMOTE_ADDR"] = clientIp;
    _env["REMOTE_HOST"] = client.findInMap("Host");
    _env["REMOTE_USER"] = client.getUser(pwd);
    _env["REQUEST_METHOD"] = client.getMethod();
    _env["SCRIPT_NAME"] = client.getPath();
    _env["SCRIPT_FILENAME"] = pwd + std::string("/") + client.getPath();
    _env["SERVER_NAME"] = "webserv";
    _env["SERVER_PORT"] = client.getServerPort();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "Webserv";
    _env["SERVER_WRITE_PATH"] = srv.getUploadDir();
    _env["UPLOAD_DIR"] = srv.getUploadDir();
    _env["LC_CTYPE"] = "C.UTF-8";
    _env["REDIRECT_STATUS"] = "true";
	free(pwd);

    //  pwd = getcwd(NULL, 0);
    // _env["AUTH_TYPE"] = "";
    // _env["CONTENT_LENGTH"] =  my_to_string(client.getRequestBody().size());
    // _env["CONTENT_TYPE"] = client.findInMap("Content-Type");
    // _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    // _env["PATH_INFO"] = client.getPath();
    // _env["PATH_TRANSLATED"] = pwd + std::string("/") + client.getPath();
    // _env["QUERY_STRING"] = client.getQueryString();
    // _env["REMOTE_ADDR"] = "";
    // _env["REMOTE_HOST"] = "::1";
    // _env["REMOTE_IDENT"] = "";
    // _env["REMOTE_USER"] = "";
    // _env["REQUEST_METHOD"] = client.getMethod();
    // _env["SCRIPT_FILENAME"] = pwd + std::string("/") + client.getPath();
    // _env["SCRIPT_NAME"] = pwd + std::string("/") + client.getPath();
    // _env["SERVER_NAME"] = "webserv";
    // _env["SERVER_PORT"] = client.getServerPort();
    // _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    // _env["SERVER_SOFTWARE"] = "webserv/1.0";
    // _env["SERVER_WRITE_PATH"] = pwd + std::string("/") +  srv.getUploadDir();
    // _env["UPLOAD_DIR"] = pwd + std::string("/")  + srv.getUploadDir();
    // _env["LC_CTYPE"] = "C.UTF-8";
    // _env["REDIRECT_STATUS"] = "true";
    


    char **envp = new char *[_env.size() + 1];

	int i = 0;
    std::ofstream ofs("env.log");
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
	{
        // std::cout << it->first << " = " << it->second << std::endl;
		// envp[i++] = strdup((it->first + "=\"" + it->second + "\"").c_str());
		envp[i++] = strdup((it->first + "=" + it->second).c_str());
        ofs << envp[i - 1] << std::endl;;
	}

	envp[i] = NULL;

	return envp;
};