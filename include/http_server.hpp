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
  
#pragma once

#include <map>

#include <thread>
#include <arpa/inet.h>

#include "mvcpp_private.hpp"

namespace mvcpp{
    class http_server
    {
    public:
        http_server(const unsigned short port = 80);

        void join();
        void stop();

        void set_handler(std::function<int (const std::string path, const std::string method, const std::vector<std::string> headers, const std::map<std::string, std::string> data, std::string& response)> handler){this->_request_handler = handler;}

    private:
        const unsigned short _port;
        int _socket;

        void _listen();

        void _handle_request(int, struct sockaddr_in, socklen_t);

        std::thread _listener_thread;
        std::function<int (const std::string path, 
                const std::string method,
                const std::vector<std::string> headers, 
                const std::map<std::string, std::string> data,
                std::string& response)> _request_handler;
    };
}
