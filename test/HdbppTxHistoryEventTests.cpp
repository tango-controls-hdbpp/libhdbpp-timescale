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
#include "HdbppTxHistoryEvent.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

#include <string>
#include <vector>

using namespace std;
using namespace hdbpp;
using namespace hdbpp_test::attr_name;

namespace hdbpp_hist_event_test
{
// Mock connection to test the HdbppTxHistoryEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string &) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept { return _conn_state; }
    bool isClosed() const noexcept { return !isOpen(); }

    // storage API
    void storeHistoryEvent(const string &full_attr_name, const std::string &event)
    {
        if (store_attribute_triggers_ex)
            throw runtime_error("A test exception");

        att_name = full_attr_name;
        att_last_event = event;
        event_seq.push_back(event);
    }

    std::string fetchLastHistoryEvent(const string &) { return att_last_event; }

    // expose the results of the store function so they can be checked
    // in the results
    string att_name;
    string att_last_event;

    // record event seqence for more complex tests
    vector<string> event_seq;

    bool store_attribute_triggers_ex = false;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_hist_event_test

SCENARIO("Construct and store HdbppTxHistoryEvent data without error", "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("An HdbppTxNewAttribute object with no data set")
    {
        auto tx = conn.createTx<HdbppTxHistoryEvent>();

        WHEN("Passing a valid configuration with method chaining")
        {
            tx.withName(TestAttrFQDName).withEvent(events::InsertEvent);

            THEN("Storing the transaction does not raise an exception")
            {
                REQUIRE_NOTHROW(tx.store());
                REQUIRE(tx.result());
            }
            AND_WHEN("The result of the store is examined")
            {
                REQUIRE_NOTHROW(tx.store());

                THEN("The data is the same as that passed via method chaining")
                {
                    REQUIRE(conn.att_name == TestAttrFinalName);
                    REQUIRE(conn.att_last_event == events::InsertEvent);
                }
            }
        }
    }
}

SCENARIO("When attempting to store invalid HdbppTxHistoryEvent states, errors are thrown",
    "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("An HdbppTxHistoryEvent object with no data set")
    {
        auto tx = conn.createTx<HdbppTxHistoryEvent>();

        WHEN("Attempting to store without setting data")
        {
            THEN("An exception is raised") { REQUIRE_THROWS(tx.store()); }
            AND_WHEN("Setting a valid name and trying to store again")
            {
                tx.withName(TestAttrFQDName);

                THEN("An exception is raised since no event is set")
                {
                    REQUIRE_THROWS(tx.store());
                    REQUIRE(!tx.result());
                }
                AND_WHEN("Setting a valid event and trying to store again")
                {
                    tx.withEvent(events::InsertEvent);

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
            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName).withEvent(events::InsertEvent));

            THEN("An exception is raised")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
        WHEN("Attempting to set an invalid event type via the libhdb++ event interface")
        {
            THEN("An exception is raised")
            {
                REQUIRE_THROWS(tx.withName(TestAttrFQDName).withEvent(10));
                REQUIRE(!tx.result());
            }
        }
    }
}

SCENARIO("HdbppTxHistoryEvent's overloaded functions return identical results without error",
    "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("Two HdbppTxHistoryEvent objects with no data set")
    {
        auto tx1 = conn.createTx<HdbppTxHistoryEvent>();
        auto tx2 = conn.createTx<HdbppTxHistoryEvent>();

        WHEN("Setting the event on both HdbppTxHistoryEvent objects via overloaded functions")
        {
            tx1.withName(TestAttrFQDName);
            tx2.withName(TestAttrFQDName);

            REQUIRE_NOTHROW(tx1.withEvent(libhdbpp_compatibility::HdbppInsert));
            REQUIRE_NOTHROW(tx2.withEvent(events::InsertEvent));

            THEN("The events are the same after storing")
            {
                REQUIRE_NOTHROW(tx1.store());
                REQUIRE(tx1.result());
                auto tx1_att_last_event = conn.att_last_event;
                REQUIRE_NOTHROW(tx2.store());
                REQUIRE(tx2.result());
                REQUIRE(tx1_att_last_event == conn.att_last_event);
            }
        }
    }
}

SCENARIO("Detect CrashEvents due to double StartEvents", "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("An HdbppTxHistoryEvent object which has processed a StartEvent")
    {
        auto tx = conn.createTx<HdbppTxHistoryEvent>().withName(TestAttrFQDName).withEvent(events::StartEvent).store();

        REQUIRE(tx.result());
        REQUIRE(conn.att_last_event == events::StartEvent);

        WHEN("Attempting to store a second StartEvent")
        {
            tx.withName(TestAttrFQDName).withEvent(events::StartEvent).store();
            REQUIRE(conn.att_last_event == events::StartEvent);

            THEN("A crash event is stored first")
            {
                REQUIRE(conn.event_seq[0] == events::StartEvent);
                REQUIRE(conn.event_seq[1] == events::CrashEvent);
                REQUIRE(conn.event_seq[2] == events::StartEvent);
            }
        }
    }
}

SCENARIO("All event conversions result in correct events at store time", "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("An HdbppTxHistoryEvent object with name set")
    {
        auto tx = conn.createTx<HdbppTxHistoryEvent>().withName(TestAttrFQDName);

        WHEN("Attempting to store event libhdbpp_compatibility::HdbppInsert")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppInsert).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::InsertEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppStart")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppStart).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::StartEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppStop")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppStop).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::StopEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppRemove")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppRemove).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::RemoveEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppInsertParam")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppInsertParam).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::InsertParamEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppPause")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppPause).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::PauseEvent); }
        }
        WHEN("Attempting to store event libhdbpp_compatibility::HdbppUpdateTTL")
        {
            REQUIRE_NOTHROW(tx.withEvent(libhdbpp_compatibility::HdbppUpdateTTL).store());

            THEN("InsertEvent is recorded at store time") { REQUIRE(conn.att_last_event == events::UpdateTTLEvent); }
        }
    }
}

SCENARIO("HdbppTxHistoryEvent Simulated exception received", "[hdbpp-tx][hdbpp-tx-history-event]")
{
    hdbpp_hist_event_test::MockConnection conn;

    GIVEN("An HdbppTxHistoryEvent object with name and traits set")
    {
        auto tx = conn.createTx<HdbppTxHistoryEvent>().withName(TestAttrFQDName).withEvent(events::StartEvent);

        WHEN("Storing the transaction with a triggered exception set")
        {
            conn.store_attribute_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}