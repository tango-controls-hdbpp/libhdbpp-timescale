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
#include "HdbppTxBase.hpp"
#include "HdbppTxFactory.hpp"
#include "TestHelpers.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;
using namespace hdbpp_test::attr_name;

namespace hdbpp_base_test
{
// Mock connection to test the HdbppTxHistoryEvent class, only
// implements the functions that storeAttribute use, nothing more
class MockConnection
{};

class TestHdbppTxBase : public HdbppTxBase<MockConnection>
{
public:
    static string testAttrNameForStorage(AttributeName &attr_name)
    {
        return HdbppTxBase<MockConnection>::attrNameForStorage(attr_name);
    }
};
}; // namespace hdbpp_base_test

SCENARIO("HdbppTvBase can construct a valid attribute name for storage", "[hdbpp-tx][hdbpp-tx-base]")
{
    GIVEN("An AttributeName object with valid name")
    {
        AttributeName attr_name(TestAttrFQDName);

        WHEN("Requesting the name for storage")
        {
            THEN("The name is valid and as expected")
            {
                REQUIRE(hdbpp_base_test::TestHdbppTxBase::testAttrNameForStorage(attr_name) == TestAttrFinalName);
            }
        }
    }
}