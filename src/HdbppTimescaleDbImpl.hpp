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

#ifndef _HDBPP_TIMESCALE_IMPL_HPP
#define _HDBPP_TIMESCALE_IMPL_HPP

#include <hdb++/AbstractDB.h>
#include "DbConnection.hpp"
#include <string>
#include <tango.h>
#include <vector>
#include <memory>

namespace hdbpp
{
class HdbppTimescaleDbImpl : public AbstractDB
{
public:
    
    // Takes a list of configuration parameters to start the driver with
    HdbppTimescaleDbImpl(const string &id, const std::vector<std::string> &configuration);

    virtual ~HdbppTimescaleDbImpl();

    // Inserts an attribute archive event for the EventData into the database. If the attribute
    // does not exist in the database, then an exception will be raised. If the attr_value
    // field of the data parameter if empty, then the attribute is in an error state
    // and the error message will be archived.
    void insert_event(Tango::EventData *event_data, const HdbEventDataType &data_type) override;

    // Insert multiple attribute archive events. Any attributes that do not exist will
    // cause an exception. On failure the fall back is to insert events individually
    void insert_events(std::vector<std::tuple<Tango::EventData*, HdbEventDataType>> events) override;

    // Inserts the attribute configuration data (Tango Attribute Configuration event data)
    // into the database. The attribute must be configured to be stored in HDB++,
    // otherwise an exception will be thrown.
    void insert_param_event(Tango::AttrConfEventData *param_event, const HdbEventDataType &/* data_type */) override;

    // Add an attribute to the database. Trying to add an attribute that already exists will
    // cause an exception
    void add_attribute(const std::string &fqdn_attr_name, int type, int format, int write_type, unsigned int ttl) override;

    // Update the attribute ttl. The attribute must have been configured to be stored in
    // HDB++, otherwise an exception is raised
    void update_ttl(const std::string &fqdn_attr_name, unsigned int ttl) override;

    // Inserts a history event for the attribute name passed to the function. The attribute
    // must have been configured to be stored in HDB++, otherwise an exception is raised.
    // This function will also insert an additional CRASH history event before the START
    // history event if the given event parameter is DB_START and if the last history event
    // stored was also a START event.
    void insert_history_event(const std::string &fqdn_attr_name, unsigned char event) override;

    // Check what hdbpp features this library supports. This library supports: TTL, BATCH_INSERTS
    bool supported(HdbppFeatures feature) override;


private:

    std::unique_ptr<hdbpp_internal::pqxx_conn::DbConnection> Conn;
};

} // namespace hdbpp
#endif // _HDBPP_TIMESCALE_IMPL_HPP
