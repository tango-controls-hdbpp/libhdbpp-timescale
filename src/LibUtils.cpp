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
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/syslog_sink.h"

namespace hdbpp
{
//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttrWriteType write_type)
{
    switch (write_type)
    {
        case Tango::READ: os << "READ"; return os;
        case Tango::WRITE: os << "WRITE"; return os;
        case Tango::READ_WRITE: os << "READ_WRITE"; return os;
        case Tango::READ_WITH_WRITE: os << "READ_WITH_WRITE"; return os;
    }

    os << "UNKNOWN";
    return os;
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttrDataFormat format)
{
    switch (format)
    {
        case Tango::SPECTRUM: os << "SPECTRUM"; return os;
        case Tango::SCALAR: os << "SCALAR"; return os;
        case Tango::IMAGE: os << "IMAGE"; return os;
    }

    os << "UNKNOWN";
    return os;
}

//=============================================================================
//=============================================================================
ostream &operator<<(ostream &os, Tango::AttributeDataType type)
{
    switch (type)
    {
        case Tango::DEV_BOOLEAN: os << "DEV_BOOLEAN"; return os;
        case Tango::DEV_SHORT: os << "DEV_SHORT"; return os;
        case Tango::DEV_LONG: os << "DEV_LONG"; return os;
        case Tango::DEV_LONG64: os << "DEV_LONG64"; return os;
        case Tango::DEV_FLOAT: os << "DEV_FLOAT"; return os;
        case Tango::DEV_DOUBLE: os << "DEV_DOUBLE"; return os;
        case Tango::DEV_UCHAR: os << "DEV_UCHAR"; return os;
        case Tango::DEV_USHORT: os << "DEV_USHORT"; return os;
        case Tango::DEV_ULONG: os << "DEV_ULONG"; return os;
        case Tango::DEV_ULONG64: os << "DEV_ULONG64"; return os;
        case Tango::DEV_STRING: os << "DEV_STRING"; return os;
        case Tango::DEV_STATE: os << "DEV_STATE"; return os;
        case Tango::DEV_ENUM: os << "DEV_ENUM"; return os;
        case Tango::DEV_ENCODED: os << "DEV_ENCODED"; return os;
    }

    os << "UNKNOWN";
    return os;
}

const string DefaultLogName = "log.txt";
const int LoggerThreadCount = 1;

//=============================================================================
//=============================================================================
void LogConfigurator::initLogging(bool enable_file, bool enable_console)
{
    try
    {
        spdlog::init_thread_pool(8192, LoggerThreadCount);

        vector<spdlog::sink_ptr> sinks;

        if (enable_file)
            sinks.push_back(make_shared<spdlog::sinks::rotating_file_sink_mt>(DefaultLogName, 1024 * 1024 * 10, 3));

        if (enable_console)
            sinks.push_back(make_shared<spdlog::sinks::stdout_color_sink_mt>());

        //sinks.push_back(make_shared<spdlog::sinks::syslog_sink_mt>("my_ident");

        if (sinks.empty())
            sinks.push_back(make_shared<spdlog::sinks::null_sink_mt>());

        auto logger = make_shared<spdlog::async_logger>(LibLoggerName,
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::overrun_oldest);

#if defined(PRODUCTION_BUILD)
        logger->set_level(spdlog::level::warn);
#else
        logger->set_level(spdlog::level::trace);
#endif

        spdlog::register_logger(logger);
        spdlog::flush_every(std::chrono::seconds(1));
        spdlog::set_default_logger(logger);

        logger->debug("Initialised the logging system...");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        // TODO kinda fatal?
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::shutdownLogging()
{
    auto logger = spdlog::get(LibLoggerName);

    if (!logger)
    {
        logger->debug("Shutting down logging...");
        logger->flush();
    }
}

//=============================================================================
//=============================================================================
void LogConfigurator::setLoggingLevel(spdlog::level::level_enum level)
{
    auto logger = spdlog::get(LibLoggerName);

    if (!logger)
    {
        logger->set_level(level);
        logger->debug("Logger set to level {}", (int)level);
    }
}

} // namespace hdbpp
