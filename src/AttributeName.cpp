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

#include <netdb.h>

using namespace std;

namespace hdbpp
{
//=============================================================================
//=============================================================================
AttributeName::AttributeName(const std::string &fqdn_attr_name) { set(fqdn_attr_name); }

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
        _tango_host_cache = getAttrTangoHost(fqdnAttributeName());

    return _tango_host_cache;
}

//=============================================================================
//=============================================================================
const string &AttributeName::tangoHostWithDomain()
{
    validate();

    if (_tango_host_with_domain_cache.empty())
        _tango_host_with_domain_cache = addDomainToTangoHost(tangoHost());

    return _tango_host_with_domain_cache;
}

//=============================================================================
//=============================================================================
const string &AttributeName::fullAttributeName()
{
    validate();

    if (_full_attribute_name_cache.empty())
        _full_attribute_name_cache = getFullAttributeName(fqdnAttributeName());

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
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    auto second_slash = full_attr_name.find('/', first_slash + 1);

    if (second_slash == string::npos)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". There is only one slash in attribute name"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    auto third_slash = full_attr_name.find('/', second_slash + 1);

    if (third_slash == string::npos)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". There are only two slashes in attribute name"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    auto last_slash = full_attr_name.rfind('/');

    if (last_slash != third_slash)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Too many slashes provided in attribute name"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    if (first_slash == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty domain"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    if (second_slash - first_slash - 1 == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty family"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    if (third_slash - second_slash - 1 == 0)
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty member"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    if (third_slash + 1 == full_attr_name.length())
    {
        string msg {"Invalid attribute name: " + full_attr_name + ". Empty name"};
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }

    _domain_cache = full_attr_name.substr(0, first_slash);
    _family_cache = full_attr_name.substr(first_slash + 1, second_slash - first_slash - 1);
    _member_cache = full_attr_name.substr(second_slash + 1, third_slash - second_slash - 1);
    _attribute_name_cache = full_attr_name.substr(third_slash + 1);
}

//=============================================================================
//=============================================================================
string AttributeName::getAttrTangoHost(const string &fqdn_attr_name)
{
    // if tango:// exists on the string, strip it off by moving the start in 8 characters
    auto start = fqdn_attr_name.find("tango://") == string::npos ? 0 : 8;
    auto end = fqdn_attr_name.find('/', start);
    return fqdn_attr_name.substr(start, end - start);
}

//=============================================================================
//=============================================================================
string AttributeName::getFullAttributeName(const string &fqdn_attr_name)
{
    // if tango:// exists on the string, strip it off by moving the start in 8 characters
    auto start = fqdn_attr_name.find("tango://") == string::npos ? 0 : 8;
    start = fqdn_attr_name.find('/', start);
    start++;
    return fqdn_attr_name.substr(start);
}

//=============================================================================
//=============================================================================
string AttributeName::addDomainToTangoHost(const string &tango_host)
{
    if (tango_host.find('.') == string::npos)
    {
        string server_name_with_domain;
        auto server_name = tango_host.substr(0, tango_host.find(':', 0));

        struct addrinfo hints = {};
        hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        struct addrinfo *result, *rp;
        const int status = getaddrinfo(server_name.c_str(), nullptr, &hints, &result);

        if (status != 0)
        {
            //LOG(LogLevel::Error) << "Error: getaddrinfo: " << gai_strerror(status) << endl;
            return tango_host;
        }

        for (rp = result; rp != nullptr; rp = rp->ai_next)
            server_name_with_domain = string(rp->ai_canonname) + tango_host.substr(tango_host.find(':', 0));

        freeaddrinfo(result); // all done with this structure
        return server_name_with_domain;
    }

    return tango_host;
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
        //LOG(LogLevel::Error) << msg << endl;
        throw invalid_argument(msg);
    }
}

//=============================================================================
//=============================================================================
void AttributeName::print(std::ostream &os) const { os << "AttributeName(_fqdn_attr_name: " << _fqdn_attr_name << ")"; }

//=============================================================================
//=============================================================================
AttributeName &AttributeName::operator=(const AttributeName &other)
{
    // clear the cache
    clear();

    // now copy the fqdn, we do not copy the cache
    _fqdn_attr_name = other._fqdn_attr_name;
}

} // namespace hdbpp
