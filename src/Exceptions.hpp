
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

#ifndef _HDBPP_EXCEPTIONS_HPP
#define _HDBPP_EXCEPTIONS_HPP

#include <stdexcept>

namespace hdbpp
{
// These exceptions exist mainly to transmit errors to the transaction classes
// without using those from the library in use in the Connection class.
// Currently this would be pqxx exceptions. If we want to allow a swapping
// of the Connection implementation in future, then its types must not
// perculate up into the transaction layer. Further, using our own exceptions
// allows us to simplify the problem for the user front end, yet dump a much
// more complex diagnostic to a log file.

// TODO maybe add some see log text to the errors?

class connection_error : public std::runtime_error
{
public:
    explicit connection_error(const std::string &what_arg) : std::runtime_error(what_arg + " See log for details.") {}
};

class storage_error : public std::runtime_error
{
public:
    explicit storage_error(const std::string &what_arg) : std::runtime_error(what_arg + " See log for details.") {}
};

class consistency_error : public std::runtime_error
{
public:
    explicit consistency_error(const std::string &what_arg) : std::runtime_error(what_arg + " See log for details.") {}
};

class unknown_storage_error : public std::runtime_error
{
public:
    explicit unknown_storage_error(const std::string &what_arg) : std::runtime_error(what_arg + " See log for details.")
    {}
};

} // namespace hdbpp
#endif // _HDBPP_EXCEPTIONS_HPP