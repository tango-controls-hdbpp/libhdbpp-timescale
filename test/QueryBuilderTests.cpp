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
#include "TestHelpers.hpp"
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
        QueryBuilder query_builder;
        auto value_r = make_unique<vector<double>>(1.1, 2.2);
        auto value_r_empty = make_unique<vector<double>>();
        auto value_w = make_unique<vector<double>>(3.3, 4.4);
        auto value_w_empty = make_unique<vector<double>>();

        WHEN("Requesting a query string for traits configured for Tango::READ")
        {
           AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = query_builder.storeDataEventString<double>(
                TestAttrFQDName,
                string("0"),
                string("1"),
                value_r,
                value_w_empty,
                traits);

            THEN("The result must include the DAT_COL_VALUE_R field only")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, !Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_r, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::WRITE")
        {
            AttributeTraits traits {Tango::WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = query_builder.storeDataEventString<double>(
                TestAttrFQDName,
                string("0"),
                string("1"),
                value_r_empty,
                value_w,
                traits);

            THEN("The result must include the DAT_COL_VALUE_W field only")
            {
                REQUIRE_THAT(result, !Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_w, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WRITE")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = query_builder.storeDataEventString<double>(
                TestAttrFQDName,
                string("0"),
                string("1"),
                value_r,
                value_w,
                traits);

            THEN("The result must include both the DAT_COL_VALUE_R and DAT_COL_VALUE_W field")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_r, traits)));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_w, traits)));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WITH_WRITE")
        {
            AttributeTraits traits {Tango::READ_WITH_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};

            auto result = query_builder.storeDataEventString<double>(
                TestAttrFQDName,
                string("0"),
                string("1"),
                value_r,
                value_w,
                traits);

            THEN("The result must include both the DAT_COL_VALUE_R and DAT_COL_VALUE_W field")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_r, traits)));
                REQUIRE_THAT(result, Contains(query_utils::ToString<double>::run(value_w, traits)));
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

            THEN("The result must include the DAT_COL_VALUE_R field only")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, !Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, !Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::WRITE")
        {
            AttributeTraits traits {Tango::WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include the DAT_COL_VALUE_W field only")
            {
                REQUIRE_THAT(result, !Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, !Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WRITE")
        {
            AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include both the DAT_COL_VALUE_R and DAT_COL_VALUE_W field")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, Contains("$5"));
            }
        }
        WHEN("Requesting a query string for traits configured for Tango::READ_WITH_WRITE")
        {
            AttributeTraits traits {Tango::READ_WITH_WRITE, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto result = query_builder.storeDataEventStatement<double>(traits);

            THEN("The result must include both the DAT_COL_VALUE_R and DAT_COL_VALUE_W field")
            {
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_R));
                REQUIRE_THAT(result, Contains(DAT_COL_VALUE_W));
                REQUIRE_THAT(result, Contains("$4"));
                REQUIRE_THAT(result, Contains("$5"));
            }
        }
    }
}

SCENARIO("Creating valid insert queries with storeDataEventErrorQuery()", "[query-string]")
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

    vector<string> types_str {TYPE_DEV_DOUBLE,
        TYPE_DEV_FLOAT,
        TYPE_DEV_STRING,
        TYPE_DEV_LONG,
        TYPE_DEV_ULONG,
        TYPE_DEV_LONG64,
        TYPE_DEV_ULONG64,
        TYPE_DEV_SHORT,
        TYPE_DEV_USHORT,
        TYPE_DEV_BOOLEAN,
        TYPE_DEV_UCHAR,
        TYPE_DEV_STATE,
        TYPE_DEV_ENCODED,
        TYPE_DEV_ENUM};

    vector<string> format_types_str {TYPE_SCALAR, TYPE_ARRAY, TYPE_IMAGE};

    // loop for every combination of type in Tango
    for (unsigned int t = 0; t < types.size(); ++t)
    {
        for (unsigned int f = 0; f < format_types.size(); ++f)
        {
            for (auto &write_type : write_types)
            {
                QueryBuilder query_builder;
                AttributeTraits traits {write_type, format_types[f], types[t]};

                DYNAMIC_SECTION("Testing table name for traits: " << traits)
                {
                    auto result = query_builder.tableName(traits);
                    REQUIRE_THAT(result, Contains(types_str[t]));
                    REQUIRE_THAT(result, Contains(format_types_str[f]));
                }
            }
        }
    }
}
