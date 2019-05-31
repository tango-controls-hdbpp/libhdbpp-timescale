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

#include "AttributeTraits.hpp"
#include "HdbppTxBase.hpp"

#include <iostream>
#include <string>

namespace hdbpp
{
template<typename Conn>
class HdbppTxDataEvent : public HdbppTxBase<Conn>
{
public:
    // TODO print()
    // TODO check fully tested
    // TODO when storing INVALID events, i.e. empty or bad quality, do we need to generate an event time, or is the given one
    // still valid?

    HdbppTxDataEvent(Conn &conn) : HdbppTxBase<Conn>(conn) {}
    virtual ~HdbppTxDataEvent() {}

    HdbppTxDataEvent<Conn> &withName(const std::string &fqdn_attr_name)
    {
        _attr_name = AttributeName {fqdn_attr_name};
        return *this;
    }

    HdbppTxDataEvent<Conn> &withTraits(Tango::AttrWriteType write, Tango::AttrDataFormat format, unsigned int type)
    {
        _traits = AttributeTraits(write, format, type);
        _traits_set = true;
        return *this;
    }

    HdbppTxDataEvent<Conn> &withAttribute(Tango::DeviceAttribute *dev_attr)
    {
        // just set the pointer here, we will do a full event data extraction at
        // point of storage, this reduces complexity but limits the functionality, i.e
        // we can not safely queue these events
        _dev_attr = dev_attr;
        return *this;
    }

    HdbppTxDataEvent<Conn> &withEventTime(Tango::TimeVal tv)
    {
        _event_time = tv.tv_sec + tv.tv_usec / 1.0e6;
        return *this;
    }

    HdbppTxDataEvent<Conn> &withQuality(int quality)
    {
        _quality = quality;
        return *this;
    }

    HdbppTxDataEvent<Conn> &withError(const string &error_msg)
    {
        _error_msg = error_msg;
        return *this;
    }

    HdbppTxDataEvent<Conn> &store();

    /// @brief Print the HdbppTxDataEvent object to the stream
    void print(std::ostream &os) const override;

private:
    // perform the actual storage for the type, this template helps
    // resolve the fact we are storing many different types via this tx
    // class
    template<typename T>
    void doStore();

    // store an error to the database
    void doStoreError();

    AttributeName _attr_name;
    AttributeTraits _traits;

    std::string _error_msg;
    int _quality = Tango::ATTR_INVALID;

    // the device attribute to extract the value from
    Tango::DeviceAttribute *_dev_attr = nullptr;

    // time this parameter change event was generated
    double _event_time = 0;

    // force user to set traits
    bool _traits_set = false;
};

//=============================================================================
//=============================================================================
template<typename Conn>
HdbppTxDataEvent<Conn> &HdbppTxDataEvent<Conn>::store()
{
    if (_attr_name.empty())
    {
        std::string msg {"AttributeName is reporting empty. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (!_traits_set)
    {
        std::string msg {"AttributeTraits are not set. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (!_dev_attr)
    {
        std::string msg {"Device Attribute is not set. Unable to complete the transaction."};
        throw std::invalid_argument(msg);
    }
    else if (HdbppTxBase<Conn>::connection().isClosed())
    {
        string msg {"The connection is reporting it is closed. Unable to store data event."};
        throw std::invalid_argument(msg);
    }

    // disable is_empty exception
    _dev_attr->reset_exceptions(Tango::DeviceAttribute::isempty_flag);

    // check for error messages, if we have one set, assume this is an error
    // event and store it as so
    if (!_error_msg.empty())
    {
        // store the error message
        doStoreError();
    }
    else
    {
        // translate the Tango Type into a C++ type via templates, inside
        // doStore the data is extracted and then stored
        switch (_traits.type())
        {
            //case Tango::DEV_BOOLEAN: this->template doStore<bool>();
            case Tango::DEV_SHORT: this->template doStore<int16_t>();
            case Tango::DEV_LONG: this->template doStore<int32_t>();
            case Tango::DEV_LONG64: this->template doStore<int64_t>();
            case Tango::DEV_FLOAT: this->template doStore<float>();
            case Tango::DEV_DOUBLE: this->template doStore<double>();
            case Tango::DEV_UCHAR: this->template doStore<uint8_t>();
            case Tango::DEV_USHORT: this->template doStore<uint16_t>();
            case Tango::DEV_ULONG: this->template doStore<uint32_t>();
            case Tango::DEV_ULONG64: this->template doStore<uint64_t>();
            case Tango::DEV_STRING: this->template doStore<std::string>();
            case Tango::DEV_STATE:
                this->template doStore<int32_t>();

                // TODO enable these calls.
                //case Tango::DEV_ENUM: this->template doStore<?>();
                //case Tango::DEV_ENCODED: this->template doStore<vector<uint8_t>>();
        }
    }

    // success in running the store command, so set the result as true
    HdbppTxBase<Conn>::setResult(true);
    return *this;
}

//=============================================================================
//=============================================================================
template<typename Conn>
template<typename T>
void HdbppTxDataEvent<Conn>::doStore()
{
    auto value = [this](auto extractor) {
        // this is the return, a unique ptr potentially with a vector in
        auto value = make_unique<std::vector<T>>();

        // its possible in some cases to get events that are empty or invalid,
        // we still store the event, but with no event data, so filter them
        // here, and if we detect one, do not extract data, instead return
        // a vector with no elements in
        if (!_dev_attr->is_empty() && _quality != Tango::ATTR_INVALID)
        {
            // attempt to extract data, if none is received then clear
            // the unique_ptr as a signal to following functions there is no data
            if (!extractor(*value))
            {
                // TODO convert to std exception
                //std::stringstream error_desc;

                //error_desc << "Failed to extract the attribute " << data->attr_name
                //<< " from the Tango EventData. Possible type mismatch?" << ends;

                //LOG(Utils::Error) << error_desc.str() << endl;
                //Tango::Except::throw_exception(EXCEPTION_TYPE_TYPE_MISMATCH, error_desc.str(), __func__);
            }
        }

        // release ownership of the unique_ptr back to the caller
        return std::move(value);
    };

    // attempt to store the error in the database, any exceptions are left to
    // propergate to the caller
    HdbppTxBase<Conn>::connection().template storeDataEvent<T>(HdbppTxBase<Conn>::attrNameForStorage(_attr_name),
        _event_time,
        _quality,
        std::move(value([this](std::vector<T> &v) { return _dev_attr->extract_read(v); })),
        std::move(value([this](std::vector<T> &v) { return _dev_attr->extract_set(v); })),
        _traits);
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxDataEvent<Conn>::doStoreError()
{
    // attempt to store the error in the database, any exceptions are left to
    // propergate to the caller
    HdbppTxBase<Conn>::connection().storeDataEventError(
        HdbppTxBase<Conn>::attrNameForStorage(_attr_name), _event_time, _quality, _error_msg, _traits);
}

//=============================================================================
//=============================================================================
template<typename Conn>
void HdbppTxDataEvent<Conn>::print(std::ostream &os) const
{
    HdbppTxBase<Conn>::print(os);

    //os << "_attr_name: " << _attr_name;
}

} // namespace hdbpp
#endif // _HDBPP_TX_DATA_EVENT_HPP
