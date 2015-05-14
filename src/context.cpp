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

#include "context.hpp"

namespace mvcpp{
    context::context(
        const std::string& path, 
        const std::string& method, 
        const std::vector<std::string>& request_headers, 
        const std::map<std::string, std::string>& views,
        std::stringstream& response)
        : _path(path), _method(method), _request_headers(request_headers), _response_code(200),
          _response_stream(response), _views(views)
    {
        _response_headers["Content-Type"] = "text/html"; // Default to text/html
    }

    void context::render(const std::string& s)
    {
        _contents = s;
    }

    void context::compile()
    {
        _response_stream.str(""); 
        std::ostringstream ss;
        ss << _contents.size();
        _response_headers["Content-Length"] = ss.str();
        for(auto iter : _response_headers)
        {
            _response_stream << iter.first << ": " << iter.second << "\r\n";
        }
        _response_stream << "\r\n"
            << _contents;
    }

    view context::get_view(const std::string& name)
    {
        try
        {
            return view(_views.at(name));
        }
        catch(std::out_of_range& e)
        {
            return view("");
        }
    }
}
