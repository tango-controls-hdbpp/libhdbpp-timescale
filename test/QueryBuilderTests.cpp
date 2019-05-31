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

#include "QueryBuilder.hpp"
#include "TimescaleSchema.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp;
using namespace hdbpp::pqxx_conn;
using namespace Catch::Matchers;

SCENARIO("Creating valid database table names for SCALAR", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for scalar")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_SCALAR from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_SCALAR));
            }
        }
    }
}

SCENARIO("Creating valid database table names for SPECTRUM", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for spectrum")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DEV_DOUBLE};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_ARRAY from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_ARRAY));
            }
        }
    }
}

SCENARIO("Creating valid database table names for IMAGE", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for image")
    {
        AttributeTraits traits {Tango::READ, Tango::IMAGE, Tango::DEV_DOUBLE};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_IMAGE from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_IMAGE));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_BOOLEAN", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for boolean")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_BOOLEAN};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_BOOLEAN from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_BOOLEAN));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_UCHAR", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_UCHAR};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_UCHAR from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_UCHAR));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_DOUBLE", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_DOUBLE from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_DOUBLE));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_FLOAT", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_FLOAT};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_FLOAT from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_FLOAT));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_STRING", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_STRING};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_STRING from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_STRING));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_LONG", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_LONG};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_LONG from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_LONG));
            }
        }
    }
}

SCENARIO("Creating valid database table names for DEV_ULONG", "[query-string]")
{
    QueryBuilder query_builder;

    GIVEN("An Attribute traits configured for unsigned char")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_ULONG};

        WHEN("Requesting a table name for the traits")
        {
            auto result = query_builder.tableName(traits);

            THEN("The result must include the TYPE_DEV_ULONG from the schema")
            {
                REQUIRE_THAT(result, Contains(TYPE_DEV_ULONG));
            }
        }
    }
}