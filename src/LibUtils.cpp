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

#include "LibUtils.hpp"

#include "spdlog/async.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/syslog_sink.h"

namespace hdbpp_internal
{
//=============================================================================
//=============================================================================
string tangoEnumToString(Tango::AttrWriteType write_type)
{
    switch (write_type)
    {
        case Tango::READ: return "READ";
        case Tango::WRITE: return "WRITE";
        case Tango::READ_WRITE: return "READ_WRITE";
        case Tango::READ_WITH_WRITE: return "READ_WITH_WRITE";
        case Tango::WT_UNKNOWN: return "WT_UNKNOWN";
    }

    return "UNKNOWN";
}

//=============================================================================
//=============================================================================
string tangoEnumToString(Tango::AttrDataFormat format)
{
    switch (format)
    {
        case Tango::SPECTRUM: return "SPECTRUM";
        case Tango::SCALAR: return "SCALAR";
        case Tango::IMAGE: return "IMAGE";
        case Tango::FMT_UNKNOWN: return "FMT_UNKNOWN";
    }

    return "UNKNOWN";
}

//=============================================================================
//=============================================================================
string tangoEnumToString(Tango::CmdArgType type)
{
    switch (type)
    {
        case Tango::DEV_BOOLEAN: return "DEV_BOOLEAN";
        case Tango::DEV_SHORT: return "DEV_SHORT";
        case Tango::DEV_LONG: return "DEV_LONG";
        case Tango::DEV_LONG64: return "DEV_LONG64";
        case Tango::DEV_FLOAT: return "DEV_FLOAT";
        case Tango::DEV_DOUBLE: return "DEV_DOUBLE";
        case Tango::DEV_UCHAR: return "DEV_UCHAR";
        case Tango::DEV_USHORT: return "DEV_USHORT";
        case Tango::DEV_ULONG: return "DEV_ULONG";
        case Tango::DEV_ULONG64: return "DEV_ULONG64";
        case Tango::DEV_STRING: return "DEV_STRING";
        case Tango::DEV_STATE: return "DEV_STATE";
        case Tango::DEV_ENUM: return "DEV_ENUM";
        case Tango::DEV_ENCODED: return "DEV_ENCODED";
        case Tango::DATA_TYPE_UNKNOWN: return "DATA_TYPE_UNKNOWN";
    }

    return "UNKNOWN";
}

//=============================================================================
//=============================================================================
string tangoEnumToString(Tango::AttrQuality quality)
{
    switch (quality)
    {
        case Tango::ATTR_VALID: return "ATTR_VALID";
        case Tango::ATTR_INVALID: return "ATTR_INVALID";
        case Tango::ATTR_ALARM: return "ATTR_ALARM";
        case Tango::ATTR_CHANGING: return "ATTR_CHANGING";
        case Tango::ATTR_WARNING: return "ATTR_WARNING";
    }

    return "UNKNOWN";
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttrWriteType write_type)
{
    os << tangoEnumToString(write_type);
    return os;
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttrDataFormat format)
{
    os << tangoEnumToString(format);
    return os;
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::CmdArgType type)
{
    os << tangoEnumToString(type);
    return os;
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttrQuality quality)
{
    os << tangoEnumToString(quality);
    return os;
}

//=============================================================================
//=============================================================================
void LogConfigurator::initLogging()
{
    auto logger = spdlog::get(logging_utils::LibLoggerName);

    if (!logger)
    {
        try
        {
            spdlog::init_thread_pool(8192, 1);

            auto dist_sink = make_shared<spdlog::sinks::dist_sink_mt>();

            auto logger = make_shared<spdlog::async_logger>(logging_utils::LibLoggerName,
                dist_sink,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::overrun_oldest);

            // set the logger as the default so it can be accessed all over the library
            spdlog::register_logger(logger);
            spdlog::flush_every(std::chrono::seconds(1));
            spdlog::flush_on(spdlog::level::warn);
            spdlog::set_default_logger(logger);
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            string msg {"Failed to initialise the logging system, caught error: " + string(ex.what())};
            cout << msg << endl;
            Tango::Except::throw_exception("Runtime Error", msg, LOCATION_INFO);
        }
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::initSyslogLogging()
{
    try
    {
        auto logger = spdlog::get(logging_utils::LibLoggerName);
        auto &sinks_tmp = dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(*(logger->sinks().begin()))->sinks();
        sinks_tmp.push_back(make_shared<spdlog::sinks::syslog_sink_mt>(logging_utils::SyslogIdent, 0, LOG_USER, false));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        string msg {"Failed to initialise the syslog logging system, caught error: " + string(ex.what())};
        cout << msg << endl;
        Tango::Except::throw_exception("Runtime Error", msg, LOCATION_INFO);
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::initConsoleLogging()
{
    try
    {
        auto logger = spdlog::get(logging_utils::LibLoggerName);
        auto &sinks_tmp = dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(*(logger->sinks().begin()))->sinks();
        sinks_tmp.push_back(make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        string msg {"Failed to initialise the console logging system, caught error: " + string(ex.what())};
        cout << msg << endl;
        Tango::Except::throw_exception("Runtime Error", msg, LOCATION_INFO);
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::initFileLogging(const std::string &log_file_name)
{
    try
    {
        auto logger = spdlog::get(logging_utils::LibLoggerName);
        auto &sinks_tmp = dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(*(logger->sinks().begin()))->sinks();
        sinks_tmp.push_back(make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, 1024 * 1024 * 10, 3));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        string msg {"Failed to initialise the file logging system, caught error: " + string(ex.what())};
        cout << msg << endl;
        Tango::Except::throw_exception("Runtime Error", msg, LOCATION_INFO);
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::shutdownLogging()
{
    auto logger = spdlog::get(logging_utils::LibLoggerName);

    if (!logger)
    {
        spdlog::debug("Shutting down logging...");
        logger->flush();
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::setLoggingLevel(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
    spdlog::debug("Logger set to level {}", (int)level);
}

} // namespace hdbpp_internal
