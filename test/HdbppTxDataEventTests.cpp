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
#include "HdbppTxDataEvent.hpp"
#include "HdbppTxFactory.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_test::attr_name;
using namespace hdbpp_test::data_gen;

namespace hdbpp_data_event_test
{
Tango::DeviceAttribute createDeviceAttribute(const AttributeTraits &traits)
{
    // this is all one messy hack as Tango is not easy to use in a unit test, in this case, we can not
    // set up and create a DeviceAttribute easy. Instead we use the publicly (!!) available variables
    auto attr_gen = [&traits](int size_x, int size_y) {
        switch (traits.type())
        {
            case Tango::DEV_BOOLEAN:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_BOOLEAN>(false, size_x + size_y));

            case Tango::DEV_SHORT:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_SHORT>(false, size_x + size_y));

            case Tango::DEV_LONG:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_LONG>(false, size_x + size_y));

            case Tango::DEV_LONG64:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_LONG64>(false, size_x + size_y));

            case Tango::DEV_FLOAT:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_FLOAT>(false, size_x + size_y));

            case Tango::DEV_DOUBLE:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_DOUBLE>(false, size_x + size_y));

            case Tango::DEV_UCHAR:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_UCHAR>(false, size_x + size_y));

            case Tango::DEV_USHORT:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_USHORT>(false, size_x + size_y));

            case Tango::DEV_ULONG:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_ULONG>(false, size_x + size_y));

            case Tango::DEV_ULONG64:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_ULONG64>(false, size_x + size_y));

            case Tango::DEV_STRING:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_STRING>(false, size_x + size_y));

            case Tango::DEV_STATE:
                return Tango::DeviceAttribute(
                    TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_STATE>(false, size_x + size_y));

                //case Tango::DEV_ENUM:
                //return Tango::DeviceAttribute(TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_ENUM>(false, size_x + size_y));

                //case Tango::DEV_ENCODED:
                //return Tango::DeviceAttribute(TestAttrFQDName.c_str(), *generateSpectrumData<Tango::DEV_ENCODED>(false, size_x + size_y));

            default: throw "Should never be here!";
        }

        throw runtime_error("Control should not reach here");
        return Tango::DeviceAttribute();
    };

    auto select_size = [](bool has_data, int size) {
        if (has_data)
            return size;

        return 1;
    };

    // hack hack hack.... this mess of forced public (!) variable setting actually
    // sets up the conditions for valid extracts inside the HdbppTxDataEvent class.
    // Should the Tango API ever be improved then this hack is doomed.
    auto attr = attr_gen(select_size(traits.hasReadData(), 1), select_size(traits.hasWriteData(), 1));
    attr.dim_x = select_size(traits.hasReadData(), 1);
    attr.dim_y = select_size(traits.hasWriteData(), 1);
    attr.w_dim_x = select_size(traits.hasReadData(), 1);
    attr.w_dim_y = select_size(traits.hasWriteData(), 1);
    return attr;
}

// Mock connection to test the HdbppTxDataEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string & /* connect_string */) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept override { return _conn_state; }
    bool isClosed() const noexcept override { return !isOpen(); }

    template<typename T>
    void storeDataEvent(const std::string &full_attr_name,
        double event_time,
        int quality,
        std::unique_ptr<vector<T>> value_r,
        std::unique_ptr<vector<T>> value_w,
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
        data_size_r = value_r->size();
        data_size_w = value_w->size();
    }

    // expose the results of the store function so they can be checked
    // in the results

    // storeDataEvent/storeDataEventError results
    string att_name;
    double att_event_time = 0;
    Tango::AttrQuality att_quality = Tango::ATTR_INVALID;
    AttributeTraits att_traits;
    int data_size_r = -1;
    int data_size_w = -1;
    bool store_attribute_triggers_ex = false;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_data_event_test

SCENARIO("Construct a valid HdbppTxDataEvent data event for storage", "[hdbpp-tx][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};

    struct Tango::TimeVal tango_tv
    {};

    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("Traits for a device attribute")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);

        WHEN("Configuring an HdbppTxDataEvent object as a scalar data event and storing")
        {
            auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
            auto tx = conn.createTx<HdbppTxDataEvent>();

            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(traits)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withAttribute(&attr));

            REQUIRE_NOTHROW(tx.store());

            THEN("The data is the same as that passed via method chaining")
            {
                REQUIRE(conn.att_name == TestAttrFinalName);
                REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                REQUIRE(conn.att_traits == traits);
                REQUIRE(conn.data_size_r == 1);
                REQUIRE(conn.data_size_w == 0);
            }
        }
        WHEN("Configuring an HdbppTxDataEvent object as a spectrum data event")
        {
            auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
            auto tx = conn.createTx<HdbppTxDataEvent>();

            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(traits)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withAttribute(&attr));

            REQUIRE_NOTHROW(tx.store());

            THEN("The data is the same as that passed via method chaining")
            {
                REQUIRE(conn.att_name == TestAttrFinalName);
                REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                REQUIRE(conn.att_traits == traits);
                REQUIRE(conn.data_size_r > 0);
                REQUIRE(conn.data_size_w == 0);
            }
        }
    }
}

SCENARIO("An invalid quality results in an HdbppTxDataEvent event with no data", "[hdbpp-tx][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};

    struct Tango::TimeVal tango_tv
    {};

    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEvent object with no data set")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
        auto tx = conn.createTx<HdbppTxDataEvent>();

        WHEN("Configuring an HdbppTxDataEvent object with an invalid quality")
        {
            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(traits)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_INVALID)
                                .withAttribute(&attr));

            REQUIRE_NOTHROW(tx.store());

            THEN("The data is the same as that passed via method chaining")
            {
                REQUIRE(conn.att_name == TestAttrFinalName);
                REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                REQUIRE(conn.att_quality == Tango::ATTR_INVALID);
                REQUIRE(conn.att_traits == traits);
                REQUIRE(conn.data_size_r == 0);
                REQUIRE(conn.data_size_w == 0);
            }
        }
    }
}

SCENARIO("A DeviceAttribute with no data results in an HdbppTxDataEvent event with no data",
    "[hdbpp-tx][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};

    struct Tango::TimeVal tango_tv
    {};

    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEvent object with no data set")
    {
        Tango::DeviceAttribute attr;
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto tx = conn.createTx<HdbppTxDataEvent>();

        WHEN("Configuring an HdbppTxDataEvent object with an empty DeviceAttribute")
        {
            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(traits)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withAttribute(&attr));

            REQUIRE_NOTHROW(tx.store());

            THEN("The data is the same as that passed via method chaining")
            {
                REQUIRE(conn.att_name == TestAttrFinalName);
                REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                REQUIRE(conn.att_traits == traits);
                REQUIRE(conn.data_size_r == 0);
                REQUIRE(conn.data_size_w == 0);
            }
        }
    }
}

SCENARIO(
    "When attempting to store invalid HdbppTxDataEvent states, errors are thrown", "[hdbpp-tx][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};

    struct Tango::TimeVal tango_tv
    {};

    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxDataEvent object with no data set")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto tx = conn.createTx<HdbppTxDataEvent>();

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
                AND_WHEN("Setting the DeviceAttribute and trying again")
                {
                    auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
                    tx.withAttribute(&attr);

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

            auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
            auto tx = conn.createTx<HdbppTxDataEvent>();

            REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                .withTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE)
                                .withEventTime(tango_tv)
                                .withQuality(Tango::ATTR_VALID)
                                .withAttribute(&attr));

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
    }
}

TEST_CASE("Creating HdbppTxDataEvents for each tango type and storing them", "[db-access][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};

    struct Tango::TimeVal tango_tv
    {};

    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    vector<Tango::CmdArgType> types {
        Tango::DEV_BOOLEAN,
        Tango::DEV_DOUBLE,
        Tango::DEV_FLOAT,
        Tango::DEV_STRING,
        Tango::DEV_LONG,
        Tango::DEV_ULONG,
        Tango::DEV_LONG64,
        Tango::DEV_ULONG64,
        Tango::DEV_SHORT,
        Tango::DEV_USHORT,
        Tango::DEV_UCHAR,
        Tango::DEV_STATE,
        /* Tango::DEV_ENCODED, 
        Tango::DEV_ENUM */};

    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM};

    // loop for every combination of type in Tango
    for (auto &type : types)
    {
        for (auto &format : format_types)
        {
            for (auto &write : write_types)
            {
                AttributeTraits traits {write, format, type};

                DYNAMIC_SECTION("Storing with traits: " << traits)
                {
                    auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
                    auto tx = conn.createTx<HdbppTxDataEvent>();

                    REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                                        .withTraits(traits)
                                        .withEventTime(tango_tv)
                                        .withQuality(Tango::ATTR_VALID)
                                        .withAttribute(&attr)
                                        .store());

                    REQUIRE(conn.att_name == TestAttrFinalName);
                    REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                    REQUIRE(conn.att_quality == Tango::ATTR_VALID);
                    REQUIRE(conn.att_traits == traits);

                    if (traits.hasReadData())
                        REQUIRE(conn.data_size_r > 0);

                    if (traits.hasWriteData())
                        REQUIRE(conn.data_size_w > 0);
                }
            }
        }
    }
}

SCENARIO("HdbppTxDataEvent Simulated exception received", "[hdbpp-tx][hdbpp-tx-data-event]")
{
    hdbpp_data_event_test::MockConnection conn;

    // ugly, how is this dealt with in Tango!?!
    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxHisHdbppTxDatatoryEventEvent object with name and traits set")
    {
        auto traits = AttributeTraits(Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE);
        auto attr = hdbpp_data_event_test::createDeviceAttribute(traits);
        auto tx = conn.createTx<HdbppTxDataEvent>();

        REQUIRE_NOTHROW(tx.withName(TestAttrFQDName)
                            .withTraits(traits)
                            .withEventTime(tango_tv)
                            .withQuality(Tango::ATTR_VALID)
                            .withAttribute(&attr));

        WHEN("Storing the transaction with a triggered exception set")
        {
            conn.store_attribute_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}
