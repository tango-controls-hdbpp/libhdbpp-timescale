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

#ifndef _HDBPP_TX_DATA_EVENT_BASE_HPP
#define _HDBPP_TX_DATA_EVENT_BASE_HPP

#include "AttributeName.hpp"
#include "AttributeTraits.hpp"
#include "HdbppTxBase.hpp"
#include "LibUtils.hpp"

#include <iostream>
#include <string>

namespace hdbpp
{
// Collects the common functionality for data storage events together into a single,
// simple base class. Obviously derived from HdbppTxBase to gain any base class 
// functionality that may be provided for all transaction classes
template<typename Conn, template<typename> class Derived>
class HdbppTxDataEventBase : public HdbppTxBase<Conn>
{
public:
    // TODO when storing INVALID events, i.e. empty or bad quality, do we need to generate an event time, or is the given one still valid?
    // TODO auto add new attribute feature

    HdbppTxDataEventBase(Conn &conn) : HdbppTxBase<Conn>(conn) {}
    virtual ~HdbppTxDataEventBase() {}

    Derived<Conn> &withName(const std::string &fqdn_attr_name)
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return static_cast<Derived<Conn> &>(*this);
    }

    Derived<Conn> &withTraits(Tango::AttrWriteType write, Tango::AttrDataFormat format, Tango::CmdArgType type)
    {
        _traits = AttributeTraits(write, format, type);
        return static_cast<Derived<Conn> &>(*this);
    }

    Derived<Conn> &withTraits(AttributeTraits &traits)
    {
        _traits = traits;
        return static_cast<Derived<Conn> &>(*this);
    }

    Derived<Conn> &withEventTime(Tango::TimeVal tv)
    {
        // convert to something more usable
        _event_time = tv.tv_sec + tv.tv_usec / 1.0e6;
        return static_cast<Derived<Conn> &>(*this);
    }

    Derived<Conn> &withQuality(Tango::AttrQuality quality)
    {
        _quality = quality;
        return static_cast<Derived<Conn> &>(*this);
    }

    /// @brief Print the HdbppTxDataEventBase object to the stream
    virtual void print(std::ostream &os) const noexcept override;

protected:
    // release the private data safely for the derived classes
    AttributeName &attributeName() { return _attr_name; }
    const AttributeTraits &attributeTraits() const { return _traits; }
    Tango::AttrQuality quality() const { return _quality; }
    double eventTime() const { return _event_time; }

private:
    AttributeName _attr_name;
    AttributeTraits _traits;
    Tango::AttrQuality _quality = Tango::ATTR_INVALID;

    // time this parameter change event was generated
    double _event_time = 0;
};

//=============================================================================
//=============================================================================
template<typename Conn, template<typename> class Derived>
void HdbppTxDataEventBase<Conn, Derived>::print(std::ostream &os) const noexcept
{
    // TODO can not print tango objects, the operator<< are not const correct!

    os << "HdbppTxDataEventBase(base: ";
    HdbppTxBase<Conn>::print(os);

    os << ", "
       << "_event_time: " << _event_time << ", "
       << "_attr_name: " << _attr_name << ", "
       << "_traits: " << _traits << ", "
       << "_quality: " << _quality << ", "
       << "_event_time: " << _event_time << ")";
}

} // namespace hdbpp
#endif // _HDBPP_TX_DATA_EVENT_BASE_HPP
