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
#include <functional>
#include <memory>
#include <vector>

namespace mvcpp{
    class context;
    typedef std::function<void (std::shared_ptr<context>)> controller_method;
    class router
    {
    public:
        router(std::vector<std::string>& statics) : _statics(statics) {}
        controller_method operator() (const std::string& path);
        void operator() (const std::string& path, controller_method m)
        {
            _routes["GET"][path] = m;
        }
        template <class T>
        void operator() (const std::string& path, void (T::* f) (std::shared_ptr<context>) )
        {
            using namespace std::placeholders;
            (*this)(path, std::bind(f, T::instance(), _1));
        }

        void post(const std::string& path, controller_method m)
        {
            _routes["POST"][path] = m;
        }
        template <class T>
        void post(const std::string& path, void(T::* f) (std::shared_ptr<context>))
        {
            using namespace std::placeholders;
            this->post(path, std::bind(f, T::instance(), _1));
        }

        controller_method route(std::string method, std::string path);

        template <class T>
        void register_controller()
        {
            auto controller = T::instance();
            controller->register_routes(*this);
        }
    private:
        std::map<std::string /* method */,
            std::map<std::string /* pattern */, 
            controller_method>> _routes;
        std::vector<std::string> & _statics;
    };
}
