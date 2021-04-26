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

#ifndef _LIBUTILS_H
#define _LIBUTILS_H

#include <iostream>
#include <tango.h>
#include <type_traits>

// spdlog includes used by them module
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

// why is it OmniORB (via Tango)) and Pqxx define these types in different ways? Perhaps
// its the autotools used to configure them? Either way, we do not use tango, just need its
// types, so undef and allow the Pqxx defines to take precedent
#undef HAVE_UNISTD_H
#undef HAVE_SYS_TYPES_H
#undef HAVE_SYS_TIME_H
#undef HAVE_POLL

namespace hdbpp_internal
{
// generic output operator for classes
template<class T>
auto operator<<(std::ostream &os, const T &t) -> decltype(t.print(os), static_cast<void>(0), os)
{
    t.print(os);
    return os;
}

// to_string functions for tango enums
std::string tangoEnumToString(Tango::AttrWriteType write_type);
std::string tangoEnumToString(Tango::AttrDataFormat format);
std::string tangoEnumToString(Tango::CmdArgType type);
std::string tangoEnumToString(Tango::AttrQuality quality);

// some output operators for tango enums
std::ostream &operator<<(std::ostream &os, Tango::AttrWriteType write_type);
std::ostream &operator<<(std::ostream &os, Tango::AttrDataFormat format);
std::ostream &operator<<(std::ostream &os, Tango::CmdArgType type);
std::ostream &operator<<(std::ostream &os, Tango::AttrQuality quality);

struct LogConfigurator
{
    static void initLogging(const std::string &identity);
    static void initSyslogLogging(const std::string &identity);
    static void initConsoleLogging(const std::string &identity);
    static void initFileLogging(const std::string &identity, const std::string &log_file_name);

    static void shutdownLogging(const std::string &identity);
    static void setLoggingLevel(spdlog::level::level_enum level);
};

namespace logging_utils
{
    // SPDLOG config and setup
    const std::string LibLoggerName = "hdbpp";
    const std::string SyslogIdent = "hdbpp-timescale-";

    // get the file name from the __FILE__ variable for error messages
    constexpr auto *getFileName(const char *const path)
    {
        // We silence clang warnings for this funciton, this is a quick and simple
        // way to produce the file name, and yes we use pointer arithmetic, but
        // the alternatives can be messy or overworked.
        const auto *start_position = path;

        // NOLINTNEXTLINE
        for (const auto *current_character = path; *current_character != '\0'; ++current_character)
            // NOLINTNEXTLINE
            if (*current_character == '\\' || *current_character == '/')
                start_position = current_character;

        if (start_position != path)
            // NOLINTNEXTLINE
            ++start_position;

        return start_position;
    }
} // namespace logging_utils

// Macros to get the location for reporting errors
#define S1(x) #x
#define S2(x) S1(x)

#define LOCATION_INFO std::string(logging_utils::getFileName(__FILE__)) + ":" + std::string(__func__) + ":" S2(__LINE__)

}; // namespace hdbpp_internal
#endif // _LIBUTILS_H
