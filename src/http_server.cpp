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
#include <errno.h>

#include "http_server.hpp"
  
namespace mvcpp{
    const char HEX2DEC[256] = 
    {
        /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
        /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

        /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

        /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

        /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
    };
    // Source: http://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12759/URI-Encoding-and-Decoding.htm
    std::string UriDecode(const std::string & sSrc)
    {
       // Note from RFC1630: "Sequences which start with a percent
       // sign but are not followed by two hexadecimal characters
       // (0-9, A-F) are reserved for future extension"
     
       const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
       const int SRC_LEN = sSrc.length();
       const unsigned char * const SRC_END = pSrc + SRC_LEN;
       // last decodable '%' 
       const unsigned char * const SRC_LAST_DEC = SRC_END - 2;
     
       char * const pStart = new char[SRC_LEN];
       char * pEnd = pStart;
     
       while (pSrc < SRC_LAST_DEC)
       {
          if (*pSrc == '%')
          {
             char dec1, dec2;
             if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
             {
                *pEnd++ = (dec1 << 4) + dec2;
                pSrc += 3;
                continue;
             }
          }
          else if(*pSrc == '+')
          {
              *pEnd++=' ';
              pSrc++;
          }
     
          *pEnd++ = *pSrc++;
       }
     
       // the last 2- chars
       while (pSrc < SRC_END)
          *pEnd++ = *pSrc++;
     
       std::string sResult(pStart, pEnd);
       delete [] pStart;
       return sResult;
    }

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
        std::string headers, data;

        time_t request_time = time(NULL);

        float transferrate = 100;

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

            transferrate = (float) (headers.size())/(float)(time(NULL) - request_time);
            size_t hep;
            if((hep = headers.find("\r\n\r\n")) != headers.npos)
            {
                // Some data came through after the endlines
                data = headers.substr(hep + 4);
                headers = headers.substr(0, hep);
                break;
            }
            if((time(NULL) - request_time) > 100) // TODO: check for a file post.
            {
                std::string response("HTTP/0.9 408 request timed out. Try again.\r\n\r\n");
                send(socket, response.c_str(), response.size(), 0);
                close(socket);
                return;
            }
            if(error == 0)
            {
                // Sleep for the time needed to get 100 bytes
                std::this_thread::sleep_for(std::chrono::milliseconds((int64_t)(100 * transferrate)));
                //std::cout << "Sleeping for this slower client" << std::endl;
            }
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

        //std::cout << "Request: " << std::endl << headers;
        //std::cout << requestline << std::endl;

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

        //Parse data
        std::map<std::string, std::string> parsed_data;
        iss.str(data);
        iss.clear();

        for(std::string token; std::getline(iss, token, '&');)
        {
            std::string key;
            std::string value;
            size_t posofequals = token.find("=");
            key = token.substr(0, posofequals);
            if(posofequals != std::string::npos)
            {
                value=token.substr(posofequals + 1);
            }

            parsed_data[key]=UriDecode(value);
            std::cout << "k:" << key<< ", v=" << value<<std::endl;
        }


        int status = _request_handler(path, method, header_lines, parsed_data, response);

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
            if(client_socket < 0)
            {
                if(errno == ECONNABORTED 
                        or errno == EBADF 
                        or errno == EINVAL)
                {
                    close(_socket);
                    break;
                }
                else
                {
                    std::cout << "Error " << errno << std::endl;
                }
            }
            std::thread handler(&http_server::_handle_request, this, client_socket, sourceAddr, addrLen);
            handler.detach();
        }
    }

    void http_server::stop()
    {
        shutdown(_socket, 2);
    }

    void http_server::join()
    {
        _listener_thread.join();
    }
}
