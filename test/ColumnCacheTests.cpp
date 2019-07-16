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

#include "ColumnCache.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

#include <pqxx/pqxx>
#include <string>

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_internal::pqxx_conn;
using namespace hdbpp_test::psql_conn_test;

namespace value_cache_test
{
const string IdCol = "IdCol";
const string ReferenceCol = "ReferenceCol";
const string TableName = "id_columncache_test";
const string Ref1 = "Reference string";
const string Ref2 = "Some more reference \"strings\", and more";
const string Ref3 = "A cat likes to eat 'sausages'";

const string NewValue1 = "Newvalue1";
const string NewValue2 = "Newvalue2";
const int NewValue1Id = 11;
const int NewValue2Id = 12;

void createColumnCacheTestDb(pqxx::work &tx)
{
    tx.exec("CREATE TEMP TABLE " + TableName + " (" + IdCol + " serial, " + ReferenceCol + " text, " + "PRIMARY KEY (" +
        IdCol + ")) ON COMMIT PRESERVE ROWS;");

    tx.exec("INSERT INTO " + TableName + "(" + ReferenceCol + ") VALUES (" + tx.quote(Ref1) + ");");
    tx.exec("INSERT INTO " + TableName + "(" + ReferenceCol + ") VALUES (" + tx.quote(Ref2) + ");");
    tx.exec("INSERT INTO " + TableName + "(" + ReferenceCol + ") VALUES (" + tx.quote(Ref3) + ");");
}

shared_ptr<pqxx::connection> connectDb()
{
    shared_ptr<pqxx::connection> conn = nullptr;

    REQUIRE_NOTHROW(conn = make_shared<pqxx::connection>(postgres_db::ConnectionString));
    REQUIRE(conn->is_open());

    {
        pqxx::work tx {*conn};
        REQUIRE_NOTHROW(createColumnCacheTestDb(tx));
        REQUIRE_NOTHROW(tx.commit());
    }

    return conn;
}
}; // namespace value_cache_test

using namespace value_cache_test;

SCENARIO("ColumnCache can access and retrieve data from the database", "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("An empty ColumnCache")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);

        WHEN("Requesting the ColumnCache fetch all values")
        {
            REQUIRE_NOTHROW(cache.fetchAll());

            THEN("Data is fetched and cache size 3") { REQUIRE(cache.size() == 3); }
            AND_WHEN("Request to fetch data again")
            {
                REQUIRE_NOTHROW(cache.fetchAll());

                THEN("The size is still 3") { REQUIRE(cache.size() == 3); }
            }
        }
        WHEN("Requesting a value that is not cached")
        {
            REQUIRE_NOTHROW(cache.value(Ref1));

            THEN("The ColumnCache can access the data in the database and cache the value, growing its size to 1")
            {
                REQUIRE(cache.valueExists(Ref1) == true);
                REQUIRE(cache.size() == 1);
            }
        }
    }

    conn->disconnect();
}

SCENARIO("ColumnCache can grown in size when caching", "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("An empty ColumnCache")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);

        WHEN("Requesting a value")
        {
            REQUIRE_NOTHROW(cache.value(Ref1));

            THEN("The ColumnCache size grows by 1 to 1") { REQUIRE(cache.size() == 1); }
            AND_WHEN("Requesting a second unique value")
            {
                REQUIRE_NOTHROW(cache.value(Ref2));

                THEN("The size grows by 1 again to 2") { REQUIRE(cache.size() == 2); }
                AND_WHEN("Requesting a third unique value")
                {
                    REQUIRE_NOTHROW(cache.value(Ref3));

                    THEN("The size grows by 1 again to 3") { REQUIRE(cache.size() == 3); }
                }
            }
        }
        WHEN("Requesting a value")
        {
            REQUIRE_NOTHROW(cache.value(Ref1));

            THEN("The ColumnCache size grows by 1 to 1") { REQUIRE(cache.size() == 1); }
            AND_WHEN("Requesting the same value again")
            {
                REQUIRE_NOTHROW(cache.value(Ref1));

                THEN("The size does not change") { REQUIRE(cache.size() == 1); }
            }
        }
        WHEN("Caching a value")
        {
            REQUIRE_NOTHROW(cache.cacheValue(NewValue1Id, NewValue1));

            THEN("The ColumnCache size grows by 1 to 1") { REQUIRE(cache.size() == 1); }
            AND_WHEN("Caching a second value again")
            {
                REQUIRE_NOTHROW(cache.cacheValue(NewValue2Id, NewValue2));

                THEN("The size grows by 1 to 2") { REQUIRE(cache.size() == 2); }
                AND_WHEN("Asking for the values back")
                {
                    THEN("Both new values can be returned")
                    {
                        REQUIRE(cache.value(NewValue1) == NewValue1Id);
                        REQUIRE(cache.value(NewValue2) == NewValue2Id);
                    }
                }
            }
        }
    }

    conn->disconnect();
}

SCENARIO("ColumnCache will handle the same value being added twice", "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("An empty ColumnCache")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);

        WHEN("Caching a value")
        {
            REQUIRE_NOTHROW(cache.cacheValue(1, Ref1));

            THEN("The ColumnCache size grows by 1 to 1") { REQUIRE(cache.size() == 1); }
            AND_WHEN("Caching the same value again")
            {
                THEN("Then an no exception is thrown") { REQUIRE_NOTHROW(cache.cacheValue(1, Ref1)); }
            }
        }
        WHEN("Loading the database and trying to cache an existing value")
        {
            REQUIRE_NOTHROW(cache.fetchAll());
            REQUIRE(cache.size() == 3);

            THEN("Then an no exception is thrown") { REQUIRE_NOTHROW(cache.cacheValue(1, Ref1)); }
        }
    }

    conn->disconnect();
}

SCENARIO("It is an error to request invalid values", "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("A ColumnCache with loaded values")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);
        REQUIRE_NOTHROW(cache.fetchAll());
        REQUIRE(cache.size() == 3);

        WHEN("Requesting the ColumnCache fetch a value this is not cached or in the database")
        {
            THEN("An exception is thrown") { REQUIRE_THROWS(cache.value("Invalid")); }
        }
    }

    conn->disconnect();
}

SCENARIO("Clearing the cache does not stop entries being cached again", "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("An empty ColumnCache")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);

        WHEN("Requesting the ColumnCache fetch all value/reference values")
        {
            REQUIRE_NOTHROW(cache.fetchAll());

            THEN("Data is fetched and cache size 3") { REQUIRE(cache.size() == 3); }
            AND_WHEN("The cache is cleared")
            {
                cache.clear();

                THEN("The size is 0") { REQUIRE(cache.size() == 0); }
                AND_WHEN("A Db value is added back to the cache")
                {
                    REQUIRE_NOTHROW(cache.cacheValue(1, Ref1));

                    THEN("The ColumnCache grows by 1 to size 1") { REQUIRE(cache.size() == 1); }
                }
            }
        }
    }

    conn->disconnect();
}

SCENARIO("ColumnCache will fetch values from db and cache them as they are requested by reference",
    "[db-access][column-cache][psql]")
{
    auto conn = connectDb();

    GIVEN("An empty ColumnCache")
    {
        ColumnCache<int, string> cache(conn, TableName, IdCol, ReferenceCol);
        REQUIRE(cache.size() == 0);

        WHEN("Requesting a value")
        {
            REQUIRE_NOTHROW(cache.value(Ref1));

            THEN("Data is fetched and the value exists") { REQUIRE(cache.valueExists(Ref1) == true); }
            AND_WHEN("Requesting a second value")
            {
                REQUIRE_NOTHROW(cache.value(Ref1));

                THEN("Data is fetched and the value exists") { REQUIRE(cache.valueExists(Ref2) == true); }
            }
        }
    }

    conn->disconnect();
}