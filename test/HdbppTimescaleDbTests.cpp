/* Copyright (C) : 2014-2019
   European Synchrotron Radiation Facility
   BP 220, Grenoble 38043, FRANCE

   This file is part of libhdb++timescale.

   libhdb++timescale is free software: you can redistribute it and/or modify
   it under the terms of the Lesser GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libhdb++timescale is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser
   GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with libhdb++timescale.  If not, see <http://www.gnu.org/licenses/>. */

#include "HdbppTimescaleDb.hpp"
#include "catch2/catch.hpp"

#include <netdb.h>

using namespace std;
using namespace hdbpp;

/*SCENARIO("HdbppTimescaleDbImpl initialisation accepts valid configuration", "[hdbpp-timescale-impl][config]")
{
    GIVEN("A HdbppTimescaleDbImpl object and a vector of configuration strings")
    {
        HdbppTimescaleDbImpl impl;
        vector<string> config;

        config.push_back("user=user");
        config.push_back("password=password");
        config.push_back("host=host");
        config.push_back("port=port");
        config.push_back("logging_level=ERROR");

        WHEN("Passing a valid configuration to init")
        {
            THEN("No exception is raised")
            {
                REQUIRE_NOTHROW(impl.init(config));
            }
        }
    }
}
*/