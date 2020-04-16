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

#include "AttributeName.hpp"

#include "LibUtils.hpp"

#include <netdb.h>

using namespace std;

namespace hdbpp_internal
{
//=============================================================================
//=============================================================================
AttributeName::AttributeName(const std::string &fqdn_attr_name)
{
    set(fqdn_attr_name);
}

//=============================================================================
//=============================================================================
void AttributeName::set(const std::string &fqdn_attr_name)
{
    // clear the cache and set the name
    clear();
    _fqdn_attr_name = fqdn_attr_name;
}

//=============================================================================
//=============================================================================
void AttributeName::clear() noexcept
{
    _fqdn_attr_name.clear();
    _full_attribute_name_cache.clear();
    _tango_host_cache.clear();
    _tango_host_with_domain_cache.clear();
    _domain_cache.clear();
    _family_cache.clear();
    _member_cache.clear();
    _attribute_name_cache.clear();
}

//=============================================================================
//=============================================================================
const string &AttributeName::tangoHost()
{
    validate();

    if (_tango_host_cache.empty())
    {
        // if tango:// exists on the string, strip it off by moving the start in 8 characters
        auto start = _fqdn_attr_name.find("tango://") == string::npos ? 0 : 8;
        auto end = _fqdn_attr_name.find('/', start);
        _tango_host_cache = _fqdn_attr_name.substr(start, end - start);
    }

    return _tango_host_cache;
}

//=============================================================================
//=============================================================================
const string &AttributeName::tangoHostWithDomain()
{
    validate();

    if (_tango_host_with_domain_cache.empty())
    {
        string tango_host = tangoHost();

        if (tango_host.find('.') == string::npos)
        {
            string server_name_with_domain;
            auto server_name = tango_host.substr(0, tango_host.find(':', 0));

            struct addrinfo hints = {};
            hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_CANONNAME;

            struct addrinfo *result;
            const int status = getaddrinfo(server_name.c_str(), nullptr, &hints, &result);

            if (status != 0)
            {
                spdlog::error("Error: Unable to add domain to tango host: getaddrinfo failed with error: {}",
                    gai_strerror(status));

                return tangoHost();
            }
			
            if (result == nullptr)
            {
                spdlog::error("Error: Unable to add domain to tango host {}: getaddrinfo didn't return the canonical name (result == nullptr)", tango_host);
                return tangoHost();
            }
			
            if (result->ai_canonname == nullptr)
            {
                spdlog::error("Error: Unable to add domain to tango host {}: getaddrinfo didn't return the canonical name (result->ai_canonname == nullptr)", tango_host);
                freeaddrinfo(result);
                return tangoHost();
            }
            
            server_name_with_domain = string(result->ai_canonname) + tango_host.substr(tango_host.find(':', 0));

            freeaddrinfo(result); // all done with this structure
            _tango_host_with_domain_cache = server_name_with_domain;
        }
        else
        {
            _tango_host_with_domain_cache = tango_host;
        }
    }

    return _tango_host_with_domain_cache;
}

//=============================================================================
//=============================================================================
const string &AttributeName::fullAttributeName()
{
    validate();

    if (_full_attribute_name_cache.empty())
    {
        // if tango:// exists on the string, strip it off by moving the start in 8 characters
        auto start = _fqdn_attr_name.find("tango://") == string::npos ? 0 : 8;
        start = _fqdn_attr_name.find('/', start);
        start++;
        _full_attribute_name_cache = _fqdn_attr_name.substr(start);
    }

    return _full_attribute_name_cache;
}

//=============================================================================
//=============================================================================
const std::string &AttributeName::domain()
{
    validate();

    if (_domain_cache.empty())
        setDomainFamilyMemberName(fullAttributeName());

    return _domain_cache;
}

//=============================================================================
//=============================================================================
const std::string &AttributeName::family()
{
    validate();

    if (_domain_cache.empty())
        setDomainFamilyMemberName(fullAttributeName());

    return _family_cache;
}

//=============================================================================
//=============================================================================
const std::string &AttributeName::member()
{
    validate();

    if (_domain_cache.empty())
        setDomainFamilyMemberName(fullAttributeName());

    return _member_cache;
}

//=============================================================================
//=============================================================================
const std::string &AttributeName::name()
{
    validate();

    if (_domain_cache.empty())
        setDomainFamilyMemberName(fullAttributeName());

    return _attribute_name_cache;
}

//=============================================================================
//=============================================================================
void AttributeName::setDomainFamilyMemberName(const string &full_attr_name)
{
    auto first_slash = full_attr_name.find('/');

    if (first_slash == string::npos)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". There is no slash in attribute name"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    auto second_slash = full_attr_name.find('/', first_slash + 1);

    if (second_slash == string::npos)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". There is only one slash in attribute name"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    auto third_slash = full_attr_name.find('/', second_slash + 1);

    if (third_slash == string::npos)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". There are only two slashes in attribute name"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    auto last_slash = full_attr_name.rfind('/');

    if (last_slash != third_slash)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Too many slashes provided in attribute name"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    if (first_slash == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty domain"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    if (second_slash - first_slash - 1 == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty family"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    if (third_slash - second_slash - 1 == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty member"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    if (third_slash + 1 == full_attr_name.length())
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty name"};
        spdlog::error("Error: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }

    _domain_cache = full_attr_name.substr(0, first_slash);
    _family_cache = full_attr_name.substr(first_slash + 1, second_slash - first_slash - 1);
    _member_cache = full_attr_name.substr(second_slash + 1, third_slash - second_slash - 1);
    _attribute_name_cache = full_attr_name.substr(third_slash + 1);
}

//=============================================================================
//=============================================================================
void AttributeName::validate()
{
    // if the AttributeName is empty, then throw and exception, since
    // it means we just tried to execute a complex operation
    if (empty())
    {
        string msg {"AttributeName is empty."};
        spdlog::error("Failed validation for attribute: {}", msg);
        Tango::Except::throw_exception("Invalid Argument", msg, LOCATION_INFO);
    }
}

//=============================================================================
//=============================================================================
void AttributeName::print(ostream &os) const
{
    os << "AttributeName(_fqdn_attr_name: " << _fqdn_attr_name << ")";
}

//=============================================================================
//=============================================================================
AttributeName &AttributeName::operator=(const AttributeName &other)
{
    // clear the cache
    clear();

    // now copy the fqdn, we do not copy the cache
    _fqdn_attr_name = other._fqdn_attr_name;
    return *this;
}

//=============================================================================
//=============================================================================
AttributeName &AttributeName::operator=(AttributeName &&other) noexcept
{
    // clear the cache
    clear();

    // now copy the fqdn, we do not copy the cache
    _fqdn_attr_name = move(other._fqdn_attr_name);
    return *this;
}

} // namespace hdbpp_internal
