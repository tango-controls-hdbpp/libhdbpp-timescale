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

#include "HdbppTimescaleDbImpl.hpp"
#include "HdbppTxDataEvent.hpp"
#include "HdbppTxDataEventError.hpp"
#include "HdbppTxHistoryEvent.hpp"
#include "HdbppTxNewAttribute.hpp"
#include "HdbppTxParameterEvent.hpp"
#include "HdbppTxUpdateTtl.hpp"
#include "LibUtils.hpp"

#include <locale>
#include <vector>

using namespace std;
using namespace hdbpp_internal;

namespace hdbpp
{

// simple class to gather utility functions
struct HdbppTimescaleDbImplUtils
{
    static string getConfigParam(const map<string, string> &conf, const string &param, bool mandatory);
    static map<string, string> extractConfig(const vector<string> &config, const string &separator);
};

//=============================================================================
//=============================================================================
map<string, string> HdbppTimescaleDbImplUtils::extractConfig(const vector<string> &config, const string &separator)
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
string HdbppTimescaleDbImplUtils::getConfigParam(const map<string, string> &conf, const string &param, bool mandatory)
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
HdbppTimescaleDbImpl::HdbppTimescaleDbImpl(const string &id, const vector<string> &configuration)
{
    auto param_to_lower = [](auto param) {
        locale loc;
        string tmp;

        for (string::size_type i = 0; i < param.length(); ++i)
            tmp += tolower(param[i], loc);

        return tmp;
    };

    // convert the config vector to a map
    auto libhdb_conf = HdbppTimescaleDbImplUtils::extractConfig(configuration, "=");

    // logging_level optional config parameter ----
    auto level = param_to_lower(HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "logging_level", false));
    auto log_file = HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "log_file", false);
    auto log_console = HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "log_console", false);
    auto log_syslog = HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "log_syslog", false);
    auto log_file_name = HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "log_file_name", false);

    // init the base logging system
    LogConfigurator::initLogging();

    if (param_to_lower(log_file) == "true")
        LogConfigurator::initFileLogging(log_file_name);

    if (param_to_lower(log_console) == "true")
        LogConfigurator::initConsoleLogging();

    if (param_to_lower(log_syslog) == "true")
        LogConfigurator::initSyslogLogging();

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
    auto connection_string = HdbppTimescaleDbImplUtils::getConfigParam(libhdb_conf, "connect_string", true);
    spdlog::info("Mandatory config parameter connect_string: {}", connection_string);

    // allocate a connection to store data with
    Conn = make_unique<pqxx_conn::DbConnection>(pqxx_conn::DbConnection::DbStoreMethod::PreparedStatement);

    // now bring up the connection
    Conn->connect(connection_string);
    spdlog::info("Started libhdbpp-timescale shared library successfully");
}

//=============================================================================
//=============================================================================
HdbppTimescaleDbImpl::~HdbppTimescaleDbImpl()
{
    if (Conn->isOpen())
        Conn->disconnect();

    LogConfigurator::shutdownLogging();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::insert_event(Tango::EventData *event_data, const HdbEventDataType &data_type)
{
    assert(event_data);
    assert(event_data->attr_value);
    spdlog::trace("Insert data event for attribute: {}", event_data->attr_name);

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

        Conn->createTx<HdbppTxDataEventError>()
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
        Conn->createTx<HdbppTxDataEvent>()
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

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::insert_events(vector<tuple<Tango::EventData*, HdbEventDataType>> events)
{

}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::insert_param_event(
    Tango::AttrConfEventData *param_event, const HdbEventDataType & /* data_type */)
{
    assert(param_event);
    spdlog::trace("Insert parameter event request for attribute: {}", param_event->attr_name);

    Conn->createTx<HdbppTxParameterEvent>()
        .withName(param_event->attr_name)
        .withEventTime(param_event->get_date())
        .withAttrInfo(*(param_event->attr_conf))
        .store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::add_attribute(
    const std::string &fqdn_attr_name, int type, int format, int write_type, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("Insert new attribute request for attribute: {}", fqdn_attr_name);

    // forgive the ugly casting, but for some reason we receive the enum values
    // already cast to ints, we cast them back to enums so they function as
    // enums again
    Conn->createTx<HdbppTxNewAttribute>()
        .withName(fqdn_attr_name)
        .withTraits(static_cast<Tango::AttrWriteType>(write_type),
            static_cast<Tango::AttrDataFormat>(format),
            static_cast<Tango::CmdArgType>(type))
        .withTtl(ttl)
        .store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::update_ttl(const std::string &fqdn_attr_name, unsigned int ttl)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("TTL event request for attribute: {}, with ttl: {}", fqdn_attr_name, ttl);

    Conn->createTx<HdbppTxUpdateTtl>().withName(fqdn_attr_name).withTtl(ttl).store();
}

//=============================================================================
//=============================================================================
void HdbppTimescaleDbImpl::insert_history_event(const std::string &fqdn_attr_name, unsigned char event)
{
    assert(!fqdn_attr_name.empty());
    spdlog::trace("History event request for attribute: {}", fqdn_attr_name);
    Conn->createTx<HdbppTxHistoryEvent>().withName(fqdn_attr_name).withEvent(event).store();
}

//=============================================================================
//=============================================================================
bool HdbppTimescaleDbImpl::supported(HdbppFeatures feature)
{
    auto supported = false;

    switch(feature)
    {
        case HdbppFeatures::TTL:
            supported = true;
            break;

        case HdbppFeatures::BATCH_INSERTS:
            supported = true;
            break;
    }

    return supported;
}

} // namespace hdbpp
