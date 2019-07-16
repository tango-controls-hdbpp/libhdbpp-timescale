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
#include "HdbppTxParameterEvent.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace Tango;
using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_test::attr_name;
using namespace hdbpp_test::attr_info;

namespace hdbpp_param_test
{
AttributeInfoEx createAttributeInfoEx()
{
    ArchiveEventInfo arch_event_info;
    arch_event_info.archive_rel_change = AttrInfoRel;
    arch_event_info.archive_abs_change = AttrInfoAbs;
    arch_event_info.archive_period = AttrInfoPeriod;

    AttributeEventInfo event_info;
    event_info.arch_event = arch_event_info;

    AttributeInfoEx attr_info;
    attr_info.description = AttrInfoDescription;
    attr_info.label = AttrInfoLabel;
    attr_info.unit = AttrInfoUnit;
    attr_info.standard_unit = AttrInfoStandardUnit;
    attr_info.display_unit = AttrInfoDisplayUnit;
    attr_info.format = AttrInfoFormat;
    attr_info.events = event_info;

    return attr_info;
}

// Mock connection to test the HdbppTxParameterEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string & /* connect_str */) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept override { return _conn_state; }
    bool isClosed() const noexcept override { return !isOpen(); }

    // storage API
    void storeParameterEvent(const std::string &full_attr_name,
        double event_time,
        const std::string &label,
        const std::string &unit,
        const std::string &standard_unit,
        const std::string &display_unit,
        const std::string &format,
        const std::string &archive_rel_change,
        const std::string &archive_abs_change,
        const std::string &archive_period,
        const std::string &description)
    {
        if (store_attribute_triggers_ex)
            throw runtime_error("A test exception");

        att_name = full_attr_name;
        att_event_time = event_time;
        att_label = label;
        att_unit = unit;
        att_standard_unit = standard_unit;
        att_display_unit = display_unit;
        att_format = format;
        att_archive_rel_change = archive_rel_change;
        att_archive_abs_change = archive_abs_change;
        att_archive_period = archive_period;
        att_description = description;
    }

    // expose the results of the store function so they can be checked
    // in the results
    string att_name;
    double att_event_time = 0;
    string att_label;
    string att_unit;
    string att_standard_unit;
    string att_display_unit;
    string att_format;
    string att_archive_rel_change;
    string att_archive_abs_change;
    string att_archive_period;
    string att_description;

    bool store_attribute_triggers_ex = false;

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_param_test

SCENARIO("Construct and store HdbppTxParameterEvent data without error", "[hdbpp-tx][hdbpp-tx-parameter-event]")
{
    hdbpp_param_test::MockConnection conn;

    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};

    // ugly, how is this dealt with in Tango!?!
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxParameterEvent object with no data set")
    {
        auto tx = conn.createTx<HdbppTxParameterEvent>();

        WHEN("Passing a valid configuration with method chaining")
        {
            tx.withName(TestAttrFQDName)
                .withAttrInfo(hdbpp_param_test::createAttributeInfoEx())
                .withEventTime(tango_tv);

            THEN("No exception is raised when storing the transaction") { REQUIRE_NOTHROW(tx.store()); }
            AND_WHEN("The result of the store is examined after storing")
            {
                REQUIRE_NOTHROW(tx.store());
                REQUIRE(tx.result());

                THEN("The data is the same as that passed via method chaining")
                {
                    REQUIRE(conn.att_name == TestAttrFinalName);
                    REQUIRE(conn.att_event_time == (tango_tv.tv_sec + tango_tv.tv_usec / 1.0e6));
                    REQUIRE(conn.att_label == AttrInfoLabel);
                    REQUIRE(conn.att_unit == AttrInfoUnit);
                    REQUIRE(conn.att_standard_unit == AttrInfoStandardUnit);
                    REQUIRE(conn.att_display_unit == AttrInfoDisplayUnit);
                    REQUIRE(conn.att_format == AttrInfoFormat);
                    REQUIRE(conn.att_archive_rel_change == AttrInfoRel);
                    REQUIRE(conn.att_archive_abs_change == AttrInfoAbs);
                    REQUIRE(conn.att_archive_period == AttrInfoPeriod);
                    REQUIRE(conn.att_description == AttrInfoDescription);
                }
            }
        }
    }
}

SCENARIO("When attempting to store invalid HdbppTxParameterEvent states, errors are thrown",
    "[hdbpp-tx][hdbpp-tx-parameter-event]")
{
    hdbpp_param_test::MockConnection conn;

    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};

    // ugly, how is this dealt with in Tango!?!
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxParameterEvent object with no data set")
    {
        auto tx = conn.createTx<HdbppTxParameterEvent>();

        WHEN("Attempting to store without setting data")
        {
            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
            AND_WHEN("Attempting to store with just name set")
            {
                tx.withName(TestAttrFQDName);

                THEN("An exception is raised and result is false")
                {
                    REQUIRE_THROWS(tx.store());
                    REQUIRE(!tx.result());
                }
                AND_WHEN("Setting the event time and storing")
                {
                    tx.withEventTime(tango_tv);

                    THEN("An exception is raised and result is false")
                    {
                        REQUIRE_THROWS(tx.store());
                        REQUIRE(!tx.result());
                    }
                    AND_WHEN("Setting the AttributeInfoEx and storing")
                    {
                        tx.withAttrInfo(hdbpp_param_test::createAttributeInfoEx());

                        THEN("No exception should be thrown and the result is valid")
                        {
                            REQUIRE_NOTHROW(tx.store());
                            REQUIRE(tx.result());
                        }
                    }
                }
            }
        }
        WHEN("Attempting to store with valid data, but disconnected connection")
        {
            conn.disconnect();
            REQUIRE(conn.isClosed());

            tx.withName(TestAttrFQDName)
                .withEventTime(tango_tv)
                .withAttrInfo(hdbpp_param_test::createAttributeInfoEx());

            THEN("An exception is raised and result is false")
            {
                REQUIRE_THROWS(tx.store());
                REQUIRE(!tx.result());
            }
        }
    }
}

SCENARIO("HdbppTxParameterEvent Simulated exception received", "[hdbpp-tx][hdbpp-tx-parameter-event]")
{
    hdbpp_param_test::MockConnection conn;

    struct timeval tv
    {};
    struct Tango::TimeVal tango_tv
    {};

    // ugly, how is this dealt with in Tango!?!
    gettimeofday(&tv, nullptr);
    tango_tv.tv_sec = tv.tv_sec;
    tango_tv.tv_usec = tv.tv_usec;
    tango_tv.tv_nsec = 0;

    GIVEN("An HdbppTxParameterEvent object with name and traits set")
    {
        auto tx = conn.createTx<HdbppTxParameterEvent>()
                      .withName(TestAttrFQDName)
                      .withAttrInfo(hdbpp_param_test::createAttributeInfoEx())
                      .withEventTime(tango_tv);

        WHEN("Storing the transaction with a triggered exception set")
        {
            conn.store_attribute_triggers_ex = true;

            THEN("An exception is raised") { REQUIRE_THROWS_AS(tx.store(), runtime_error); }
        }
    }
}