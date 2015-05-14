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

#include "application.hpp"
#include "controller.hpp"
#include "router.hpp"

class test_controller : public mvcpp::controller<test_controller>
{
public:
    virtual void register_routes(mvcpp::router&) override;

    // Methods
    void index(std::shared_ptr<mvcpp::context>);
};

class test_application : public mvcpp::application
{
public:
    test_application() : application(8080){
        register_controller<test_controller>();
        set_default_template("template");
    }
};
