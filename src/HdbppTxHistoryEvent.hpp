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

#ifndef _HDBPP_TX_HISTORY_EVENT_HPP
#define _HDBPP_TX_HISTORY_EVENT_HPP

#include "HdbppDefines.hpp"
#include "HdbppTxBase.hpp"
#include "LibUtils.hpp"

#include <iostream>
#include <string>

namespace hdbpp_internal
{
// Store history information about an attribute. This is basically just some event
// information at the moment, i.e. added, removed etc
template<typename Conn>
class HdbppTxHistoryEvent : public HdbppTxBase<Conn>
{
public:
    // TODO make crash event configurable

    HdbppTxHistoryEvent(Conn &conn) : HdbppTxBase<Conn>(conn) {}

    auto withName(const std::string &fqdn_attr_name) -> HdbppTxHistoryEvent<Conn> &
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return *this;
    }

    // this overload converts the event types defined in libhdb to
    // usable strings
    auto withEvent(unsigned char event) -> HdbppTxHistoryEvent<Conn> &;

    // allow the adding of any type of event
    auto withEvent(const std::string &event) -> HdbppTxHistoryEvent<Conn> &
    {
        _event = event;
        return *this;
    }

    // trigger the database storage routines
    auto store() -> HdbppTxHistoryEvent<Conn> &;

    /// @brief Print the HdbppTxHistoryEvent object to the stream
    void print(std::ostream &os) const noexcept override;

private:
    AttributeName _attr_name;
    std::string _event;
};

//=============================================================================
//=============================================================================
template<typename Conn>
auto HdbppTxHistoryEvent<Conn>::withEvent(unsigned char event) -> HdbppTxHistoryEvent<Conn> &
{
    // convert the unsigned char history type of a string, we will store the event
    // based on this string, so its simpler to extract the data at a later point
    // without the need to decode a byte into a meaningful value.
    switch (event)
    {
        case libhdbpp_compatibility::HdbppInsert: _event = events::AddEvent; break;
        case libhdbpp_compatibility::HdbppStart: _event = events::StartEvent; break;
        case libhdbpp_compatibility::HdbppStop: _event = events::StopEvent; break;
        case libhdbpp_compatibility::HdbppRemove: _event = events::RemoveEvent; break;
        case libhdbpp_compatibility::HdbppInsertParam: _event = events::InsertParamEvent; break;
        case libhdbpp_compatibility::HdbppPause: _event = events::PauseEvent; break;
        case libhdbpp_compatibility::HdbppUpdateTTL: _event = events::UpdateTTLEvent; break;
        default:
        {
            std::string msg {"Unknown event type passed, unable to convert this into known event system"};
            spdlog::error("Error: {}", msg);
            Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
        }
    }

    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
auto HdbppTxHistoryEvent<Conn>::store() -> HdbppTxHistoryEvent<Conn> &
{
    if (_attr_name.empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    if (_event.empty())
    {
        std::string msg {"The event string is reporting empty. Unable to complete the transaction. For attribute" +
            _attr_name.fqdnAttributeName()};

        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        std::string msg {"The connection is reporting it is closed. Unable to store event. For attribute" +
            _attr_name.fqdnAttributeName()};

        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    // in the case that we receieve a start event, and the last was a start event,
    // then assume the device server hosting te attribute crashed and
    // record a crash event before the next start event
    if (_event == events::StartEvent)
    {
        auto last_event = HdbppTxBase<Conn>::connection().fetchLastHistoryEvent(
            HdbppTxBase<Conn>::attrNameForStorage(_attr_name));

        // check the last event was a StartEvent
        if (last_event == events::StartEvent)
        {
            spdlog::trace("Detected a double: {} event for attribute: {}, storing a {}: before the second {}:",
                events::StartEvent,
                _attr_name.fqdnAttributeName(),
                events::CrashEvent,
                events::StartEvent);

            // insert the crash event
            HdbppTxBase<Conn>::connection()
                .template createTx<HdbppTxHistoryEvent>()
                .withName(_attr_name.fqdnAttributeName())
                .withEvent(events::CrashEvent)
                .store();
        }
    }

    // attempt to store the event in the database, any exeptions are left to
    // propergate to the caller
    HdbppTxBase<Conn>::connection().storeHistoryEvent(HdbppTxBase<Conn>::attrNameForStorage(_attr_name), _event);

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxHistoryEvent<Conn>::print(std::ostream &os) const noexcept
{
    os << "HdbppTxHistoryEvent(base: ";
    HdbppTxBase<Conn>::print(os);

    os << ", "
       << "_attr_name: " << _attr_name << ", "
       << "_event: " << _event << ")";
}

} // namespace hdbpp_internal
#endif // _HDBPP_TX_HISTORY_EVENT_HPP
