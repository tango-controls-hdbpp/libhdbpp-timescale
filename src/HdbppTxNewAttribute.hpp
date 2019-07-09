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

#ifndef _HDBPP_TX_NEW_ATTRIBUTE_HPP
#define _HDBPP_TX_NEW_ATTRIBUTE_HPP

#include "AttributeTraits.hpp"
#include "HdbppTxHistoryEvent.hpp"
#include "LibUtils.hpp"

#include <iostream>
#include <string>

namespace hdbpp
{
template<typename Conn>
class HdbppTxNewAttribute : public HdbppTxBase<Conn>
{
public:
    HdbppTxNewAttribute(Conn &conn) : HdbppTxBase<Conn>(conn) {}
    virtual ~HdbppTxNewAttribute() {}

    HdbppTxNewAttribute<Conn> &withName(const std::string &fqdn_attr_name)
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return *this;
    }

    HdbppTxNewAttribute<Conn> &withTraits(
        Tango::AttrWriteType write, Tango::AttrDataFormat format, Tango::CmdArgType type)
    {
        _traits = AttributeTraits(write, format, type);
        _traits_set = true;
        return *this;
    }

    HdbppTxNewAttribute<Conn> &store();

    /// @brief Print the HdbppTxNewAttribute object to the stream
    void print(std::ostream &os) const noexcept override;

private:
    AttributeName _attr_name;
    AttributeTraits _traits;

    // force user to set traits
    bool _traits_set = false;
};

//=============================================================================
//=============================================================================
template<typename Conn>
HdbppTxNewAttribute<Conn> &HdbppTxNewAttribute<Conn>::store()
{
    if (_attr_name.empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (!_traits_set)
    {
        std::string msg {"AttributeTraits are not set. Unable to complete the transaction."};
        spdlog::error("Error: {} For attribute {}", msg, _attr_name);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        std::string msg {"The connection is reporting it is closed. Unable to store new attribute."};
        spdlog::error("Error: {} For attribute {}", msg, _attr_name);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    // check if this attribute exists in the database already, if it does
    // then it may have been removed and this is a case of readding it
    if (HdbppTxBase<Conn>::connection().fetchAttributeArchived(HdbppTxBase<Conn>::attrNameForStorage(_attr_name)))
    {
        // so it exists in the database, check its stored type
        AttributeTraits stored_traits = 
            HdbppTxBase<Conn>::connection().fetchAttributeTraits(HdbppTxBase<Conn>::attrNameForStorage(_attr_name));

        if (stored_traits != _traits)
        {
            // oops, someone is trying to change types, this is not supported yet, throw an exception
            std::string msg {"Attempt to add an attribute which is already stored with different type information."};
            spdlog::error("Error: {} For attribute {}", msg, _attr_name);
            Tango::Except::throw_exception("Consistency Error", msg, LOCATION_INFO);
        }

        // so it exists in the database and its type matches... check the last event
        auto last_event = HdbppTxBase<Conn>::connection().fetchLastHistoryEvent(
            HdbppTxBase<Conn>::attrNameForStorage(_attr_name));

        // ok, this attribute is being re-added after a remove, better check its
        // the same type
        if (last_event == events::RemoveEvent)
        {
            spdlog::info("Adding an attribute {} that is in a removed state, this is valid", _attr_name);

            // record the event as added again
            HdbppTxBase<Conn>::connection()
                .template createTx<HdbppTxHistoryEvent>()
                .withName(_attr_name.fqdnAttributeName())
                .withEvent(events::AddEvent)
                .store();
        }
        else
        {
            // someone is trying to add the same attribute over and over?
            std::string msg {"The attribute already exists in the database. Can not add again. "};
            spdlog::warn("Warning: {} For attribute {}", msg, _attr_name);

            // bad black box behaviour, this is not an error, in fact, the system
            // built top assume this undocumented behaviour!! 
        }
    }
    else
    {
        spdlog::info("Adding a new attribute to the system: {}", _attr_name);

        // attempt to store the new attribute into the database for the first time
        HdbppTxBase<Conn>::connection().storeAttribute(HdbppTxBase<Conn>::attrNameForStorage(_attr_name),
            _attr_name.tangoHostWithDomain(),
            _attr_name.domain(),
            _attr_name.family(),
            _attr_name.member(),
            _attr_name.name(),
            _traits);

        HdbppTxBase<Conn>::connection()
            .template createTx<HdbppTxHistoryEvent>()
            .withName(_attr_name.fqdnAttributeName())
            .withEvent(events::AddEvent)
            .store();
    }

    // set the result to true to indicate success
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxNewAttribute<Conn>::print(std::ostream &os) const noexcept
{
    os << "HdbppTxNewAttribute(base: ";
    HdbppTxBase<Conn>::print(os);

    os << ", "
       << "_traits: " << _traits << ", "
       << "_attr_name: " << _attr_name << ")";
}

} // namespace hdbpp
#endif // _HDBPP_TX_NEW_ATTRIBUTE_HPP
