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
    controller_method router::route(const std::string& method, const std::string& path, std::string& parameter)
    {
        std::cout << method << " request received on " << path << std::endl;
        try
        {
            return _routes[method].at(path);
        }
        catch(std::out_of_range& e)
        {
            // Perhaps a static?
            std::string staticpath = "static" + path;
            if(std::find(_statics.begin(), _statics.end(), staticpath) != _statics.end())
            {
                return [staticpath, path](std::shared_ptr<context> ctx){
                    ctx->render(view(staticpath));
                    ctx->response_header("Content-Type", "text/plain");
                    std::map<std::string, std::string> mimetypes;
                    mimetypes["css"] = "text/css";
                    mimetypes["js"] = "application/javascript";
                    mimetypes["jpg"] = "image/jpeg";
                    mimetypes["jpeg"] = "image/jpeg";
                    mimetypes["png"] = "image/png";
                    mimetypes["gif"] = "image/gif";
                    for(auto it: mimetypes)
                    {
                        if(path.substr(path.length() - it.first.size()) == it.first)
                        {
                            ctx->response_header("Content-Type", it.second);
                            break;
                        }
                    }
                };
            }
            // Perhaps a more interesting route?
            for(auto it: _routes[method])
            {
                size_t i = it.first.find(":");
                if(i == std::string::npos)
                    continue;
                if(it.first.substr(0, i) == path.substr(0, i))
                {
                    parameter = path.substr(i);
                    std::cout << "Path: " << it.first.substr(0, i) << ", param: " << parameter << std::endl;
                    return it.second;
                }
            }
            
            // Nothing
            std::cout << "404 on " << path << std::endl;
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
        std::string param;
        return this->route("GET", path, param);
    }
}
