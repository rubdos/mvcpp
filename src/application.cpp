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
  
#include <iostream>
#include <sstream>
#include <dirent.h>

#include "application.hpp"
#include "context.hpp"

namespace mvcpp{
    application::application(unsigned short port)
        : _http_server(port),
          _router(_statics)
    {
        using namespace std::placeholders;
        _http_server.set_handler(std::bind(&application::_handle_request, this, _1, _2, _3, _4, _5));
        index_views();
        index_static();
    }
    int application::_handle_request(const std::string path, 
                const std::string method, 
                const std::vector<std::string> headers, 
                const std::map<std::string, std::string> data, 
                std::string& response)
    {
        auto controller_function = _router.route(method, path);
        std::stringstream resp;
        // Create request context
        auto rc = std::make_shared<context>(path, method, headers, _views, data, resp);

        rc->set_template(rc->get_view(_default_template));
        initialize_default_template(rc);

        controller_function(rc);
        rc->compile();
        response = resp.str(); //"<h1>Hello world</h1> You requested path " + path;
        return rc->get_response_code();
    }
    void application::join()
    {
        _http_server.join();
    }
    void application::load_view(std::string filepath)
    {
        _views[filepath.substr(6, filepath.length()-6-10)] = filepath; // Strip file extension
        std::cout << filepath.substr(6, filepath.length()-6-10) << " -> " << filepath << std::endl;
    }
    void application::load_static(std::string filepath)
    {
        _statics.push_back(filepath);
    }
    void application::index_static()
    {
        std::function<void (std::string)> scan_dir = [this, &scan_dir](std::string dir)
        {
            struct dirent *entry;
            DIR *dp;
            dp = opendir(dir.c_str());
            if(dp == NULL)
            {
                std::cerr << "Cannot open static directory;"
                    " no views will be available for this application" << std::endl;
                return;
            }
            while((entry = readdir(dp)))
            {
                std::string file = dir + "/" + std::string(entry->d_name);

                if(!(entry->d_type & DT_DIR))
                {
                    // Regular file, whoop whoop
                    load_static(file);
                    std::cout << "static: " << file << std::endl;
                }
                else
                {
                    // Directory
                    if(entry->d_name[0] != '.') // Don't do hidden files and .. and .
                    {
                        std::cout << "Recursing " << file << std::endl;
                        scan_dir(file);
                    }
                }
            }
            closedir(dp);
        };
        scan_dir("static");
    }
    void application::index_views()
    {
        std::function<void (std::string)> scan_dir = [&](std::string dir)
        {
            struct dirent *entry;
            DIR *dp;
            dp = opendir(dir.c_str());
            if(dp == NULL)
            {
                std::cerr << "Cannot open views directory;"
                    " no views will be available for this application" << std::endl;
                return;
            }
            while((entry = readdir(dp)))
            {
                std::string file = dir + "/" + std::string(entry->d_name);

                if(!(entry->d_type & DT_DIR))
                {
                    // Regular file, whoop whoop
                    // Check whether the file ends in .html.view
                    if(file.substr(file.length() - 10, 10) == ".html.view")
                    {
                        load_view(file);
                    }
                    else
                        std::cout << "Strange file: " << file << std::endl;
                }
                else
                {
                    // Directory
                    if(std::string(entry->d_name)[0] != '.') // Don't do hidden files and .. and .
                    {
                        std::cout << "Recursing " << file << std::endl;
                        scan_dir(file);
                    }
                }
            }
            closedir(dp);
        };
        scan_dir("views");
    }
}
