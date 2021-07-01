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

#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <string>

namespace hdbpp_internal
{
// The ConnectionBase class defines some basics all storage classes
// must implement. The derived class must implement a further API based
// on what the transaction classes use.
class ConnectionBase
{
public:
    // TODO config params to constructor and get api

    // connection API
    virtual void connect(const std::string &connect_string) = 0;
    virtual void disconnect() = 0;
    virtual auto isOpen() const noexcept -> bool = 0;
    virtual auto isClosed() const noexcept -> bool = 0;
};

}; // namespace hdbpp_internal
#endif // _ABSTRACT_CONNECTION_HPP
