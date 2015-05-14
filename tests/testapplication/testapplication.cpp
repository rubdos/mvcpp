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
  
#include <time.h>

#include "testapplication.hpp"
#include "context.hpp"

void test_controller::register_routes(mvcpp::router& r)
{
    using namespace std::placeholders;
    r("/", &test_controller::index);
}

void test_controller::index(mvcpp::context::ptr ctx)
{
    mvcpp::view& templ = ctx->get_template();
    auto t = ctx->get_view("test");
    auto tm = time(NULL);
    t["TIJD"] = std::string(asctime(localtime(&tm)));
    templ.subview("SUBVIEW") = t;
}

int main()
{
    test_application app;
    app.join();
}
