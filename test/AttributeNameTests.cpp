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
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

#include <netdb.h>

using namespace std;
using namespace hdbpp;
using namespace hdbpp_test::attr_name;

SCENARIO("AttributeName supports fully qualified attribute name", "[attribute-name]")
{
    GIVEN("Attribute with fqdn")
    {
        AttributeName attribute_name {TestAttrFQDName};

        WHEN("Fully qualified domain name requested")
        {
            THEN("Valid fqdn returned") { REQUIRE(attribute_name.fqdnAttributeName() == TestAttrFQDName); }
        }
        WHEN("Full attribute name is requested")
        {
            THEN("Valid full attribute name returned") { REQUIRE(attribute_name.fullAttributeName() == TestAttrFullAttrName); }
        }
        WHEN("Tango host is requested")
        {
            THEN("Valid tango host is returned") { REQUIRE(attribute_name.tangoHost() == TestAttrTangoHost); }
        }
        WHEN("Tango host with domain is requested")
        {
            THEN("Valid tango host with domain is returned")
            {
                REQUIRE(attribute_name.tangoHostWithDomain() == TestAttrTangoHostWithDomain);
            }
        }
        WHEN("Domain is requested")
        {
            THEN("Valid domain is returned") { REQUIRE(attribute_name.domain() == TestAttrDomain); }
        }
        WHEN("Family is requested")
        {
            THEN("Valid family is returned") { REQUIRE(attribute_name.family() == TestAttrFamily); }
        }
        WHEN("Member is requested")
        {
            THEN("Valid member is returned") { REQUIRE(attribute_name.member() == TestAttrMember); }
        }
        WHEN("Name is requested")
        {
            THEN("Valid name is returned") { REQUIRE(attribute_name.name() == TestAttrName); }
        }
    }
}

SCENARIO("AttributeName supports fully qualified attribute name missing tango prefix", "[attribute-name]")
{
    GIVEN("Attribute name with no tango:// qualifier prefixed")
    {
        AttributeName attribute_name {TestAttrFQDNameNoTangoQual};

        WHEN("Fully qualified domain name requested")
        {
            THEN("Valid fqdn is returned") { REQUIRE(attribute_name.fqdnAttributeName() == TestAttrFQDNameNoTangoQual); }
        }
        WHEN("Full attribute name is requested")
        {
            THEN("Valid full attribute name returned") { REQUIRE(attribute_name.fullAttributeName() == TestAttrFullAttrName); }
        }
        WHEN("Tango host is requested")
        {
            THEN("Valid tango host is returned") { REQUIRE(attribute_name.tangoHost() == TestAttrTangoHost); }
        }
        WHEN("Tango host with domain is requested")
        {
            THEN("Valid tango host with domain is returned")
            {
                REQUIRE(attribute_name.tangoHostWithDomain() == TestAttrTangoHostWithDomain);
            }
        }
        WHEN("Domain is requested")
        {
            THEN("Valid domain is returned") { REQUIRE(attribute_name.domain() == TestAttrDomain); }
        }
        WHEN("Family is requested")
        {
            THEN("Valid family is returned") { REQUIRE(attribute_name.family() == TestAttrFamily); }
        }
        WHEN("Member is requested")
        {
            THEN("Valid member is returned") { REQUIRE(attribute_name.member() == TestAttrMember); }
        }
        WHEN("Name is requested")
        {
            THEN("Valid name is returned") { REQUIRE(attribute_name.name() == TestAttrName); }
        }
    }
}

SCENARIO("AttributeName supports fully qualified attribute name but no network domain", "[attribute-name]")
{
    GIVEN("Attribute with valid fqdn")
    {
        AttributeName attribute_name {TestAttrFQDNameNoDomain};

        WHEN("Fully qualified domain name requested")
        {
            THEN("Valid fqdn is returned") { REQUIRE(attribute_name.fqdnAttributeName() == TestAttrFQDNameNoDomain); }
        }
        WHEN("Full attribute name is requested")
        {
            THEN("Valid full attribute name returned") { REQUIRE(attribute_name.fullAttributeName() == TestAttrFullAttrName); }
        }
        WHEN("Tango host is requested")
        {
            THEN("Value is localhost:10000") { REQUIRE(attribute_name.tangoHost() == "localhost:10000"); }
        }
        WHEN("Tango host with domain is requested")
        {
            string server_name_with_domain;
            auto server_name = attribute_name.tangoHost().substr(0, attribute_name.tangoHost().find(':', 0));
            struct addrinfo hints = {};

            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_CANONNAME;

            struct addrinfo *result, *rp;
            const int status = getaddrinfo(server_name.c_str(), NULL, &hints, &result);

            REQUIRE(status == 0);

            for (rp = result; rp != NULL; rp = rp->ai_next)
                server_name_with_domain = string(rp->ai_canonname) +
                    attribute_name.tangoHost().substr(attribute_name.tangoHost().find(':', 0));

            freeaddrinfo(result);

            THEN("The local servers domain is added to localhost:10000 to be: " + server_name_with_domain)
            {
                REQUIRE(attribute_name.tangoHostWithDomain() == server_name_with_domain);
            }
        }
    }
}

SCENARIO("AttributeName creation and clear patterns result in valid or empty AttributeNames", "[attribute-name]")
{
    GIVEN("AttributeNames created via constructor and set() method")
    {
        AttributeName attribute_name1 {TestAttrFQDName};
        AttributeName attribute_name2;

        attribute_name2.set(TestAttrFQDName);

        WHEN("Comparing the AttributeNames")
        {
            THEN("Then the result is True")
            {
                REQUIRE(attribute_name1.empty() == false);
                REQUIRE(attribute_name2.empty() == false);
                REQUIRE(attribute_name1 == attribute_name2);
            }
            AND_WHEN("Changing the one AttributeNames contents")
            {
                attribute_name1.set("tango://localhost:10000/test-domain/test-family/test-member/test");

                THEN("Comparing the two now results in False")
                {
                    REQUIRE(attribute_name1.empty() == false);
                    REQUIRE(attribute_name2.empty() == false);
                    REQUIRE(attribute_name1 != attribute_name2);
                }
            }
        }
        WHEN("Creating a third attribute via copy constructor")
        {
            AttributeName attribute_name3(attribute_name1);

            THEN("Then the new attribute is equal to the copy constructor value")
            {
                REQUIRE(attribute_name1.empty() == false);
                REQUIRE(attribute_name2.empty() == false);
                REQUIRE(attribute_name1 == attribute_name3);
            }
        }
    }
}

SCENARIO("AttributeName can be assigned to another AttributeName", "[attribute-name]")
{
    GIVEN("AttributeName with a valid fqdn")
    {
        AttributeName attribute_name1 {TestAttrFQDName};

        WHEN("Assigning the AttributeName to another")
        {
            AttributeName attribute_name2;

            REQUIRE_NOTHROW(attribute_name2 = attribute_name1);

            THEN("The result is the same as the original") { REQUIRE(attribute_name1 == attribute_name2); }
        }
    }
}

SCENARIO("Clearing an AttributeName results in an empty AttributeName", "[attribute-name]")
{
    GIVEN("AttributeName with a valid fqdn")
    {
        AttributeName attribute_name {TestAttrFQDName};

        WHEN("Clearing the AttributeName")
        {
            REQUIRE_NOTHROW(attribute_name.clear());

            THEN("The result is empty") { REQUIRE(attribute_name.empty()); }
            AND_WHEN("Setting a new fqdn")
            {
                REQUIRE_NOTHROW(attribute_name.set(TestAttrFQDName));

                THEN("Then the result is no longer empty") { REQUIRE(attribute_name.empty() == false); }
            }
        }
    }
}

SCENARIO("Unconfigured AttributeName objects throw errors", "[attribute-name]")
{
    GIVEN("AttributeName with no fqdn")
    {
        AttributeName attribute_name;

        WHEN("Calling member fullAttributeName")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.fullAttributeName()); }
        }
        WHEN("Calling member tangoHost")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.tangoHost()); }
        }
        WHEN("Calling member tangoHostWithDomain")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.tangoHostWithDomain()); }
        }
        WHEN("Calling member domain")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.domain()); }
        }
        WHEN("Calling member family")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.family()); }
        }
        WHEN("Calling member member")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.member()); }
        }
        WHEN("Calling member name")
        {
            THEN("Then an exception is thrown") { REQUIRE_THROWS(attribute_name.name()); }
        }
    }
}
