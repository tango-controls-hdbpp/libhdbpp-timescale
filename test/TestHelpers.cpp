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

#include "TestHelpers.hpp"

using namespace std;
using namespace hdbpp_internal;

namespace hdbpp_test
{
namespace data_gen
{
    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_BOOLEAN>::array data<Tango::DEV_BOOLEAN>(int size, int dim_x, int dim_y)
    {
        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_BOOLEAN>::type>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.5);

        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_SHORT>::array data<Tango::DEV_SHORT>(int size, int dim_x, int dim_y)
    {
        return move(genericData<int16_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_LONG>::array data<Tango::DEV_LONG>(int size, int dim_x, int dim_y)
    {
        return move(genericData<int32_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_LONG64>::array data<Tango::DEV_LONG64>(int size, int dim_x, int dim_y)
    {
        return move(genericData<int64_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_DOUBLE>::array data<Tango::DEV_DOUBLE>(int size, int dim_x, int dim_y)
    {
        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_DOUBLE>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<typename TangoTypeTraits<Tango::DEV_DOUBLE>::type> d;
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_FLOAT>::array data<Tango::DEV_FLOAT>(int size, int dim_x, int dim_y)
    {
        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_FLOAT>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<typename TangoTypeTraits<Tango::DEV_FLOAT>::type> d;
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_UCHAR>::array data<Tango::DEV_UCHAR>(int size, int dim_x, int dim_y)
    {
        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_UCHAR>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> d(0, 255);
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_USHORT>::array data<Tango::DEV_USHORT>(int size, int dim_x, int dim_y)
    {
        return move(genericData<uint16_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_ULONG>::array data<Tango::DEV_ULONG>(int size, int dim_x, int dim_y)
    {
        return move(genericData<uint32_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_ULONG64>::array data<Tango::DEV_ULONG64>(int size, int dim_x, int dim_y)
    {
        return move(genericData<uint64_t>(size, dim_x, dim_y));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_STRING>::array data<Tango::DEV_STRING>(int size, int dim_x, int dim_y)
    {
        static vector<string> strings = {"this",
            "is",
            "a",
            "classic,",
            "a",
            "dish",
            "that",
            "will",
            "never",
            "go",
            "out",
            "of",
            "fashion",
            "If",
            "you're",
            "looking",
            "for",
            "a",
            "treat",
            "or",
            "to",
            "impress",
            "someone,",
            "this",
            "is",
            "at",
            "the",
            "top",
            "of",
            "my",
            "list.",
            "Whenever",
            "I",
            "make",
            "this",
            "recipe,",
            "I",
            "look",
            "forward",
            "to",
            "the",
            "moment",
            "when",
            "the",
            "steaks",
            "are",
            "returned",
            "to",
            "the",
            "pan",
            "to",
            "be",
            "covered",
            "in",
            "sauce.",
            "At",
            "that",
            "point,",
            "I",
            "just",
            "know",
            "how",
            "good",
            "it's",
            "going",
            "to",
            "taste",
            "more test's",
            "test '' quotes",
            "test \a escape"};

        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_STRING>::type>>();

        for (int i = 0; i < size; i++)
            value->push_back(strings[experimental::randint(0, ((int)strings.size()) - 1)]);

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_STATE>::array data<Tango::DEV_STATE>(int size, int dim_x, int dim_y)
    {
        auto value = make_unique<TangoValue<typename TangoTypeTraits<Tango::DEV_STATE>::type>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.25);

        for (int i = 0; i < size; i++)
            value->push_back(d(gen) ? Tango::ON : Tango::OFF);

	value->dim_x = dim_x;
	value->dim_y = dim_y;

        return value;
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_ENUM>::array data<Tango::DEV_ENUM>(int size, int dim_x, int dim_y)
    {
        return move(genericData<int16_t>(size, dim_x, dim_y));
    }
} // namespace data_gen

namespace utils
{
    //=============================================================================
    //=============================================================================
    vector<AttributeTraits> getTraits()
    {
        vector<AttributeTraits> traits_array {};

        vector<Tango::CmdArgType> types {Tango::DEV_BOOLEAN,
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
            Tango::DEV_ENCODED,
            Tango::DEV_ENUM};

        vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
        vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM, Tango::IMAGE};

        // loop for every combination of type in Tango
        for (auto &type : types)
            for (auto &format : format_types)
                for (auto &write : write_types)
                    traits_array.emplace_back(AttributeTraits {write, format, type});

        return traits_array;
    }

    //=============================================================================
    //=============================================================================
    vector<AttributeTraits> getTraitsImplemented()
    {
        vector<AttributeTraits> traits_array {};

        vector<Tango::CmdArgType> types {Tango::DEV_BOOLEAN,
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
            Tango::DEV_STATE};

        vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
        vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM, Tango::IMAGE};

        // loop for every combination of type in Tango
        for (auto &type : types)
            for (auto &format : format_types)
                for (auto &write : write_types)
                    traits_array.emplace_back(AttributeTraits {write, format, type});

        return traits_array;
    }
} // namespace utils
} // namespace hdbpp_test
