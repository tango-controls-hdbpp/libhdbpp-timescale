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

#include "HdbppTxDataEvent.hpp"
#include "HdbppTxHistoryEvent.hpp"
#include "HdbppTxNewAttribute.hpp"
#include "HdbppTxParameterEvent.hpp"
#include "LibUtils.hpp"

using namespace std;

namespace hdbpp
{
//=============================================================================
//=============================================================================
HdbppTimescaleDb::HdbppTimescaleDb(const vector<string> &configuration)
{
    // convert the config vector to a map
    auto libhdb_conf = extractConfig(configuration, "=");

    // ---- logging_level optional config parameter ----
    auto config = getConfigParam(libhdb_conf, "logging_level", false);
    setLibraryLoggingLevel(config);

    // ---- user mandatory config parameter ----
    auto user = getConfigParam(libhdb_conf, "user", true);

    // ---- password mandatory config parameter ----
    auto password = getConfigParam(libhdb_conf, "password", true);

    // ---- host mandatory config parameter ----
    auto host = getConfigParam(libhdb_conf, "host", true);

    // ---- port mandatory config parameter ----
    auto port = getConfigParam(libhdb_conf, "port", true);

    // ---- dbname mandatory config parameter ----
    auto dbname = getConfigParam(libhdb_conf, "dbname", true);

    auto connection_string = "user=" + user + "password=" + password + "host=" + host + "dbname=" + dbname;

    try
    {
        // now bring up the connection
        _connection.connect(connection_string);
    }
    catch (const connection_error &ex)
    {
        // TODO Tango exception
    }
}

//=============================================================================
//=============================================================================
HdbppTimescaleDb::~HdbppTimescaleDb()
{
    if (_connection.isOpen())
        _connection.disconnect();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::insert_Attr(Tango::EventData *event_data, HdbEventDataType event_data_type)
{
    assert(event_data);
    assert(event_data->attr_value);

    try
    {
        // if there is an error, we archive an error, since there will be no data passed in
        if (event_data->err)
        {
            _connection.createTx<HdbppTxDataEvent>()
                .withName(event_data->attr_name)
                .withTraits(
                    static_cast<Tango::AttrWriteType>(event_data_type.write_type),
                    static_cast<Tango::AttrDataFormat>(event_data_type.data_format),
                    event_data_type.data_type)
                .withError(string(event_data->errors[0].desc))
                .withEventTime(event_data->attr_value->get_date())
                .withQuality(static_cast<int>(event_data->attr_value->get_quality()))
                .store();
        }
        else
        {
            // build a data event request
            _connection.createTx<HdbppTxDataEvent>()
                .withName(event_data->attr_name)
                .withTraits(
                    static_cast<Tango::AttrWriteType>(event_data_type.write_type),
                    static_cast<Tango::AttrDataFormat>(event_data_type.data_format),
                    event_data_type.data_type)
                .withAttribute(event_data->attr_value)
                .withEventTime(event_data->attr_value->get_date())
                .withQuality(static_cast<int>(event_data->attr_value->get_quality()))
                .store();
        }
    }
    catch (const exception &e)
    {
        // TODO handle
    }
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::insert_param_Attr(Tango::AttrConfEventData *conf_event_data, HdbEventDataType event_data_type)
{
    assert(conf_event_data);
    _logger->trace("Insert parameter event request for attribute: {}", conf_event_data->attr_name);

    try
    {
        _connection.createTx<HdbppTxParameterEvent>()
            .withName(conf_event_data->attr_name)
            .withEventTime(conf_event_data->get_date())
            .withAttrInfo(*(conf_event_data->attr_conf))
            .store();
    }
    catch (const exception &e)
    {
        // TODO deal with this, convert back to Tango
    }
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::configure_Attr(
    std::string fqdn_attr_name, int type, int format, int write_type, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    _logger->trace("Insert new attribute request for attribute: {}", fqdn_attr_name);

    // TODO check if type already exists and if its changed type?

    try
    {
        // forgive the ugly casting, but for some reason we receive the enum values
        // already cast to ints, we cast them back to enums so they function as
        // enums again
        _connection.createTx<HdbppTxNewAttribute>()
            .withName(fqdn_attr_name)
            .withTraits(static_cast<Tango::AttrWriteType>(write_type), static_cast<Tango::AttrDataFormat>(format), type)
            .store();

        // add a start event
        _connection.createTx<HdbppTxHistoryEvent>()
            .withName(fqdn_attr_name)
            .withEvent(events::StartEvent)
            .store();
    }
    catch (const exception &ex)
    {
        // TODO deal with this, convert back to Tango
    }
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::updateTTL_Attr(std::string fqdn_attr_name, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    _logger->trace("TTL event request for attribute: {}", fqdn_attr_name);

    // TODO implement
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::event_Attr(std::string fqdn_attr_name, unsigned char event)
{
    assert(!fqdn_attr_name.empty());
    _logger->trace("History event request for attribute: {}", fqdn_attr_name);

    try
    {
        _connection.createTx<HdbppTxHistoryEvent>().withName(fqdn_attr_name).withEvent(event).store();
    }
    catch (const exception &e)
    {
        // TODO deal with this, convert back to Tango
    }
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDb::setLibraryLoggingLevel(const string &level)
{
    LogConfigurator::initLogging(true, true);
    _logger = spdlog::get(LibLoggerName);

    if (level == "ERROR")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::err);
    else if (level == "WARNING")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::warn);
    else if (level == "INFO")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::info);
    else if (level == "DEBUG")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::debug);
    else if (level == "DEBUG")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::trace);
    else if (level == "DISABLED")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::off);
}

//=============================================================================
//=============================================================================
map<string, string> HdbppTimescaleDb::extractConfig(vector<string> str, const string &separator)
{
    map<string, string> results;

    for (auto &it : str)
    {
        string::size_type found_eq;
        found_eq = it.find_first_of(separator);

        if (found_eq != string::npos && found_eq > 0)
            results.insert(make_pair(it.substr(0, found_eq), it.substr(found_eq + 1)));
    }

    return results;
}

//=============================================================================
//=============================================================================
string HdbppTimescaleDb::getConfigParam(const map<string, string> &conf, const string &param, bool mandatory)
{
    auto iter = conf.find(param);

    if (iter == conf.end() && mandatory)
    {
        stringstream error_desc;

        error_desc << "Configuration parsing error: mandatory configuration parameter: " << param << " not found"
                   << ends;

        _logger->error("{}", error_desc.str());
        throw invalid_argument(error_desc.str());
    }

    if (iter != conf.end())
    {
        _logger->debug("Configuration: {} set to: {}", param, (*iter).second);
    }
    else
    {
        _logger->debug("Configuration: {} not found", param);
    }

    // for non-mandatory config params that have not been set, just return
    // an empty string
    return iter == conf.end() ? "" : (*iter).second;
}

} // namespace hdbpp
