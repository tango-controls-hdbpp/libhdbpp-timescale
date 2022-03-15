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

#ifndef _TANGO_VALUE_HPP
#define _TANGO_VALUE_HPP

#include <vector>

namespace hdbpp_internal
{
// Utility class to keep data and dimension in the same place.
// This is then used for pqxx extension to properly extract 
// arrays and images.
template<class T>
class TangoValue : public std::vector<T>
{
    public:
    std::size_t dim_x;
    std::size_t dim_y;
};
}
#endif
