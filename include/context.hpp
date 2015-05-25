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
#include <memory>
#include <sstream>
#include <vector>

#include "view.hpp"

namespace mvcpp{
    class context
    {
    public:
        typedef std::shared_ptr<context> ptr;
        context(
                const std::string& path, 
                const std::string& param,
                const std::string& method, 
                const std::vector<std::string>& request_headers, 
                const std::map<std::string, std::string>& views,
                const std::map<std::string, std::string>& data,
                std::stringstream& response
                );
        void render(const std::string&);
        void render(const view& v){render(v.render());}

        void compile();

        int get_response_code() const {return _response_code;}
        void set_response_code(int rc) { _response_code = rc;}

        view get_view(const std::string& name);

        void set_template(view v);
        view& get_template();

        void response_header(std::string key, std::string value)
        {
            _response_headers[key] = value;
        }

        std::string get_parameter()
        {
            return _param;
        }

        std::string post_data(std::string key) const
        {
            return _data.at(key);
        }

        std::string get_path() const
        {
            return _path;
        }
    private:
        const std::string _path;
        const std::string _param;
        const std::string _method;
        const std::vector<std::string> _request_headers;
        const std::map<std::string, std::string> _views;
        const std::map<std::string, std::string> _data;

        std::stringstream& _response_stream;

        std::map<std::string, std::string> _response_headers;
        std::string _contents;
        view _template;
        int _response_code;
    };
}
