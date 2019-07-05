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

#ifndef _PQXX_EXTENSION_HPP
#define _PQXX_EXTENSION_HPP

#include <algorithm>
#include <iostream>
#include <pqxx/pqxx>
#include <pqxx/strconv>
#include <vector>

namespace pqxx
{
namespace internal
{
    template<>
    struct type_name<uint8_t>
    {
        static constexpr const char *value = "uint8_t";
    };

    template<>
    struct type_name<std::vector<double>>
    {
        static constexpr const char *value = "vector<double>";
    };

    template<>
    struct type_name<std::vector<float>>
    {
        static constexpr const char *value = "vector<float>";
    };

    template<>
    struct type_name<std::vector<int32_t>>
    {
        static constexpr const char *value = "vector<int32_t>";
    };

    template<>
    struct type_name<std::vector<uint32_t>>
    {
        static constexpr const char *value = "vector<uint32_t>";
    };

    template<>
    struct type_name<std::vector<int64_t>>
    {
        static constexpr const char *value = "vector<int64_t>";
    };

    template<>
    struct type_name<std::vector<uint64_t>>
    {
        static constexpr const char *value = "vector<uint64_t>";
    };

    template<>
    struct type_name<std::vector<int16_t>>
    {
        static constexpr const char *value = "vector<int16_t>";
    };

    template<>
    struct type_name<std::vector<uint16_t>>
    {
        static constexpr const char *value = "vector<uint16_t>";
    };

    template<>
    struct type_name<std::vector<uint8_t>>
    {
        static constexpr const char *value = "vector<uint8_t>";
    };

    template<>
    struct type_name<std::vector<bool>>
    {
        static constexpr const char *value = "vector<bool>";
    };

    template<>
    struct type_name<std::vector<std::string>>
    {
        static constexpr const char *value = "vector<std::string>";
    };
} // namespace internal

// this is an extension to the pqxx strconv types to allow our engine to
// handle vectors. This allows us to convert the value from Tango directly
// into a postgres array string ready for storage
template<typename T>
struct string_traits<std::vector<T>>
{
public:
    static constexpr const char *name() noexcept { return internal::type_name<T>::value; }
    static constexpr bool has_null() noexcept { return false; }
    static bool is_null(const std::vector<T> &) { return false; }
    [[noreturn]] static std::vector<T> null() { internal::throw_null_conversion(name()); }

    static void from_string(const char str[], std::vector<T> &value)
    {
        if (str == nullptr)
            internal::throw_null_conversion(name());

        if (str[0] != '{' || str[strlen(str) - 1] != '}')
            throw pqxx::conversion_error("Invalid array format");

        value.clear();

        // not the best solution right now, but we are using this for testing only
        // currently. Copy the str into a std::string so we can work with it more easily.
        std::string in(str + 1, str + (strlen(str) - 1));

        // count commas and add one to deduce elements in the string,
        // note we reduce string size to remove the brace at each end
        auto items = std::count(in.begin(), in.end(), ',');
        value.clear();
        //value.reserve(items);

        // preallocate all the items in the vector, we can then
        // simply set each in turn
        value.resize(items + 1);

        auto element = 0;
        std::string::size_type comma = 0;

        // loop and copy out each value from between the separators
        while ((comma = in.find_first_not_of(',', comma)) != std::string::npos)
        {
            auto next_comma = in.find_first_of(',', comma);
            string_traits<T>::from_string(in.substr(comma, next_comma - comma).c_str(), value[element++]);
            comma = next_comma;
        }
    }

    static std::string to_string(const std::vector<T> &value)
    {
        if (value.empty())
            return {};

        // simply use the pqxx utilities for this, rather than reinvent the wheel
        return "{" + separated_list(",", value.begin(), value.end()) + "}";
    }
};

// This specialisation is for string types to ensure the string is quoted
// for storage
template<>
struct string_traits<std::vector<std::string>>
{
public:
    static constexpr const char *name() noexcept { return "vector<string>"; }
    static constexpr bool has_null() noexcept { return false; }
    static bool is_null(const std::vector<std::string> &) { return false; }
    [[noreturn]] static std::vector<std::string> null() { internal::throw_null_conversion(name()); }

    static void from_string(const char str[], std::vector<std::string> &value)
    {
        if (str == nullptr)
            internal::throw_null_conversion(name());

        if (str[0] != '{' || str[strlen(str) - 1] != '}')
            throw pqxx::conversion_error("Invalid array format");

        value.clear();

        // not the best solution right now, but we are using this for testing only
        // currently. Copy the str into a std::string so we can work with it more easily.
        std::string in(str + 1, str + (strlen(str) - 1));
        std::string sep {"','"};
        size_t last_position = 1;
        size_t position = in.find(sep, 0);

        // as mention above, this could probably be sped up, i.e. preallocate
        // the vector etc, but for now we just need it for testing, so improvements
        // are left till later
        while (position != std::string::npos)
        {
            value.push_back(in.substr(last_position, position - last_position));
            last_position = position + 3;
            position = in.find(sep, position + 1);
        }
    }

    static std::string to_string(const std::vector<std::string> &value)
    {
        // simply use the pqxx utilities for this, rather than reinvent the wheel
        return "{" + separated_list(",", value.begin(), value.end()) + "}";
    }
};

// This specialisation is for bool, since it is not a normal container class, but
// rather some kind of bitfield. We have to adjust the from_string to take into
// account we can not use container element access
template<>
struct string_traits<std::vector<bool>>
{
public:
    static constexpr const char *name() noexcept { return "std::vector<bool>"; }
    static constexpr bool has_null() noexcept { return false; }
    static bool is_null(const std::vector<bool> &) { return false; }
    [[noreturn]] static std::vector<bool> null() { internal::throw_null_conversion(name()); }

    static void from_string(const char str[], std::vector<bool> &value)
    {
        if (str == nullptr)
            internal::throw_null_conversion(name());

        if (str[0] != '{' || str[strlen(str) - 1] != '}')
            throw pqxx::conversion_error("Invalid array format");

        value.clear();

        // not the best solution right now, but we are using this for
        // testing only. Copy the str into a std::string so we can work
        // with it more easily.
        std::string in(str + 1, str + (strlen(str) - 1));
        std::string::size_type comma = 0;

        // loop and copy out each value from between the separators
        while ((comma = in.find_first_not_of(',', comma)) != std::string::npos)
        {
            auto next_comma = in.find_first_of(',', comma);

            // we can not pass an element of the vector, since vector<bool> is not
            // in fact a container, but some kind of bit field. In this case, we
            // have to create a local variable to read the value into, then push this
            // back onto the vector
            bool field;
            string_traits<bool>::from_string(in.substr(comma, next_comma - comma).c_str(), field);
            value.push_back(field);

            comma = next_comma;
        }
    }

    static std::string to_string(const std::vector<bool> &value)
    {
        if (value.empty())
            return {};

        // simply use the pqxx utilities for this, rather than reinvent the wheel
        return "{" + separated_list(",", value.begin(), value.end()) + "}";
    }
};

// Specialization for unsigned char, which was not included in pqxx,
// this becomes an int16_t in the database
template<>
struct string_traits<uint8_t> : internal::builtin_traits<uint8_t>
{};

// Specialization for Tango::DevState char, which was not included in pqxx,
// this becomes an int16_t in the database
template<>
struct string_traits<Tango::DevState> : internal::builtin_traits<Tango::DevState>
{};
} // namespace pqxx
#endif // _PQXX_EXTENSION_HPP
