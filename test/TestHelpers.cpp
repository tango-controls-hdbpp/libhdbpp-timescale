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
using namespace hdbpp;

namespace hdbpp_test
{
namespace data_gen
{
    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_BOOLEAN>::array data<Tango::DEV_BOOLEAN>(int size)
    {
        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_BOOLEAN>::type>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.5);

        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_SHORT>::array data<Tango::DEV_SHORT>(int size)
    {
        return move(genericData<int16_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_LONG>::array data<Tango::DEV_LONG>(int size)
    {
        return move(genericData<int32_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_LONG64>::array data<Tango::DEV_LONG64>(int size)
    {
        return move(genericData<int64_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_DOUBLE>::array data<Tango::DEV_DOUBLE>(int size)
    {
        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_DOUBLE>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<typename TangoTypeTraits<Tango::DEV_DOUBLE>::type> d;
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_FLOAT>::array data<Tango::DEV_FLOAT>(int size)
    {
        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_FLOAT>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<typename TangoTypeTraits<Tango::DEV_FLOAT>::type> d;
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_UCHAR>::array data<Tango::DEV_UCHAR>(int size)
    {
        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_UCHAR>::type>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> d(0, 255);
        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_USHORT>::array data<Tango::DEV_USHORT>(int size)
    {
        return move(genericData<uint16_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_ULONG>::array data<Tango::DEV_ULONG>(int size)
    {
        return move(genericData<uint32_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_ULONG64>::array data<Tango::DEV_ULONG64>(int size)
    {
        return move(genericData<uint64_t>(size));
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_STRING>::array data<Tango::DEV_STRING>(int size)
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

        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_STRING>::type>>();

        for (int i = 0; i < size; i++)
            value->push_back(strings[experimental::randint(0, ((int)strings.size()) - 1)]);

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    typename TangoTypeTraits<Tango::DEV_STATE>::array data<Tango::DEV_STATE>(int size)
    {
        auto value = make_unique<vector<typename TangoTypeTraits<Tango::DEV_STATE>::type>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.25);

        for (int i = 0; i < size; i++)
            value->push_back(d(gen) ? Tango::ON : Tango::OFF);

        return move(value);
    }
} // namespace data_gen
} // namespace hdbpp_test