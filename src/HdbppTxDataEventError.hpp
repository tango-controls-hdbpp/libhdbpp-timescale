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

#ifndef _HDBPP_TX_DATA_EVENT_ERROR_HPP
#define _HDBPP_TX_DATA_EVENT_ERROR_HPP

#include "HdbppTxDataEventBase.hpp"

namespace hdbpp_internal
{
// When we receieve a data event with an error, we use this simplified transaction
// class to store it. This saves having to deduce type information and attempt
// to extract data as is done in HdbppTxDataEvent.
template<typename Conn>
class HdbppTxDataEventError : public HdbppTxDataEventBase<Conn, HdbppTxDataEventError>
{
private:
    // help clean up the code a little
    using Base = HdbppTxDataEventBase<Conn, HdbppTxDataEventError>;

public:
    HdbppTxDataEventError(Conn &conn) : HdbppTxDataEventBase<Conn, HdbppTxDataEventError>(conn) {}

    auto withError(const std::string &error_msg) -> HdbppTxDataEventError<Conn> &
    {
        _error_msg = error_msg;
        return *this;
    }

    // trigger the database storage routines
    auto store() -> HdbppTxDataEventError<Conn> &;

    /// @brief Print the HdbppTxDataEventError object to the stream
    void print(std::ostream &os) const noexcept override;

private:
    std::string _error_msg;
};

//=============================================================================
//=============================================================================
template<typename Conn>
auto HdbppTxDataEventError<Conn>::store() -> HdbppTxDataEventError<Conn> &
{
    if (Base::attributeName().empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (Base::attributeTraits().isInvalid())
    {
        std::string msg {"AttributeTraits are not set. Unable to complete the transaction."};
        msg += ". For attribute" + Base::attributeName().fqdnAttributeName();
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (_error_msg.empty())
    {
        std::string msg {"Error message is not set. Unable to complete the transaction."};
        msg += ". For attribute" + Base::attributeName().fqdnAttributeName();
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        string msg {"The connection is reporting it is closed. Unable to store data event."};
        msg += ". For attribute" + Base::attributeName().fqdnAttributeName();
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    // attempt to store the error in the database, any exceptions are left to
    // propergate to the caller
    HdbppTxBase<Conn>::connection().storeDataEventError(HdbppTxBase<Conn>::attrNameForStorage(Base::attributeName()),
        Base::eventTime(),
        Base::quality(),
        _error_msg,
        Base::attributeTraits());

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxDataEventError<Conn>::print(std::ostream &os) const noexcept
{
    os << "HdbppTxDataEventError(base: ";
    HdbppTxDataEventBase<Conn, HdbppTxDataEventError>::print(os);

    os << ", "
       << "_error_msg: " << _error_msg << ")";
}

} // namespace hdbpp_internal
#endif // _HDBPP_TX_DATA_EVENT_ERROR_HPP
