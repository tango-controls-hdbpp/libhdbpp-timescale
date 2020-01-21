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
#include "HdbppTxDataEventError.hpp"
#include "HdbppTxFactory.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_test::attr_name;

namespace hdbpp_data_event_test_error
{
const string TestError = "A test error message";

// Mock connection to test the HdbppTxDataEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string & /* connect_str */) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept override { return _conn_state; }
    bool isClosed() const noexcept override { return !isOpen(); }

    void storeDataEventError(const string &full_attr_name,
        double event_time,
        int quality,
        const string &error_msg,
        const AttributeTraits &traits)
    {
        if (store_attribute_triggers_ex)
            throw runtime_error("A test exception");

        // record the data for comparison
        att_name = full_attr_name;
        att_event_time = event_time;
        // TODO make it Tango::AttrQuality for API
        att_quality = (Tango::AttrQuality)quality;
        att_traits = traits;
        att_error_msg = error_msg;
    }

    // expose the results of the store function so they can be checked
    // in the results

    // storeDataEvent/storeDataEventError results
    string att_name;
    double att_event_time = 0;
    Tango::AttrQuality att_quality = Tango::ATTR_INVALID;
    AttributeTraits att_traits;
    string att_error_msg;
    bool store_attribute_triggers_ex = false;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_data_event_test_error

SCENARIO("Construct a valid HdbppTxDataEventError error event for storage", "[hdbpp-tx][hdbpp-tx-data-event-error]")
{
    hdbpp_data_event_test_error::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEventError object with no data set")
    {
        auto tx = conn.createTx<HdbppTxDataEventError>();
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);

        WHEN("Configuring an HdbppTxDataEventError object with an error")
        {
            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(traits)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withError(hdbpp_data_event_test_error::TestError));

            REQUIRE_NOTHROW(tx.store());

            THEN("The data is the same as that passed via method chaining, and there is no data")
            {
                REQUIRE(conn.att_name == TestAttrFinalName);
                REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                REQUIRE(conn.att_traits == traits);
                REQUIRE(conn.att_error_msg == hdbpp_data_event_test_error::TestError);
            }
        }
    }
}

SCENARIO("When attempting to store invalid HdbppTxDataEventError states, errors are thrown",
    "[hdbpp-tx][hdbpp-tx-data-event-error]")
{
    hdbpp_data_event_test_error::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEventError object with no data set")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto tx = conn.createTx<HdbppTxDataEventError>();

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
            tx.withName(TestAttrFQDName);

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
            AND_WHEN("Setting the AtributeTraits and trying again")
            {
                tx.withTraits(traits);

                THEN("An exception is raised and result is false")
                {
                    REQUIRE_THROWS(tx.store());
                    REQUIRE(!tx.result());
                }
                AND_WHEN("Setting the error message and trying again")
                {
                    tx.withError(hdbpp_data_event_test_error::TestError);

                    THEN("No exception is raised")
                    {
                        REQUIRE_NOTHROW(tx.store());
                        REQUIRE(tx.result());
                    }
                }
            }
        }
        WHEN("Attempting to store with valid data, but disconnected connection")
        {
            conn.disconnect();
            REQUIRE(conn.isClosed());

            auto tx = conn.createTx<HdbppTxDataEventError>();

            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withError(hdbpp_data_event_test_error::TestError));

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
    }
}

SCENARIO("HdbppTxDataEventError Simulated exception received", "[hdbpp-tx][hdbpp-tx-data-event-error]")
{
    hdbpp_data_event_test_error::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEventError object with name and traits set")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto tx = conn.createTx<HdbppTxDataEventError>();

        REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                            .withTraits(traits)
                            .withEventTime(tango_tv)
                            .withQuality(Tango::ATTR_VALID)
                            .withError(hdbpp_data_event_test_error::TestError));

        WHEN("Storing the transaction with a triggered exception set")
        {
            conn.store_attribute_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}