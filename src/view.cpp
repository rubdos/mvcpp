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
  
#include <fstream>

#include "view.hpp"

namespace mvcpp{
    view::view(const std::string& path)
    {
        std::ifstream f (path);
        if(! f)
        {
            // Returning empty view. Sad panda
            return;
        }
        std::stringstream ss;
        ss << f.rdbuf();
        _contents = ss.str();
        f.close();
    }
    std::string view::render() const
    {
        std::string result = _contents;
        for(auto it: _parameters)
        {
            size_t pos = 0;
            std::string search = "#" + it.first + "#";
            while ((pos = result.find(search, pos)) != std::string::npos)
            {
                result.replace(pos, search.length(), it.second);
                pos += it.second.length();
            }
        }
        for(auto it: _subviews)
        {
            size_t pos = 0;
            std::string search = "#" + it.first + "#";
            while ((pos = result.find(search, pos)) != std::string::npos)
            {
                auto replace = it.second.render();
                result.replace(pos, search.length(), replace);
                pos += replace.length();
            }
        }
        return result;
    }
    
    std::string& view::operator[] (const std::string& key)
    {
        return _parameters[key];
    }
}
