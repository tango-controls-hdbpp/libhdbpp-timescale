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

#ifndef _ATTRIBUTE_NAME_H
#define _ATTRIBUTE_NAME_H

#include "LibUtils.hpp"

#include <iostream>
#include <string>

namespace hdbpp_internal
{
/// Represents a FQDN for a device server attribute. The AttributeName
/// class must be primed with a valid fully qualified domain attribute name. From
/// this name the class can extract various fields for the user. Each field is cached,
/// so if it is asked for multiple times then it is only extracted once. To not
/// degrade performance, we return each cached value as a const reference, rather than value.
/// If the fully qualified domain name is not valid, then the constructor will throw
/// an exception.
class AttributeName
{
public:
    // TODO Test all exceptions

    AttributeName() = default;
    AttributeName(AttributeName &&) = default;
    ~AttributeName() = default;
    AttributeName(const AttributeName &attr_name) { *this = attr_name; }
    AttributeName(const std::string &fqdn_attr_name);

    auto fqdnAttributeName() const noexcept -> const std::string & { return _fqdn_attr_name; }
    auto fullAttributeName() -> const std::string &;

    // tango host info
    auto tangoHost() -> const std::string &;
    auto tangoHostWithDomain() -> const std::string &;

    // attribute name elements
    auto domain() -> const std::string &;
    auto family() -> const std::string &;
    auto member() -> const std::string &;
    auto name() -> const std::string &;

    // utility functions
    void set(const std::string &fqdn_attr_name);
    void clear() noexcept;
    auto empty() const noexcept -> bool { return _fqdn_attr_name.empty(); }
    void print(std::ostream &os) const;

    auto operator==(const AttributeName &other) const -> bool { return _fqdn_attr_name == other._fqdn_attr_name; }
    auto operator!=(const AttributeName &other) const -> bool { return !(_fqdn_attr_name == other._fqdn_attr_name); }
    auto operator=(const AttributeName &other) -> AttributeName &;
    auto operator=(AttributeName &&other) noexcept -> AttributeName &;

private:
    // extract the full attribute name, i.e. domain/family/member/name
    auto getFullAttributeName(const std::string &fqdn_attr_name) -> std::string;

    // takes the fqdn and breaks out the various component parts, such
    // as domain, family etc
    void setDomainFamilyMemberName(const std::string &full_attr_name);

    // combine the local domain and tango host as a std::string
    auto addDomainToTangoHost(const std::string &tango_host) -> std::string;

    // check if the AttributeName is empty before executing a complex
    // operation, such as returning the tango host
    void validate();

    // the fully qualified domain name std::string
    std::string _fqdn_attr_name;

    // each std::string is a cache, and generated only once to save
    // on performance
    std::string _full_attribute_name_cache;
    std::string _tango_host_cache;
    std::string _tango_host_with_domain_cache;
    std::string _domain_cache;
    std::string _family_cache;
    std::string _member_cache;
    std::string _attribute_name_cache;
};

} // namespace hdbpp_internal
#endif // _ATTRIBUTE_NAME_H
