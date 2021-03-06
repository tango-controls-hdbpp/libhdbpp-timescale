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

#include "PqxxExtension.hpp"

using namespace std;

namespace pqxx
{
namespace internal
{
    template<>
    // NOLINTNEXTLINE
    void builtin_traits<uint8_t>::from_string(const char Str[], uint8_t &Obj)
    {
        // convert the short back to an unsigned char
        int16_t tmp;
        builtin_traits<int16_t>::from_string(Str, tmp);
        Obj = tmp & 0xFF;
    }

    template<>
    // NOLINTNEXTLINE
    string builtin_traits<uint8_t>::to_string(uint8_t Obj)
    {
        // unsigned chars get converted to shorts for storage, there is no
        // support for unsigned char in postgres
        return builtin_traits<int16_t>::to_string(Obj & 0xFF);
    }

    template<>
    // NOLINTNEXTLINE
    void builtin_traits<Tango::DevState>::from_string(const char Str[], Tango::DevState &Obj)
    {
        int32_t tmp;
        builtin_traits<int32_t>::from_string(Str, tmp);
        Obj = static_cast<Tango::DevState>(tmp);
    }

    template<>
    // NOLINTNEXTLINE
    string builtin_traits<Tango::DevState>::to_string(Tango::DevState Obj)
    {
        // DevState is an enum, so its bit of a special case. We simply
        // convert it to an int for storage
        return builtin_traits<int32_t>::to_string(static_cast<int32_t>(Obj));
    }

} // namespace internal
} // namespace pqxx
