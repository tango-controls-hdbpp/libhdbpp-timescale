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

#ifndef _HDBPP_TX_UPDATE_TTL_HPP
#define _HDBPP_TX_UPDATE_TTL_HPP

#include "AttributeTraits.hpp"
#include "HdbppTxBase.hpp"
#include "LibUtils.hpp"

#include <iostream>
#include <string>

namespace hdbpp_internal
{
// Update the time to live (ttl) for the attribute in the database.
template<typename Conn>
class HdbppTxUpdateTtl : public HdbppTxBase<Conn>
{
public:
    HdbppTxUpdateTtl(Conn &conn) : HdbppTxBase<Conn>(conn) {}

    auto withName(const std::string &fqdn_attr_name) -> HdbppTxUpdateTtl<Conn> &
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return *this;
    }

    auto withTtl(unsigned int ttl) -> HdbppTxUpdateTtl<Conn> &
    {
        _ttl = ttl;
        return *this;
    }

    // trigger the database storage routines
    auto store() -> HdbppTxUpdateTtl<Conn> &;

    /// @brief Print the HdbppTxUpdateTtl object to the stream
    void print(std::ostream &os) const noexcept override;

private:
    AttributeName _attr_name;

    // ttl for the attribute
    unsigned int _ttl = 0;
};

//=============================================================================
//=============================================================================
template<typename Conn>
auto HdbppTxUpdateTtl<Conn>::store() -> HdbppTxUpdateTtl<Conn> &
{
    if (_attr_name.empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        string msg {"The connection is reporting it is closed. Unable to store parameter event. For attribute" +
            _attr_name.fqdnAttributeName()};

        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    auto prepared_attr_name = HdbppTxBase<Conn>::attrNameForStorage(_attr_name);

    // check if this attribute exists in the database, if it does not we have
    // an error condition
    if (HdbppTxBase<Conn>::connection().fetchAttributeArchived(prepared_attr_name))
    {
        // now store the parameter event
        HdbppTxBase<Conn>::connection().storeAttributeTtl(HdbppTxBase<Conn>::attrNameForStorage(_attr_name), _ttl);
    }
    else
    {
        // attribute does not exist, this is an error condition
        std::string msg {"Attempt to update the ttl for an attribute that does not exist. Attribute: " +
            _attr_name.fqdnAttributeName()};

        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Consistency Error", msg, LOCATION_INFO);
    }

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxUpdateTtl<Conn>::print(std::ostream &os) const noexcept
{
    // TODO can not print tango objects, the operator<< are not const correct!

    os << "HdbppTxUpdateTtl(base: ";
    HdbppTxBase<Conn>::print(os);

    os << ", "
       << "_attr_name: " << _attr_name << ", "
       << "_ttl: " << _ttl << ")";
}

} // namespace hdbpp_internal
#endif // _HDBPP_TX_UPDATE_TTL_HPP
