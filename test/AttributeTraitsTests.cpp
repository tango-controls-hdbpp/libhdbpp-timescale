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

#include "AttributeTraits.hpp"
#include "catch2/catch.hpp"

using namespace std;
using namespace hdbpp_internal;

SCENARIO("Attribute format returns expected results", "[attribute-traits]")
{
    GIVEN("Constructed AttributeTraits as an Array")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are an array")
        {
            THEN("Result is true") { REQUIRE(traits.isArray() == true); }
        }
        AND_WHEN("Checking if traits are a scalar")
        {
            THEN("Result is false") { REQUIRE(traits.isScalar() == false); }
        }
        AND_WHEN("Checking if traits are an image")
        {
            THEN("Result is false") { REQUIRE(traits.isImage() == false); }
        }
    }

    GIVEN("Constructed AttributeTraits as a Scalar")
    {
        AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are an array")
        {
            THEN("Result is false") { REQUIRE(traits.isArray() == false); }
        }
        AND_WHEN("Checking if traits are a scalar")
        {
            THEN("Result is true") { REQUIRE(traits.isScalar() == true); }
        }
        AND_WHEN("Checking if traits are an image")
        {
            THEN("Result is false") { REQUIRE(traits.isImage() == false); }
        }
    }

    GIVEN("Constructed AttributeTraits as an Image")
    {
        AttributeTraits traits {Tango::READ, Tango::IMAGE, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are an array")
        {
            THEN("Result is false") { REQUIRE(traits.isArray() == false); }
        }
        AND_WHEN("Checking if traits are a scalar")
        {
            THEN("Result is false") { REQUIRE(traits.isScalar() == false); }
        }
        AND_WHEN("Checking if traits are an image")
        {
            THEN("Result is true") { REQUIRE(traits.isImage() == true); }
        }
    }
}

SCENARIO("Attribute write type returns expected results", "[attribute-traits]")
{
    GIVEN("Constructed AttributeTraits as ReadOnly")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are ReadOnly")
        {
            THEN("Result is true") { REQUIRE(traits.isReadOnly() == true); }
        }
        AND_WHEN("Checking if traits are WriteOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isWriteOnly() == false); }
        }
        AND_WHEN("Checking if traits are ReadWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWrite() == false); }
        }
        AND_WHEN("Checking if traits are ReadWithWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWithWrite() == false); }
        }
        AND_WHEN("Checking if traits have read data")
        {
            THEN("Result is true") { REQUIRE(traits.hasReadData() == true); }
        }
        AND_WHEN("Checking if traits have write data")
        {
            THEN("Result is false") { REQUIRE(traits.hasWriteData() == false); }
        }
    }

    GIVEN("Constructed AttributeTraits as WriteOnly")
    {
        AttributeTraits traits {Tango::WRITE, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are ReadOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isReadOnly() == false); }
        }
        AND_WHEN("Checking if traits are WriteOnly")
        {
            THEN("Result is true") { REQUIRE(traits.isWriteOnly() == true); }
        }
        AND_WHEN("Checking if traits are ReadWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWrite() == false); }
        }
        AND_WHEN("Checking if traits are ReadWithWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWithWrite() == false); }
        }
        AND_WHEN("Checking if traits have read data")
        {
            THEN("Result is false") { REQUIRE(traits.hasReadData() == false); }
        }
        AND_WHEN("Checking if traits have write data")
        {
            THEN("Result is true") { REQUIRE(traits.hasWriteData() == true); }
        }
    }

    GIVEN("Constructed AttributeTraits as ReadWrite")
    {
        AttributeTraits traits {Tango::READ_WRITE, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are ReadOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isReadOnly() == false); }
        }
        AND_WHEN("Checking if traits are WriteOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isWriteOnly() == false); }
        }
        AND_WHEN("Checking if traits are ReadWrite")
        {
            THEN("Result is true") { REQUIRE(traits.isReadWrite() == true); }
        }
        AND_WHEN("Checking if traits are ReadWithWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWithWrite() == false); }
        }
        AND_WHEN("Checking if traits have read data")
        {
            THEN("Result is true") { REQUIRE(traits.hasReadData() == true); }
        }
        AND_WHEN("Checking if traits have write data")
        {
            THEN("Result is true") { REQUIRE(traits.hasWriteData() == true); }
        }
    }

    GIVEN("Constructed AttributeTraits as ReadWithWrite")
    {
        AttributeTraits traits {Tango::READ_WITH_WRITE, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits are ReadOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isReadOnly() == false); }
        }
        AND_WHEN("Checking if traits are WriteOnly")
        {
            THEN("Result is false") { REQUIRE(traits.isWriteOnly() == false); }
        }
        AND_WHEN("Checking if traits are ReadWrite")
        {
            THEN("Result is false") { REQUIRE(traits.isReadWrite() == false); }
        }
        AND_WHEN("Checking if traits are ReadWithWrite")
        {
            THEN("Result is true") { REQUIRE(traits.isReadWithWrite() == true); }
        }
        AND_WHEN("Checking if traits have read data")
        {
            THEN("Result is true") { REQUIRE(traits.hasReadData() == true); }
        }
        AND_WHEN("Checking if traits have write data")
        {
            THEN("Result is true") { REQUIRE(traits.hasWriteData() == true); }
        }
    }
}

SCENARIO("Attribute traits accessors return construction time results", "[attribute-traits]")
{
    GIVEN("Constructed AttributeTraits as a Read Only Array of type Tango::DEV_BOOLEAN")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DEV_BOOLEAN};
        REQUIRE(traits.isValid());

        WHEN("Checking if traits type is Tango::DEV_BOOLEAN")
        {
            THEN("Result is true") { REQUIRE(traits.type() == Tango::DEV_BOOLEAN); }
        }
        AND_WHEN("Checking if traits format is Tango::SPECTRUM")
        {
            THEN("Result is true") { REQUIRE(traits.formatType() == Tango::SPECTRUM); }
        }
        AND_WHEN("Checking if traits access type is Tango::READ")
        {
            THEN("Result is true") { REQUIRE(traits.writeType() == Tango::READ); }
        }
    }
}

SCENARIO("Attribute traits are invalid if not set with valid traits", "[attribute-traits]")
{
    GIVEN("Constructing an AttributeTraits with no traits")
    {
        AttributeTraits traits;

        WHEN("Checking if traits type is valid")
        {
            THEN("Result is false") 
            { 
                REQUIRE(!traits.isValid()); 
                REQUIRE(traits.isInvalid()); 
            }
        }
    }
    GIVEN("Constructing an AttributeTraits with valid write trait")
    {
        AttributeTraits traits {Tango::READ, Tango::FMT_UNKNOWN, Tango::DATA_TYPE_UNKNOWN};

        WHEN("Checking if traits type is valid")
        {
            THEN("Result is false") 
            { 
                REQUIRE(!traits.isValid()); 
                REQUIRE(traits.isInvalid()); 
            }
        }
    }
    GIVEN("Constructed AttributeTraits with valid format and write traits")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DATA_TYPE_UNKNOWN};

        WHEN("Checking if traits type is valid")
        {
            THEN("Result is false") 
            { 
                REQUIRE(!traits.isValid()); 
                REQUIRE(traits.isInvalid()); 
            }
        }
    }
    GIVEN("Constructed AttributeTraits with valid traits")
    {
        AttributeTraits traits {Tango::READ, Tango::SPECTRUM, Tango::DEV_BOOLEAN};

        WHEN("Checking if traits type is valid")
        {
            THEN("Result is true") 
            { 
                REQUIRE(traits.isValid()); 
                REQUIRE(!traits.isInvalid()); 
            }
        }
    }
}