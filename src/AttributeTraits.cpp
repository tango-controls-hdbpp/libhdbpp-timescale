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

#include "AttributeTraits.hpp"

using namespace std;

namespace hdbpp_internal
{
//=============================================================================
//=============================================================================
auto AttributeTraits::isValid() const noexcept -> bool
{
    // ensure all the type information is valid
    return _attr_write_type != Tango::WT_UNKNOWN && _attr_format != Tango::FMT_UNKNOWN &&
        _attr_type != Tango::DATA_TYPE_UNKNOWN;
}

//=============================================================================
//=============================================================================
void AttributeTraits::print(ostream &os) const noexcept
{
    os << "AttributeTraits("
       << "write_type: " << _attr_write_type << "(" << static_cast<unsigned int>(_attr_write_type) << "), "
       << "format_type: " << _attr_format << "(" << static_cast<unsigned int>(_attr_format) << "), "
       << "type: " << _attr_type << "(" << static_cast<unsigned int>(_attr_type) << ")";
}

} // namespace hdbpp_internal
