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
    unique_ptr<vector<bool>> scalarData<bool>()
    {
        auto value = make_unique<vector<bool>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.5);
        value->push_back(d(gen));
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<uint8_t>> scalarData<uint8_t>()
    {
        auto value = make_unique<vector<uint8_t>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> d(0, 255);
        value->push_back(d(gen));
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<float>> scalarData<float>()
    {
        auto value = make_unique<vector<float>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<float> d;
        value->push_back(d(gen));
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<double>> scalarData<double>()
    {
        auto value = make_unique<vector<double>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> d;
        value->push_back(d(gen));
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<string>> scalarData<string>()
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
            "taste"};

        auto value = make_unique<vector<string>>();
        value->push_back(strings[experimental::randint(0, ((int)strings.size()) - 1)]);
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    unique_ptr<vector<int32_t>> scalarStateData()
    {
        auto value = make_unique<vector<int32_t>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.25);
        value->push_back(d(gen) ? Tango::ON : Tango::OFF);
        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<bool>> spectrumData<bool>(int size)
    {
        auto value = make_unique<vector<bool>>();
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
    unique_ptr<vector<uint8_t>> spectrumData<uint8_t>(int size)
    {
        auto value = make_unique<vector<uint8_t>>();
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
    unique_ptr<vector<float>> spectrumData<float>(int size)
    {
        auto value = make_unique<vector<float>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<float> d;

        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<double>> spectrumData<double>(int size)
    {
        auto value = make_unique<vector<double>>();
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> d;

        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    template<>
    unique_ptr<vector<string>> spectrumData<string>(int size)
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
            "my ",
            " list.",
            " Whenever ",
            " I ",
            " make ",
            " this ",
            " recipe,",
            " I ",
            " look ",
            " forward ",
            " to ",
            " the ",
            " moment ",
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
            "taste"};

        auto value = make_unique<vector<string>>();

        for (int i = 0; i < size; i++)
            value->push_back(strings[experimental::randint(0, ((int)strings.size()) - 1)]);

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    unique_ptr<vector<int32_t>> spectrumStateData(int size)
    {
        auto value = make_unique<vector<int32_t>>();
        random_device rd;
        mt19937 gen(rd());
        bernoulli_distribution d(0.25);

        for (int i = 0; i < size; i++)
            value->push_back(d(gen) ? Tango::ON : Tango::OFF);

        return move(value);
    }

    //=============================================================================
    //=============================================================================
    unique_ptr<vector<int32_t>> generateStateData(const AttributeTraits &traits, bool empty_data)
    {
        if (!empty_data)
        {
            if (traits.isArray())
                return spectrumStateData(1024);
            else
                return scalarStateData();
        }

        return move(make_unique<vector<int32_t>>());
    }
} // namespace data_gen
} // namespace hdbpp_test