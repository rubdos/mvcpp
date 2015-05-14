  /* This file is part of MVC++.

  *  MVC++ is free software: you can redistribute it and/or modify
  *  it under the terms of the GNU Lesser General Public License as 
  *  published by the Free Software Foundation, either version 3 of
  *  the License, or (at your option) any later version.

  *  MVC++ is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.

  *  You should have received a copy of the GNU Lesser General Public 
  *  License along with MVC++. 
  *  If not, see <http://www.gnu.org/licenses/>.
  */

#include <functional>

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <sstream>

#include "http_server.hpp"
  
namespace mvcpp{
    http_server::http_server(unsigned short port)
        : _port(port), 
          _listener_thread(&http_server::_listen, this)
    {
    }
    void http_server::_handle_request(int socket,
            struct sockaddr_in addr, 
            socklen_t addrlen)
    {
        int error;
        char buff[16*1024]; // Accept a maximum of 16K of header data
        std::string headers;

        while(headers.size() < 4 
                or 
                headers.substr(headers.size() - 4, 4) != "\r\n\r\n")
            // TODO: this is a VERY terrible way of doing this
        {
            error = recv(socket, buff, 16*1024, 0);
            if(error < 0)
            {
                close(socket);
                return;
            }
            headers += std::string(buff, error);
        }

        std::vector<std::string> header_lines;
        std::istringstream iss(headers);

        for(std::string token; std::getline(iss, token, '\n');)
        {
            header_lines.push_back(token.substr(0, token.size() - 1));
            // Chop of the \r without checking TODO
        }

        std::string requestline = header_lines[0];
        header_lines.erase(header_lines.begin());

        std::cout << "Request: " << std::endl << headers;
        std::cout << requestline << std::endl;

        iss.str(requestline);
        iss.clear();

        std::vector<std::string> requestline_tokens;

        for(std::string token; std::getline(iss, token, ' ');)
        {
            requestline_tokens.push_back(token);
        }
        std::string method = requestline_tokens[0];
        std::string path = requestline_tokens[1];

        auto error_response = [](std::string version, std::string text){
            std::stringstream html;
            html << "<h1>" << text << "</h1>";
            std::stringstream ss;
            ss << version << " " << text << "\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << html.str().size() << "\r\n"
                << "\r\n"
                << html.str();
            return ss.str();
        };

        if(requestline_tokens.size() != 3)
        {
            std::string response("HTTP/0.9 505 HTTP version not supported\r\n\r\n");
            send(socket, response.c_str(), response.size(), 0);
            close(socket);
            return;
        }

        std::string version = requestline_tokens[2];
        if(version == "HTTP/0.9")
        {
            std::string response("HTTP/0.9 505 HTTP version not supported\r\n\r\n");
            send(socket, response.c_str(), response.size(), 0);
            close(socket);
            return;
        }

        std::string response;
        if(!_request_handler)
        {
            std::string response = error_response("HTTP/1.1", "500 Server currently unavailable");
            send(socket, response.c_str(), response.size(), 0);
            close(socket);
            return;
        }
        int status = _request_handler(path, method, header_lines, response);

        std::string explanation = "OK"; // TODO
        std::stringstream ss;
        ss << version << " " << status << " " << explanation << "\r\n";
        ss << response;

        send(socket, ss.str().c_str(), ss.str().size(), 0);
        close(socket);
    }
    void http_server::_listen()
    {
        struct sockaddr_in sourceAddr;
        struct sockaddr_in destAddr;

        int error;

        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(_port);
        destAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

        _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        error = bind(_socket, (sockaddr*)&destAddr, sizeof(destAddr));
        if(error < 0)
        {
            std::cerr << "Cannot bind to port " << _port << std::endl;
            return;
        }
        error = listen(_socket, 10);
        if(error < 0)
        {
            std::cerr << "Cannot listen on port " << _port << std::endl;
            return;
        }
        while(true)
        {
            struct sockaddr_in sourceAddr;
            socklen_t addrLen = sizeof(sourceAddr);
            int client_socket = accept(_socket, 
                    (sockaddr*) &sourceAddr, &addrLen);
            std::thread handler(&http_server::_handle_request, this, client_socket, sourceAddr, addrLen);
            handler.detach();
        }
    }

    void http_server::join()
    {
        _listener_thread.join();
    }
}
