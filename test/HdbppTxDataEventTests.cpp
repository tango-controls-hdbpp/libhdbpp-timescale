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
#include "HdbppTxEventData.hpp"
#include "HdbppTxFactory.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace tango;
using namespace hdbpp;
using namespace hdbpp_test::attr_name;

namespace hdbpp_data_event_test
{
DeviceAttribute createDeviceAttribute(const AttributeTrits &traits)
{
    switch (traits.type())
    {
        //case Tango::DEV_BOOLEAN: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_BOOLEAN>(traits));
        case Tango::DEV_SHORT: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_SHORT>(traits));
        case Tango::DEV_LONG: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_LONG>(traits));
        case Tango::DEV_LONG64: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_LONG64>(traits));
        case Tango::DEV_FLOAT: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_FLOAT>(traits));
        case Tango::DEV_DOUBLE: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_DOUBLE>(traits));
        case Tango::DEV_UCHAR: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_UCHAR>(traits));
        case Tango::DEV_USHORT: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_USHORT>(traits));
        case Tango::DEV_ULONG: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_ULONG>(traits));
        case Tango::DEV_ULONG64: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_ULONG64>(traits));
        case Tango::DEV_STRING: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_STRING>(traits));
        case Tango::DEV_STATE:
            return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_STATE>(traits));
            //case Tango::DEV_ENUM: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_BOOLEAN>(traits));
            //case Tango::DEV_ENCODED: return DeviceAttribute attr(TestAttrFQDName, *generateData<Tango::DEV_BOOLEAN>(traits));
    }
}

// Mock connection to test the HdbppTxDataEvent class, only
// implements the functions that storeAttribute use, nothing more
template<typename T>
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string &) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept { return _conn_state; }
    bool isClosed() const noexcept { return !isOpen(); }

    template<typename U>
    void storeDataEvent(const std::string &full_attr_name,
        double event_time,
        int quality,
        std::unique_ptr<vector<T>> value_r,
        std::unique_ptr<vector<T>> value_w,
        const AttributeTraits &traits)
    {
        att_name = full_attr_name;
        att_event_time = event_time;
        att_quality = quality;
        att_traits = traits;
        att_value_r = *value_r;
        attr_value_w = *value_w;
    }

    void storeDataEventError(
        const std::string &full_attr_name, double event_time, int quality, const std::string &error_msg, const AttributeTraits &traits);
    {
        att_name = full_attr_name;
        att_event_time = event_time;
        att_quality = quality;
        att_traits = traits;
        att_error_msg = error_msg;
    }

    string att_name;
    double att_event_time;
    int att_quality;
    AttributeTraits att_traits;
    string att_error_msg;

    vector<T> attr_value_r;
    vector<T> attr_value_w;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_data_event_test

SCENARIO("Construct a valid HdbppTxEventData data event for storage", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // seriously ugly, how is this dealt with in Tango!?!
    gettimeofday(&tv, NULL);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An empty HdbppTxEventData object")
    {
        auto attr = createDeviceAttribute(AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE));
        auto tx = conn.createTx<HdbppTxEventData>();

        WHEN("Configuring the HdbppTxEventData object with event chaining")
        {
            tx.withName(TestAttrFQDName).withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
            .withEventTime(tango_tv).withQuality(Tango::ATTR_VALID).withAttribute(&attr);

            THEN("Then storing the transaction does not raise an exception") { REQUIRE_NOTHROW(tx.store()); }
            AND_WHEN("The result of the store is examined after storing")
            {
                THEN("The data is the same as that passed via method chaining") {}
            }
        }
    }
}

SCENARIO("Construct a valid HdbppTxEventData error event for storage", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    string error_msg = "Test error";
    hdbpp_data_event_test::MockConnection conn;

    // seriously ugly, how is this dealt with in Tango!?!
    gettimeofday(&tv, NULL);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An empty HdbppTxEventData object")
    {
        auto attr = createDeviceAttribute(AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE));
        auto tx = conn.createTx<HdbppTxEventData>();

        WHEN("Configuring the HdbppTxEventData object with event chaining")
        {
            tx.withName(TestAttrFQDName).withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
            .withEventTime(tango_tv).withQuality(Tango::ATTR_VALID).withError(error_msg);

            THEN("Then storing the transaction does not raise an exception") { REQUIRE_NOTHROW(tx.store()); }
            AND_WHEN("The result of the store is examined after storing")
            {
                THEN("The data is the same as that passed via method chaining")
                {
                    REQUIRE(conn.att_name == TestAttrFinalName);
                    REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                    REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                    REQUIRE(conn.att_error_msg == error_msg);
                    REQUIRE(conn.att_traits == AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE));
                }
            }
        }
    }
}

SCENARIO("An invalid quality results in an HdbppTxEventData event with no data", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    GIVEN("An AttributeName object with valid name")
    {
        WHEN("Requesting the name for storage")
        {
            THEN("The name is valid and as expected") {}
        }
    }
}

SCENARIO("When attempting to store invalid HdbppTxEventData states, errors are thrown", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

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

SCENARIO("Can store every tango data type", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    GIVEN("An AttributeName object with valid name")
    {
        WHEN("Requesting the name for storage")
        {
            THEN("The name is valid and as expected") {}
        }
    }
}

SCENARIO("HdbppTxHistoryEvent Simulated exception received", "[hdbpp-tx][hdbpp-txt-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

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