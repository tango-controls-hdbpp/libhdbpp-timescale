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

#include "HdbppTimescaleDb.hpp"

#include "DbConnection.hpp"
#include "HdbppTxDataEvent.hpp"
#include "HdbppTxHistoryEvent.hpp"
#include "HdbppTxNewAttribute.hpp"
#include "HdbppTxParameterEvent.hpp"
#include "LibUtils.hpp"

#include <memory>
#include <vector>

using namespace std;

namespace hdbpp
{
// declaring this variable here removes it from the header, and keeps the header clean.
// It is allocated in the constructor. It can be abstracted further to allow easy plug
// in of different backends at a later point
unique_ptr<pqxx_conn::DbConnection> conn;

// simple class to gather utility functions that were previously part of HdbppTimescaleDb,
// removes them from the header and keeps it clean for includes
struct HdbppTimescaleDbUtils
{
    static string getConfigParam(const map<string, string> &conf, const string &param, bool mandatory);
    static map<string, string> extractConfig(vector<string> str, const string &separator);
};

//=============================================================================
//=============================================================================
map<string, string> HdbppTimescaleDbUtils::extractConfig(vector<string> config, const string &separator)
{
    map<string, string> results;

    for (auto &item : config)
    {
        auto found_separator = item.find_first_of(separator);

        if (found_separator != string::npos && found_separator > 0)
            results.insert(make_pair(item.substr(0, found_separator), item.substr(found_separator + 1)));
    }

    return results;
}

//=============================================================================
//=============================================================================
string HdbppTimescaleDbUtils::getConfigParam(const map<string, string> &conf, const string &param, bool mandatory)
{
    auto iter = conf.find(param);

    if (iter == conf.end() && mandatory)
    {
        std::string msg {"Configuration parsing error: mandatory configuration parameter: " + param + " not found"};
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    // for non-mandatory config params that have not been set, just return
    // an empty string
    return iter == conf.end() ? "" : (*iter).second;
}

//=============================================================================
//=============================================================================
HdbppTimescaleDb::HdbppTimescaleDb(const vector<string> &configuration)
{
    // convert the config vector to a map
    auto libhdb_conf = HdbppTimescaleDbUtils::extractConfig(configuration, "=");

    // logging_level optional config parameter ----
    auto level = HdbppTimescaleDbUtils::getConfigParam(libhdb_conf, "logging_level", false);

    LogConfigurator::initLogging(true, true);

    if (level == "ERROR" || level.empty())
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::err);
    else if (level == "WARNING")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::warn);
    else if (level == "INFO")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::info);
    else if (level == "DEBUG")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::debug);
    else if (level == "TRACE")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::trace);
    else if (level == "DISABLED")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::off);

    spdlog::info("Starting libhdbpp-timescale shared library...");

    // connect_string mandatory config parameter ----
    auto connection_string = HdbppTimescaleDbUtils::getConfigParam(libhdb_conf, "connect_string", true);
    spdlog::info("Manatory config parameter connect_string: {}", connection_string);

    // allocate a connection to store data with
    conn = make_unique<pqxx_conn::DbConnection>();

    // now bring up the connection
    conn->connect(connection_string);
    spdlog::info("Startied libhdbpp-timescale shared library successfully");
}

//=============================================================================
//=============================================================================
HdbppTimescaleDb::~HdbppTimescaleDb()
{
    if (conn->isOpen())
        conn->disconnect();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::insert_Attr(Tango::EventData *event_data, HdbEventDataType event_data_type)
{
    assert(event_data);
    assert(event_data->attr_value);

    // if there is an error, we store an error, since there will be no data passed in
    if (event_data->err)
    {
        conn->createTx<HdbppTxDataEvent>()
            .withName(event_data->attr_name)
            .withTraits(static_cast<Tango::AttrWriteType>(event_data_type.write_type),
                static_cast<Tango::AttrDataFormat>(event_data_type.data_format),
                event_data_type.data_type)
            .withError(string(event_data->errors[0].desc))
            .withEventTime(event_data->attr_value->get_date())
            .withQuality(static_cast<int>(event_data->attr_value->get_quality()))
            .store();
    }
    else
    {
        // build a data event request, this will store 0 or more data elements,
        // pending on type, format and quality
        conn->createTx<HdbppTxDataEvent>()
            .withName(event_data->attr_name)
            .withTraits(static_cast<Tango::AttrWriteType>(event_data_type.write_type),
                static_cast<Tango::AttrDataFormat>(event_data_type.data_format),
                event_data_type.data_type)
            .withAttribute(event_data->attr_value)
            .withEventTime(event_data->attr_value->get_date())
            .withQuality(static_cast<int>(event_data->attr_value->get_quality()))
            .store();
    }
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::insert_param_Attr(Tango::AttrConfEventData *conf_event_data, HdbEventDataType event_data_type)
{
    assert(conf_event_data);
    spdlog::trace("Insert parameter event request for attribute: {}", conf_event_data->attr_name);

    conn->createTx<HdbppTxParameterEvent>()
        .withName(conf_event_data->attr_name)
        .withEventTime(conf_event_data->get_date())
        .withAttrInfo(*(conf_event_data->attr_conf))
        .store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::configure_Attr(std::string fqdn_attr_name, int type, int format, int write_type, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("Insert new attribute request for attribute: {}", fqdn_attr_name);

    // TODO check if type already exists and if its changed type?

    // forgive the ugly casting, but for some reason we receive the enum values
    // already cast to ints, we cast them back to enums so they function as
    // enums again
    conn->createTx<HdbppTxNewAttribute>()
        .withName(fqdn_attr_name)
        .withTraits(static_cast<Tango::AttrWriteType>(write_type), static_cast<Tango::AttrDataFormat>(format), type)
        .store();

    // add a start event
    conn->createTx<HdbppTxHistoryEvent>().withName(fqdn_attr_name).withEvent(events::StartEvent).store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::updateTTL_Attr(std::string fqdn_attr_name, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("TTL event request for attribute: {}", fqdn_attr_name);

    // TODO implement
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::event_Attr(std::string fqdn_attr_name, unsigned char event)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("History event request for attribute: {}", fqdn_attr_name);
    conn->createTx<HdbppTxHistoryEvent>().withName(fqdn_attr_name).withEvent(event).store();
}
} // namespace hdbpp
