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
#include "HdbppTxNewAttribute.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp;
using namespace hdbpp_test::attr_name;

namespace hdbpp_new_attr_test
{
// Mock connection to test the HdbppTxNewAttribute class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string & /* connect_str */) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept override { return _conn_state; }
    bool isClosed() const noexcept override { return !isOpen(); }

    // mock storeAttribute just records parameters
    void storeAttribute(const string &full_attr_name,
        const string &control_system,
        const string &att_domain,
        const string &att_family,
        const string &att_member,
        const string &att_name,
        const AttributeTraits &traits)
    {
        if (store_attribute_triggers_ex)
            throw runtime_error("A test exception");

        new_att_full_attr_name = full_attr_name;
        new_att_control_system = control_system;
        new_att_domain = att_domain;
        new_att_family = att_family;
        new_att_member = att_member;
        new_att_name = att_name;
        att_traits = traits;
    }

    // expose the results of the store function so they can be checked
    // in the results

    // storeAttribute results
    string new_att_full_attr_name;
    string new_att_control_system;
    string new_att_domain;
    string new_att_family;
    string new_att_member;
    string new_att_name;
    AttributeTraits att_traits;
    bool store_attribute_triggers_ex = false;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_new_attr_test

SCENARIO("Construct and store HdbppTxNewAttribute data without error", "[hdbpp-tx][hdbpp-tx-new-attribute]")
{
    hdbpp_new_attr_test::MockConnection conn;

    GIVEN("An HdbppTxNewAttribute object with no data set")
    {
        auto tx = conn.createTx<HdbppTxNewAttribute>();

        WHEN("Passing a valid configuration with method chaining")
        {
            tx.withName(TestAttrFQDName).withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);

            THEN("Then storing the transaction does not raise an exception")
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
                    REQUIRE(conn.new_att_full_attr_name == TestAttrFinalName);
                    REQUIRE(conn.new_att_domain == TestAttrDomain);
                    REQUIRE(conn.new_att_family == TestAttrFamily);
                    REQUIRE(conn.new_att_member == TestAttrMember);
                    REQUIRE(conn.new_att_name == TestAttrName);
                    REQUIRE(conn.att_traits == AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE));
                }
            }
        }
    }
}

SCENARIO("When attempting to store invalid HdbppTxNewAttribute states, errors are thrown",
    "[hdbpp-tx][hdbpp-tx-new-attribute]")
{
    hdbpp_new_attr_test::MockConnection conn;

    GIVEN("An HdbppTxNewAttribute object with no data set")
    {
        auto tx = conn.createTx<HdbppTxNewAttribute>();

        WHEN("Attempting to store without setting data")
        {
            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
        WHEN("Attempting to store with just name set")
        {
            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.withName(TestAttrFQDName).store());
                REQUIRE(!tx.result());
            }
        }
        WHEN("Attempting to store with valid data, but disconnected connection")
        {
            conn.disconnect();
            REQUIRE(conn.isClosed());

            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName).withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE));

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
    }
}

SCENARIO("HdbppTxNewAttribute Simulated exception received", "[hdbpp-tx][hdbpp-tx-new-attribute]")
{
    hdbpp_new_attr_test::MockConnection conn;

    GIVEN("An HdbppTxNewAttribute object with name and traits set")
    {
        auto tx = conn.createTx<HdbppTxNewAttribute>()
                      .withName(TestAttrFQDName)
                      .withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);

        WHEN("Storing the transaction with a triggered exception set")
        {
            conn.store_attribute_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}