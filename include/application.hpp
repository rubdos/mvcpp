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

#include "http_server.hpp"
#include "router.hpp"

namespace mvcpp{
    class application
    {
    public:
        application(unsigned short port = 80);

        void join();

        void index_views();
        void load_view(std::string path);

    protected:
        template <class T>
        void register_controller()
        {
            _router.register_controller<T>();
        }

    private:
        int _handle_request(const std::string path, 
                const std::string method, 
                const std::vector<std::string> headers, 
                std::string& response);
        http_server _http_server;
        router _router;
        std::map<std::string /*view*/, std::string /*viewpath*/> _views;
    };
}