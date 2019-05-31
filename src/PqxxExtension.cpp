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
        int16_t tmp;
        builtin_traits<int16_t>::from_string(Str, tmp);
        Obj = tmp & 0xFF;
    }

    template<>
    // NOLINTNEXTLINE
    string builtin_traits<uint8_t>::to_string(uint8_t Obj)
    {
        return builtin_traits<int16_t>::to_string(Obj & 0xFF);
    }

} // namespace internal
} // namespace pqxx
