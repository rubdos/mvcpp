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
  
#include <sys/stat.h>
#include <algorithm>
#include <iostream>

#include "router.hpp"
#include "context.hpp"

namespace mvcpp{
    controller_method router::route(std::string method, std::string path)
    {
        try
        {
            return _routes[method].at(path);
        }
        catch(std::out_of_range& e)
        {
            // Perhaps a static?
            if(std::find(_statics.begin(), _statics.end(), "static" + path) != _statics.end())
            {
                return [&](std::shared_ptr<context> ctx){
                    ctx->render(view("static" + path));
                    ctx->response_header("Content-Type", "text/plain");
                    if(path.substr(path.length() - 3, 3) == "css")
                    {
                        ctx->response_header("Content-Type", "text/css");
                    }
                    else if(path.substr(path.length() - 2, 2) == "js")
                    {
                        ctx->response_header("Content-Type", "text/javascript");
                    }
                };
            }
            return [&](std::shared_ptr<context> ctx){
                ctx->set_response_code(404);
                ctx->render("<h1>404: Page not found</h1><p>The path you requested, <tt>" 
                        + path 
                        + "</tt>, could not be found</p>");
            };
        }
    }
    controller_method router::operator() (const std::string& path)
    {
        return this->route("GET", path);
    }
}
