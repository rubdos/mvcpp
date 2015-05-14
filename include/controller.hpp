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

#include <memory>

namespace mvcpp{
    class router;
    class controller_base
    {
        friend class router;
        virtual void register_routes(router&)
        {
        };
    };
    template <class T>
    class controller : public controller_base
    {
    public:
        static std::shared_ptr<T> instance()
        {
            if(_instance == nullptr)
            {
                _instance=std::shared_ptr<T>(new T);
            }
            return _instance;
        }
    private:
        static std::shared_ptr<T> _instance;
    };
    template <class T>
    std::shared_ptr<T> controller<T>::_instance;
}
