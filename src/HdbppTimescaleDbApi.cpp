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

#include "HdbppTimescaleDbApi.hpp"

#include "DbConnection.hpp"
#include "HdbppTxDataEvent.hpp"
#include "HdbppTxDataEventError.hpp"
#include "HdbppTxHistoryEvent.hpp"
#include "HdbppTxNewAttribute.hpp"
#include "HdbppTxParameterEvent.hpp"
#include "HdbppTxUpdateTtl.hpp"
#include "LibUtils.hpp"

#include <locale>

using namespace std;
using namespace hdbpp_internal;

namespace hdbpp
{
// simple class to gather utility functions
struct HdbppTimescaleDbApiUtils
{
    static auto getConfigParam(const map<string, string> &conf, const string &param, bool mandatory) -> string;
    static auto extractConfig(const vector<string> &config, const string &separator) -> map<string, string>;
};

//=============================================================================
//=============================================================================
auto HdbppTimescaleDbApiUtils::extractConfig(const vector<string> &config, const string &separator) -> map<string, string>
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
auto HdbppTimescaleDbApiUtils::getConfigParam(const map<string, string> &conf, const string &param, bool mandatory) -> string
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
HdbppTimescaleDbApi::HdbppTimescaleDbApi(const string &id, const vector<string> &configuration) : _identity(id)
{
    auto param_to_lower = [](auto param) {
        locale loc;
        string tmp;

        for (string::size_type i = 0; i < param.length(); ++i)
            tmp += tolower(param[i], loc);

        return tmp;
    };

    // convert the config vector to a map
    auto libhdb_conf = HdbppTimescaleDbApiUtils::extractConfig(configuration, "=");

    // logging_level optional config parameter ----
    auto level = param_to_lower(HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "logging_level", false));
    auto log_file = HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "log_file", false);
    auto log_console = HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "log_console", false);
    auto log_syslog = HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "log_syslog", false);
    auto log_file_name = HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "log_file_name", false);

    // init the base logging system
    LogConfigurator::initLogging(_identity);

    if (param_to_lower(log_file) == "true")
        LogConfigurator::initFileLogging(_identity, log_file_name);

    if (param_to_lower(log_console) == "true")
        LogConfigurator::initConsoleLogging(_identity);

    if (param_to_lower(log_syslog) == "true")
        LogConfigurator::initSyslogLogging(_identity);

    if (level == "error" || level.empty())
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::err);
    else if (level == "warning")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::warn);
    else if (level == "info")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::info);
    else if (level == "debug")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::debug);
    else if (level == "trace")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::trace);
    else if (level == "disabled")
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::off);
    else
        LogConfigurator::setLoggingLevel(spdlog::level::level_enum::err);

    spdlog::info("Logging level: {}", level);
    spdlog::info("Logging to console: {}", log_console);
    spdlog::info("Logging to syslog: {}", log_syslog);
    spdlog::info("Logging to file: {}", log_file);
    spdlog::info("Logfile (if any): {}", log_file_name);

    spdlog::info("Starting libhdbpp-timescale shared library...");

    // connect_string mandatory config parameter ----
    auto connection_string = HdbppTimescaleDbApiUtils::getConfigParam(libhdb_conf, "connect_string", true);
    spdlog::info("Mandatory config parameter connect_string: {}", connection_string);

    // allocate a connection to store data with
    _conn = make_unique<pqxx_conn::DbConnection>(pqxx_conn::DbConnection::DbStoreMethod::PreparedStatement);

    // now bring up the connection
    _conn->connect(connection_string);

    spdlog::info("Started libhdbpp-timescale shared library successfully");
}

//=============================================================================
//=============================================================================
HdbppTimescaleDbApi::~HdbppTimescaleDbApi()
{
    if (_conn->isOpen())
        _conn->disconnect();

    LogConfigurator::shutdownLogging(_identity);
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::insert_event(Tango::EventData *event_data, const HdbEventDataType &data_type)
{
    assert(event_data);
    assert(event_data->attr_value);
    spdlog::trace("Insert data event for attribute: {}", event_data->attr_name);

    // hand the call to the internal routine
    doInsertEvent(event_data, data_type);
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::insert_events(vector<tuple<Tango::EventData *, HdbEventDataType>> events)
{
    _conn->buffer(true);

    try
    {
        for (auto event : events)
            doInsertEvent(get<0>(event), get<1>(event));

        _conn->flush();
    }
    catch (Tango::DevFailed &e)
    {
        // ensure this is disabled on error
        _conn->buffer(false);
        throw;
    }

    _conn->buffer(false);
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::insert_param_event(
    Tango::AttrConfEventData *param_event, const HdbEventDataType & /* data_type */)
{
    assert(param_event);
    spdlog::trace("Insert parameter event request for attribute: {}", param_event->attr_name);

    _conn->createTx<HdbppTxParameterEvent>()
        .withName(param_event->attr_name)
        .withEventTime(param_event->get_date())
        .withAttrInfo(*(param_event->attr_conf))
        .store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::add_attribute(const std::string &fqdn_attr_name, int type, int format, int write_type)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("Insert new attribute request for attribute: {}", fqdn_attr_name);

    // forgive the ugly casting, but for some reason we receive the enum values
    // already cast to ints, we cast them back to enums so they function as
    // enums again
    _conn->createTx<HdbppTxNewAttribute>()
        .withName(fqdn_attr_name)
        .withTraits(static_cast<Tango::AttrWriteType>(write_type),
            static_cast<Tango::AttrDataFormat>(format),
            static_cast<Tango::CmdArgType>(type))
        .withTtl(0)
        .store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::update_ttl(const std::string &fqdn_attr_name, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("TTL event request for attribute: {}, with ttl: {}", fqdn_attr_name, ttl);
    _conn->createTx<HdbppTxUpdateTtl>().withName(fqdn_attr_name).withTtl(ttl).store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::insert_history_event(const std::string &fqdn_attr_name, unsigned char event)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("History event request for attribute: {}", fqdn_attr_name);
    _conn->createTx<HdbppTxHistoryEvent>().withName(fqdn_attr_name).withEvent(event).store();
}

//=============================================================================
//=============================================================================
auto HdbppTimescaleDbApi::supported(HdbppFeatures feature) -> bool
{
    auto supported = false;

    switch (feature)
    {
        case HdbppFeatures::TTL: supported = true; break;

        case HdbppFeatures::BATCH_INSERTS: supported = true; break;
    }

    return supported;
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbApi::doInsertEvent(Tango::EventData *event_data, const HdbEventDataType &data_type)
{
    // if there is an error, we store an error, since there will be no data passed in
    if (event_data->err)
    {
        spdlog::trace("Event type is error for attribute: {}", event_data->attr_name);

        // no time data is passed for errors, so make something up
        struct timeval tv
        {};

        struct Tango::TimeVal tango_tv
        {};

        gettimeofday(&tv, nullptr);
        tango_tv.tv_sec = tv.tv_sec;
        tango_tv.tv_usec = tv.tv_usec;
        tango_tv.tv_nsec = 0;

        _conn->createTx<HdbppTxDataEventError>()
            .withName(event_data->attr_name)
            .withTraits(static_cast<Tango::AttrWriteType>(data_type.write_type),
                static_cast<Tango::AttrDataFormat>(data_type.data_format),
                static_cast<Tango::CmdArgType>(data_type.data_type))
            .withError(string(event_data->errors[0].desc))
            .withEventTime(tango_tv)
            .withQuality(event_data->attr_value->get_quality())
            .store();
    }
    else
    {
        spdlog::trace("Event type is data for attribute: {}", event_data->attr_name);

        // build a data event request, this will store 0 or more data elements,
        // pending on type, format and quality
        _conn->createTx<HdbppTxDataEvent>()
            .withName(event_data->attr_name)
            .withTraits(static_cast<Tango::AttrWriteType>(data_type.write_type),
                static_cast<Tango::AttrDataFormat>(data_type.data_format),
                static_cast<Tango::CmdArgType>(data_type.data_type))
            .withAttribute(event_data->attr_value)
            .withEventTime(event_data->attr_value->get_date())
            .withQuality(event_data->attr_value->get_quality())
            .store();
    }
}

} // namespace hdbpp
