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

#include <iostream>
#include <string>

namespace hdbpp
{
/// @class AttributeName
/// @brief Represents a fully qualified domain name (FQDN) for a device server attribute.
/// @details
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
    // TODO Tidy exceptions
    // TODO print/<< function
    // TODO logging

    /// @brief Default constructor
    AttributeName() = default;

    /// @brief Default move constructor
    AttributeName(AttributeName &&) = default;

    /// @brief Copy constructor
    /// @param attr_name AttributeName to construct from
    AttributeName(const AttributeName &attr_name) { *this = attr_name; }

    /// @brief Construct an AttributeName object
    /// @param fqdn_attr_name FQDN attribute name
    AttributeName(const std::string &fqdn_attr_name);

    /// @brief Set the contained attribute name
    /// @param fqdn_attr_name FQDN attribute name
    void set(const std::string &fqdn_attr_name);

    /// @brief Clear attribute name and any internal cached items
    void clear() noexcept;

    /// @brief Return the fully qualified attribute name the object was created with.
    /// @return FQDN attribute name
    const std::string &fqdnAttributeName() const noexcept { return _fqdn_attr_name; }

    /// @brief Return the full attribute name extracted from the fully qualified attribute name.
    /// @return Full attribute name
    /// @throw std::invalid_argument
    const std::string &fullAttributeName();

    /// @brief Return the tango host extracted from the fully qualified attribute name.
    /// @return Tango Host name
    /// @throw std::invalid_argument
    const std::string &tangoHost();

    /// @brief Return the tango host with the domain, i.e. "esrf.fr", appended
    /// @return Tango Host with domain name appended
    /// @throw std::invalid_argument
    const std::string &tangoHostWithDomain();

    /// @brief Return the domain element of the full attribute name
    /// @throw std::invalid_argument
    const std::string &domain();

    /// @brief Return the family element of the full attribute name
    /// @throw std::invalid_argument
    const std::string &family();

    /// @brief Return the member element of the full attribute name
    /// @throw std::invalid_argument
    const std::string &member();

    /// @brief Return the name element of the full attribute name
    /// @throw std::invalid_argument
    const std::string &name();

    /// @brief Return the status of the AttributeName setting.
    /// @return True if empty, False otherwise
    bool empty() const noexcept { return _fqdn_attr_name.empty(); }

    /// @brief Print the AttributeName object to the stream
    void print(std::ostream &os) const;

    /// @brief Equality operator
    /// @return True if same, False otherwise
    bool operator==(const AttributeName &other) const { return _fqdn_attr_name == other._fqdn_attr_name; }

    /// @brief Inequality operator
    /// @return True if different, False otherwise
    bool operator!=(const AttributeName &other) const { return !(_fqdn_attr_name == other._fqdn_attr_name); }

    /// @brief Copy operator
    /// @return Reference to the current AttributeName
    AttributeName &operator=(const AttributeName &other);

private:
    // extract the tango host from the fqdm attribute name, returns unknown
    // if unable to find the tango host
    std::string getAttrTangoHost(const std::string &fqdn_attr_name);

    // extract the full attribute name, i.e. domain/family/member/name
    std::string getFullAttributeName(const std::string &fqdn_attr_name);

    // takes the fqdn and breaks out the various component parts, such
    // as domain, family etc
    void setDomainFamilyMemberName(const std::string &full_attr_name);

    // combine the local domain and tango host as a string
    std::string addDomainToTangoHost(const std::string &tango_host);

    // check if the AttributeName is empty before executing a complex
    // operation, such as returning the tango host
    void validate();

    // the fully qualified domain name string
    std::string _fqdn_attr_name;

    // each string is a cache, and generated only once to save
    // on performance. Yet to measure this to see if the difference is worth
    // the complexity
    std::string _full_attribute_name_cache;
    std::string _tango_host_cache;
    std::string _tango_host_with_domain_cache;
    std::string _domain_cache;
    std::string _family_cache;
    std::string _member_cache;
    std::string _attribute_name_cache;
};

} // namespace hdbpp
#endif // _ATTRIBUTE_NAME_H
