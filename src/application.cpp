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
#include <sys/stat.h>

#include "application.hpp"
#include "context.hpp"

namespace mvcpp{
    application::application(unsigned short port)
        : _http_server(port),
          _router(_statics)
    {
        using namespace std::placeholders;
        _http_server.set_handler(std::bind(&application::_handle_request, this, _1, _2, _3, _4));
        index_views();
        index_static();
    }
    int application::_handle_request(const std::string path, 
                const std::string method, 
                const std::vector<std::string> headers, 
                std::string& response)
    {
        auto controller_function = _router.route(method, path);
        std::stringstream resp;
        // Create request context
        auto rc = std::make_shared<context>(path, method, headers, _views, resp);
        rc->set_template(rc->get_view(_default_template));
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
        struct dirent *entry;
        DIR *dp;
        std::function<void (std::string)> scan_dir = [&](std::string dir)
        {
            dp = opendir(dir.c_str());
            if(dp == NULL)
            {
                std::cerr << "Cannot open static directory;"
                    " no views will be available for this application" << std::endl;
                return;
            }
            while((entry = readdir(dp)))
            {
                struct stat s;
                std::string file = dir + "/" + std::string(entry->d_name);
                if(stat(file.c_str(), &s) == 0)
                {
                    if(s.st_mode & S_IFREG)
                    {
                        // Regular file, whoop whoop
                        load_static(file);
                        std::cout << "static: " << file << std::endl;
                    }
                    else if (s.st_mode & S_IFDIR)
                    {
                        // Directory
                        std::string subdir;
                        if((subdir = std::string(entry->d_name))[0] != '.') // Don't do hidden files and .. and .
                        {
                            dir = file;
                            std::cout << "Recursing " << dir << std::endl;
                            return scan_dir(dir);
                        }
                    }
                }
            }
            closedir(dp);
        };
        scan_dir("static");
    }
    void application::index_views()
    {
        struct dirent *entry;
        DIR *dp;
        std::function<void (std::string)> scan_dir = [&](std::string dir)
        {
            dp = opendir(dir.c_str());
            if(dp == NULL)
            {
                std::cerr << "Cannot open views directory;"
                    " no views will be available for this application" << std::endl;
                return;
            }
            while((entry = readdir(dp)))
            {
                struct stat s;
                std::string file = dir + "/" + std::string(entry->d_name);
                if(stat(file.c_str(), &s) == 0)
                {
                    if(s.st_mode & S_IFREG)
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
                    else if (s.st_mode & S_IFDIR)
                    {
                        // Directory
                        std::string subdir;
                        if((subdir = std::string(entry->d_name))[0] != '.') // Don't do hidden files and .. and .
                        {
                            dir = file;
                            std::cout << "Recursing " << dir << std::endl;
                            return scan_dir(dir);
                        }
                    }
                }
            }
            closedir(dp);
        };
        scan_dir("views");
    }
}
