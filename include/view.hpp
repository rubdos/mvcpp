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
  
#include <string>
#include <map>
#include <sstream>

namespace mvcpp{
    class view 
    {
        friend class context;
    public:
        view();
        view(const std::string& path);

        void load(const std::string& path);

        std::string& operator[] (const std::string& key);
        view& subview (const std::string& key);

        std::string render() const;
    private:

        std::map<std::string, std::string> _parameters;
        std::map<std::string, view> _subviews;

        std::string _contents;
    };
}
