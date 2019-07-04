
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

#ifndef _HDBPP_DEFINES_HPP
#define _HDBPP_DEFINES_HPP

#include <string>

namespace hdbpp
{
namespace libhdbpp_compatibility
{
    // to remove the internal dependency on LibHdb++ header we use
    // these event id redefines, this means we do not have to include
    // Libhdbp++.h internally
    const unsigned char HdbppInsert = 0;
    const unsigned char HdbppStart = 1;
    const unsigned char HdbppStop = 2;
    const unsigned char HdbppRemove = 3;
    const unsigned char HdbppInsertParam = 4;
    const unsigned char HdbppPause = 5;
    const unsigned char HdbppUpdateTTL = 6;
} // namespace libhdbpp_compatibility

namespace events
{
    // attribute history events as strings, these are used in the storage system
    const std::string AddEvent = "add"; // represents a attribute added to the system
    const std::string StartEvent = "start"; // represents when the attribute starts sending data
    const std::string StopEvent = "stop"; // represents when the attribute stops sending data
    const std::string RemoveEvent = "remove"; // represents a attribute removed from the system
    const std::string InsertParamEvent = "insert_param";
    const std::string UpdateTTLEvent = "update_ttl";
    const std::string PauseEvent = "pause";
    const std::string CrashEvent = "crash";
} // namespace events
} // namespace hdbpp
#endif // _HDBPP_DEFINES_HPP
