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

#ifndef _HDBPP_TX_PARAMETER_EVENT_HPP
#define _HDBPP_TX_PARAMETER_EVENT_HPP

#include "AttributeTraits.hpp"
#include "HdbppTxBase.hpp"

#include <iostream>
#include <string>

namespace hdbpp
{
template<typename Conn>
class HdbppTxParameterEvent : public HdbppTxBase<Conn>
{
public:
    // TODO print()

    HdbppTxParameterEvent(Conn &conn) : HdbppTxBase<Conn>(conn) {}
    virtual ~HdbppTxParameterEvent() {}

    HdbppTxParameterEvent<Conn> &withName(const std::string &fqdn_attr_name)
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return *this;
    }

    HdbppTxParameterEvent<Conn> &withAttrInfo(const Tango::AttributeInfoEx &attr_conf)
    {
        _attr_conf = attr_conf;
        _attr_conf_set = true;
        return *this;
    }

    HdbppTxParameterEvent<Conn> &withEventTime(Tango::TimeVal tv)
    {
        _event_time = tv.tv_sec + tv.tv_usec / 1.0e6;
        return *this;
    }

    HdbppTxParameterEvent<Conn> &store();

    /// @brief Print the HdbppTxParameterEvent object to the stream
    void print(std::ostream &os) const override;

private:
    AttributeName _attr_name;

    // time this parameter change event was generated
    double _event_time = 0;

    // a copy to the AttributeInfo passed when the event was raised, taken
    // as a copy so we can in future pipeline these events and not worry about
    // holding pointers to tango structures.
    Tango::AttributeInfoEx _attr_conf;

    bool _attr_conf_set = false;
};

//=============================================================================
//=============================================================================
template<typename Conn>
HdbppTxParameterEvent<Conn> &HdbppTxParameterEvent<Conn>::store()
{
    if (_attr_name.empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (!_attr_conf_set)
    {
        std::string msg {"AttributeInfo is not set. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (_event_time == 0)
    {
        std::string msg {"Event time is not set. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        string msg {"The connection is reporting it is closed. Unable to store parameter event."};
        throw std::invalid_argument(msg);
    }

    // now store the parameter event
    HdbppTxBase<Conn>::connection().storeParameterEvent(HdbppTxBase<Conn>::attrNameForStorage(_attr_name),
        _event_time,
        _attr_conf.label,
        _attr_conf.unit,
        _attr_conf.standard_unit,
        _attr_conf.display_unit,
        _attr_conf.format,
        _attr_conf.events.arch_event.archive_rel_change,
        _attr_conf.events.arch_event.archive_abs_change,
        _attr_conf.events.arch_event.archive_period,
        _attr_conf.description);

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxParameterEvent<Conn>::print(std::ostream &os) const
{
    HdbppTxBase<Conn>::print(os);

    //os << "_attr_name: " << _attr_name;
}

} // namespace hdbpp
#endif // _HDBPP_TX_PARAMETER_EVENT_HPP
