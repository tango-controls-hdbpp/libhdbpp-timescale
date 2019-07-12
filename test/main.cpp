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

#define CATCH_CONFIG_RUNNER

#include "LibUtils.hpp"
#include "catch2/catch.hpp"

int main(int argc, char *argv[])
{
    hdbpp::LogConfigurator::initLogging(false, true, "/tmp/hdb/test.log");
    hdbpp::LogConfigurator::setLoggingLevel(spdlog::level::err);

    int result = Catch::Session().run(argc, argv);

    hdbpp::LogConfigurator::shutdownLogging();
    return result;
}
