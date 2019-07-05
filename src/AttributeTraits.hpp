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

#ifndef _ATTRIBUTE_TRAITS_H
#define _ATTRIBUTE_TRAITS_H

#include "LibUtils.hpp"

#include <iostream>
#include <tango.h>

// why is it OmniORB (via Tango)) and Pqxx define these types in different ways? Perhaps
// its the autotools used to configure them? Either way, we do not use tango, just need its
// types, so undef and allow the Pqxx defines to take precedent
#undef HAVE_UNISTD_H
#undef HAVE_SYS_TYPES_H
#undef HAVE_SYS_TIME_H
#undef HAVE_POLL

namespace hdbpp
{
/// @class AttributeTraits
/// @brief Represents The combined traits for a Tango Attribute.
/// @details
/// The combined traits for a Tango Attribute and gives a series of convientent
/// accessors to work with the traits
class AttributeTraits
{
public:
    AttributeTraits() = default;
    AttributeTraits(const AttributeTraits &) = default;
    AttributeTraits(AttributeTraits &&) = default;
    ~AttributeTraits() = default;

    AttributeTraits(Tango::AttrWriteType write_type, Tango::AttrDataFormat format, Tango::CmdArgType data_type) :
        _attr_write_type(write_type),
        _attr_format(format),
        _attr_type(data_type)
    {}

    bool isArray() const noexcept { return _attr_format == Tango::SPECTRUM; }
    bool isScalar() const noexcept { return _attr_format == Tango::SCALAR; }
    bool isImage() const noexcept { return _attr_format == Tango::IMAGE; }

    bool isReadOnly() const noexcept { return _attr_write_type == Tango::READ; }
    bool isWriteOnly() const noexcept { return _attr_write_type == Tango::WRITE; }
    bool isReadWrite() const noexcept { return _attr_write_type == Tango::READ_WRITE; }
    bool isReadWithWrite() const noexcept { return _attr_write_type == Tango::READ_WITH_WRITE; }

    bool hasReadData() const noexcept { return isReadOnly() || isReadWrite() || isReadWithWrite(); }
    bool hasWriteData() const noexcept { return isWriteOnly() || isReadWrite() || isReadWithWrite(); }

    Tango::CmdArgType type() const noexcept { return _attr_type; }
    Tango::AttrWriteType writeType() const noexcept { return _attr_write_type; }
    Tango::AttrDataFormat formatType() const noexcept { return _attr_format; }

    AttributeTraits &operator=(const AttributeTraits &) = default;
    AttributeTraits &operator=(AttributeTraits &&) = default;

    bool operator==(const AttributeTraits &other) const
    {
        return _attr_write_type == other.writeType() && _attr_format == other.formatType() &&
            _attr_type == other.type();
    }

    /// @brief Print the AttributeTraits object to the stream
    void print(std::ostream &os) const;

private:
    Tango::AttrWriteType _attr_write_type = Tango::WT_UNKNOWN;
    Tango::AttrDataFormat _attr_format = Tango::FMT_UNKNOWN;
    Tango::CmdArgType _attr_type = Tango::DATA_TYPE_UNKNOWN;
};

} // namespace hdbpp
#endif // _ATTRIBUTE_TRAITS_H
