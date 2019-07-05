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

#ifndef _HDBPP_TX_DATA_EVENT_HPP
#define _HDBPP_TX_DATA_EVENT_HPP

#include "HdbppTxDataEventBase.hpp"

namespace hdbpp
{
template<typename Conn>
class HdbppTxDataEvent : public HdbppTxDataEventBase<Conn, HdbppTxDataEvent>
{
private:
    // help clean up the code a little
    using Base = HdbppTxDataEventBase<Conn, HdbppTxDataEvent>;

public:
    HdbppTxDataEvent(Conn &conn) : HdbppTxDataEventBase<Conn, HdbppTxDataEvent>(conn) {}
    virtual ~HdbppTxDataEvent() {}

    HdbppTxDataEvent<Conn> &withAttribute(Tango::DeviceAttribute *dev_attr)
    {
        // just set the pointer here, we will do a full event data extraction at
        // point of storage, this reduces complexity but limits the functionality, i.e
        // we can not safely queue these events
        _dev_attr = dev_attr;
        return *this;
    }

    HdbppTxDataEvent<Conn> &store();

    /// @brief Print the HdbppTxDataEvent object to the stream
    void print(std::ostream &os) const noexcept override;

private:
    // perform the actual storage for the type, this template helps
    // resolve the fact we are storing many different types via this tx
    // class
    template<typename T, typename ReadFunctor, typename WriteFunctor>
    void doStore(ReadFunctor extract_read, WriteFunctor extract_write);

    // the device attribute to extract the value from
    Tango::DeviceAttribute *_dev_attr = nullptr;
};

//=============================================================================
//=============================================================================
template<typename Conn>
HdbppTxDataEvent<Conn> &HdbppTxDataEvent<Conn>::store()
{
    if (Base::attributeName().empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (!Base::traitsSet())
    {
        std::string msg {"AttributeTraits are not set. Unable to complete the transaction."};
        spdlog::error("Error: {} For attribute {}", msg, Base::attributeName());
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (!_dev_attr)
    {
        std::string msg {"Device Attribute is not set. Unable to complete the transaction."};
        spdlog::error("Error: {} For attribute {}", msg, Base::attributeName());
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        string msg {"The connection is reporting it is closed. Unable to store data event."};
        spdlog::error("Error: {} For attribute {}", msg, Base::attributeName());
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    // disable is_empty exception
    _dev_attr->reset_exceptions(Tango::DeviceAttribute::isempty_flag);

    auto read_extractor = [this](auto &v) { return _dev_attr->extract_read(v); };
    auto write_extractor = [this](auto &v) { return _dev_attr->extract_set(v); };

    // translate the Tango Type into a C++ type via templates, inside
    // doStore the data is extracted and then stored
    switch (Base::attributeTraits().type())
    {
        case Tango::DEV_BOOLEAN: this->template doStore<bool>(read_extractor, write_extractor); break;
        case Tango::DEV_SHORT: this->template doStore<int16_t>(read_extractor, write_extractor); break;
        case Tango::DEV_LONG: this->template doStore<int32_t>(read_extractor, write_extractor); break;
        case Tango::DEV_LONG64: this->template doStore<int64_t>(read_extractor, write_extractor); break;
        case Tango::DEV_FLOAT: this->template doStore<float>(read_extractor, write_extractor); break;
        case Tango::DEV_DOUBLE: this->template doStore<double>(read_extractor, write_extractor); break;
        case Tango::DEV_UCHAR: this->template doStore<uint8_t>(read_extractor, write_extractor); break;
        case Tango::DEV_USHORT: this->template doStore<uint16_t>(read_extractor, write_extractor); break;
        case Tango::DEV_ULONG: this->template doStore<uint32_t>(read_extractor, write_extractor); break;
        case Tango::DEV_ULONG64: this->template doStore<uint64_t>(read_extractor, write_extractor); break;
        case Tango::DEV_STRING: this->template doStore<std::string>(read_extractor, write_extractor); break;

        case Tango::DEV_STATE: 
            if (Base::attributeTraits().formatType() == Tango::SCALAR)
            {
                auto state_scalar_read_extractor = [this](auto &v) {
                    Tango::DevState state;
                    bool read_state = ((*_dev_attr) >> state);

                    if (read_state)
                        v.push_back(state);

                    return read_state;
                };

                this->template doStore<Tango::DevState>(state_scalar_read_extractor, write_extractor);
            }
            else
                this->template doStore<Tango::DevState>(read_extractor, write_extractor); 

            break;

        //case Tango::DEV_ENUM: this->template doStoreEnum<?>(); break;
        //case Tango::DEV_ENCODED: this->template doStoreEncoded<vector<uint8_t>>(); break;
        default:
            std::string msg {
                "HdbppTxDataEvent built for unsupported type: " + std::to_string(Base::attributeTraits().type()) +
                ", for attribute: [" + Base::attributeName().fullAttributeName() + "]"};

            spdlog::error("Error: {}", msg);
            Tango::Except::throw_exception("Runtime Error", msg, LOCATION_INFO);
    }

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
template<typename T, typename ReadFunctor, typename WriteFunctor>
void HdbppTxDataEvent<Conn>::doStore(ReadFunctor extract_read, WriteFunctor extract_write)
{
    // this is the general extractor algorithm, it is primed for a means to do the
    // actual extraction, the split allows some variation in types to be dealt with.
    auto value = [this](auto extractor, bool has_data, const std::string &write_type) {
        // this is the return, a unique ptr potentially with a vector in
        auto value = make_unique<std::vector<T>>();

        // its possible in some cases to get events that are empty or invalid,
        // we still store the event, but with no event data, so filter them
        // here, and if we detect one, do not extract data, instead return
        // a vector with no elements in
        if (has_data && !_dev_attr->is_empty() && Base::quality() != Tango::ATTR_INVALID)
        {
            // attempt to extract data, if none is received then clear
            // the unique_ptr as a signal to following functions there is no data
            if (!extractor(*value))
            {
                std::stringstream msg;

                msg << "Failed to extract the attribute data for attribute: ["
                    << Base::attributeName().fullAttributeName()
                    << "]. Traits: ["
                    << Base::attributeTraits()
                    << "], and read action ["
                    << write_type 
                    << "]";

                spdlog::error("Error: {}", msg.str());
                Tango::Except::throw_exception("Runtime Error", msg.str(), LOCATION_INFO);
            }
        }
        // log some more unusual conditions
        else if (Base::quality() == Tango::ATTR_INVALID)
        {
            spdlog::trace("Quality is {} for attribute: [{}] (write type: {}), no data extracted",
                Base::quality(),
                Base::attributeName().fqdnAttributeName(),
                write_type);
        }
        else if (_dev_attr->is_empty())
        {
            spdlog::debug("Attribute [{}] (write type: {}), empty, no data extracted",
                Base::attributeName().fqdnAttributeName(),
                write_type);
        }

        // release ownership of the unique_ptr back to the caller
        return std::move(value);
    };

    // attempt to store the error in the database, any exceptions are left to
    // propergate to the caller
    HdbppTxBase<Conn>::connection().template storeDataEvent<T>(
        HdbppTxBase<Conn>::attrNameForStorage(Base::attributeName()),
        Base::eventTime(),
        Base::quality(),
        std::move(value(extract_read, Base::attributeTraits().hasReadData(), "read")),
        std::move(value(extract_write, Base::attributeTraits().hasWriteData(), "set")),
        Base::attributeTraits());
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxDataEvent<Conn>::print(std::ostream &os) const noexcept
{
    // TODO can not print tango objects, the operator<< are not const correct!

    os << "HdbppTxDataEvent(base: ";
    HdbppTxDataEventBase<Conn, HdbppTxDataEvent>::print(os);
    os << ")";
}

} // namespace hdbpp
#endif // _HDBPP_TX_DATA_EVENT_HPP
