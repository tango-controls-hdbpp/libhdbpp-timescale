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
using namespace hdbpp;
using namespace hdbpp_test::attr_name;

namespace hdbpp_data_event_test
{
// Mock connection to test the HdbppTxDataEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection : public ConnectionBase, public HdbppTxFactory<MockConnection>
{
public:
    // Enforced connection API from ConnectionBase
    void connect(const string &) override { _conn_state = true; }
    void disconnect() override { _conn_state = false; }
    bool isOpen() const noexcept { return _conn_state; }
    bool isClosed() const noexcept { return !isOpen(); }

    template<typename T>
    void storeDataEvent(const std::string &full_attr_name,
        double event_time,
        int quality,
        std::unique_ptr<vector<T>> value_r,
        std::unique_ptr<vector<T>> value_w,
        const AttributeTraits &traits)
    {}

    void storeDataEventError(const std::string &full_attr_name,
        double event_time,
        int quality,
        const std::string &error_msg,
        const AttributeTraits &traits);
    {}

private:
    // connection is always open unless test specifies closed
    bool _conn_state = true;
};
}; // namespace hdbpp_data_event_test

SCENARIO("HdbppTvBase can construct a valid attribute name for storage", "[hdbpp-tx][hdbpp-tx-base]")
{
    GIVEN("An AttributeName object with valid name")
    {
        WHEN("Requesting the name for storage")
        {
            THEN("The name is valid and as expected") {}
        }
    }
}