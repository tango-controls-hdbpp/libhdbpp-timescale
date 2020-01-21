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

#include "ConnectionBase.hpp"
#include "HdbppTxFactory.hpp"
#include "HdbppTxUpdateTtl.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_test::attr_name;

namespace hdbpp_update_ttl_test
{
// Mock connection to test the HdbppTxUpdateTtl class
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string & /* connect_str */) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept override { return _conn_state; }
    bool isClosed() const noexcept override { return !isOpen(); }

    // mock storeAttribute just records parameters
    void storeAttributeTtl(const string &full_attr_name, unsigned int ttl)
    {
        if (store_ttl_triggers_ex)
            throw runtime_error("A test exception");

        att_name = full_attr_name;
        att_ttl = ttl;
    }

    bool fetchAttributeArchived(const string & /* unused */) { return att_archived; }

    // expose the results of the store function so they can be checked
    // in the results

    // storeTtl results
    string att_name;
    unsigned int att_ttl = 0;
    bool store_ttl_triggers_ex = false;
    bool att_archived = true;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_update_ttl_test

SCENARIO("Construct and store HdbppTxUpdateTtl data without error", "[hdbpp-tx][hdbpp-tx-update-ttl]")
{
    hdbpp_update_ttl_test::MockConnection conn;
    auto ttl = 10;

    GIVEN("An HdbppTxUpdateTtl object with no data set")
    {
        auto tx = conn.createTx<HdbppTxUpdateTtl>();

        WHEN("Passing a valid configuration with method chaining")
        {
            tx.withName(TestAttrFQDName).withTtl(ttl);

            THEN("Then storing the HdbppTxUpdateTtl object does not raise an exception")
            {
                REQUIRE_NOTHROW(tx.store());
                REQUIRE(tx.result());
            }
            AND_WHEN("The result of the store is examined after storing")
            {
                REQUIRE_NOTHROW(tx.store());
                REQUIRE(tx.result());

                THEN("The data is the same as that passed via method chaining")
                {
                    REQUIRE(conn.att_name == TestAttrFinalName);
                    REQUIRE(conn.att_ttl == ttl);
                }
            }
        }
    }
}

SCENARIO(
    "When attempting to store invalid HdbppTxUpdateTtl states, errors are thrown", "[hdbpp-tx][hdbpp-tx-update-ttl]")
{
    hdbpp_update_ttl_test::MockConnection conn;
    auto ttl = 10;

    GIVEN("An HdbppTxUpdateTtl object with no data set")
    {
        auto tx = conn.createTx<HdbppTxUpdateTtl>();

        WHEN("Attempting to store without setting data")
        {
            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
        WHEN("Attempting to store without the attribute existing first")
        {
            conn.att_archived = false;
            tx.withName(TestAttrFQDName).withTtl(ttl);

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
        WHEN("Attempting to store with valid data, but disconnected connection")
        {
            conn.disconnect();
            REQUIRE(conn.isClosed());

            tx.withName(TestAttrFQDName).withTtl(ttl);

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
    }
}

SCENARIO("HdbppTxUpdateTtl Simulated exception received", "[hdbpp-tx][hdbpp-tx-update-ttl]")
{
    hdbpp_update_ttl_test::MockConnection conn;

    GIVEN("An HdbppTxUpdateTtl object with name and traits set")
    {
        auto tx = conn.createTx<HdbppTxUpdateTtl>().withName(TestAttrFQDName).withTtl(10);

        WHEN("Storing the HdbppTxUpdateTtl object with a triggered exception set")
        {
            conn.store_ttl_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}
