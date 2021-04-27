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
#include "TestHelpers.hpp"
#include "TimescaleSchema.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_internal::pqxx_conn;
using namespace hdbpp_test::attr_name;
using namespace Catch::Matchers;

SCENARIO("storeDataEventString() returns the correct Value fields for the given traits", "[query-string]")
{
    GIVEN("A query builder object with nothing cached")
    {
        auto value_r = make_unique<vector<double>>(1.1, 2.2);
        auto value_r_empty = make_unique<vector<double>>();
        auto value_w = make_unique<vector<double>>(3.3, 4.4);
        auto value_w_empty = make_unique<vector<double>>();

        WHEN("Requesting a query string for traits configured for Tango::READ")
        {
            AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r, value_w_empty, traits);

            THEN("The result must include the schema::DatColValueR field only")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, !Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_r, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::WRITE")
        {
            AttributeTraits traits {Tango::WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r_empty, value_w, traits);

            THEN("The result must include the schema::DatColValueW field only")
            {
                REQUIRE_THAT(result, !Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_w, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WRITE")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r, value_w, traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_r, traits)));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_w, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WITH_WRITE")
        {
            AttributeTraits traits {Tango::READ_WITH_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r, value_w, traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_r, traits)));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_w, traits)));
            }
        }
    }
}

SCENARIO("storeDataEventString() adds a null when value is size zero", "[query-string]")
{
    GIVEN("A query builder object with nothing cached")
    {
        auto value_r = make_unique<vector<double>>(1.1, 2.2);
        auto value_r_empty = make_unique<vector<double>>();
        auto value_w = make_unique<vector<double>>(3.3, 4.4);
        auto value_w_empty = make_unique<vector<double>>();

        WHEN("Requesting a query string with a size zero read value")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r, value_w_empty, traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_r, traits)));
                REQUIRE_THAT(result, Contains("NULL"));
            }
        }
        WHEN("Requesting a query string with a size zero write value")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = QueryBuilder::storeDataEventString<double>(
                TestAttrFQDName, string("0"), string("1"), value_r_empty, value_w, traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains(query_utils::DataToString<double>::run(value_w, traits)));
                REQUIRE_THAT(result, Contains("NULL"));
            }
        }
    }
}

SCENARIO("storeDataEventStatement() returns the correct Value fields for the given traits", "[query-string]")
{
    GIVEN("A query builder object with nothing cached")
    {
        QueryBuilder query_builder;

        WHEN("Requesting a query string for traits configured for Tango::READ")
        {
            AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include the schema::DatColValueR field only")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, !Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, !Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::WRITE")
        {
            AttributeTraits traits {Tango::WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include the schema::DatColValueW field only")
            {
                REQUIRE_THAT(result, !Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, !Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WRITE")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WITH_WRITE")
        {
            AttributeTraits traits {Tango::READ_WITH_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include both the schema::DatColValueR and schema::DatColValueW field")
            {
                REQUIRE_THAT(result, Contains(schema::DatColValueR));
                REQUIRE_THAT(result, Contains(schema::DatColValueW));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, Contains("$5"));
            }
        }
    }
}

SCENARIO("Creating valid insert queries with storeDataEventErrorString()", "[query-string]")
{
    GIVEN("An Attribute traits configured for scalar")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

        WHEN("Requesting an error query string")
        {
            auto result = QueryBuilder::storeDataEventErrorString(
                string("1"), string("0"), string("1"), string("An error message"), traits);

            THEN("The result must include the schema::DatColValueR field only")
            {
                REQUIRE_THAT(result, Contains(string("An error message")));
            }
        }
    }
}

TEST_CASE("Creating valid database table names for types", "[query-string]")
{
    vector<Tango::CmdArgType> types {Tango::DEV_DOUBLE,
        Tango::DEV_FLOAT,
        Tango::DEV_STRING,
        Tango::DEV_LONG,
        Tango::DEV_ULONG,
        Tango::DEV_LONG64,
        Tango::DEV_ULONG64,
        Tango::DEV_SHORT,
        Tango::DEV_USHORT,
        Tango::DEV_BOOLEAN,
        Tango::DEV_UCHAR,
        Tango::DEV_STATE,
        Tango::DEV_ENCODED,
        Tango::DEV_ENUM};

    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM, Tango::IMAGE};

    vector<string> types_str {schema::TypeDevDouble,
        schema::TypeDevFloat,
        schema::TypeDevString,
        schema::TypeDevLong,
        schema::TypeDevUlong,
        schema::TypeDevLong64,
        schema::TypeDevUlong64,
        schema::TypeDevShort,
        schema::TypeDevUshort,
        schema::TypeDevBoolean,
        schema::TypeDevUchar,
        schema::TypeDevState,
        schema::TypeDevEncoded,
        schema::TypeDevEnum};

    vector<string> format_types_str {schema::TypeScalar, schema::TypeArray, schema::TypeImage};

    // loop for every combination of type in Tango
    for (unsigned int t = 0; t < types.size(); ++t)
    {
        for (unsigned int f = 0; f < format_types.size(); ++f)
        {
            for (auto &write_type : write_types)
            {
                AttributeTraits traits {write_type, format_types[f], types[t]};

                DYNAMIC_SECTION("Testing table name for traits: " << traits)
                {
                    auto result = QueryBuilder::tableName(traits);
                    REQUIRE_THAT(result, Contains(types_str[t]));
                    REQUIRE_THAT(result, Contains(format_types_str[f]));
                }
            }
        }
    }
}
