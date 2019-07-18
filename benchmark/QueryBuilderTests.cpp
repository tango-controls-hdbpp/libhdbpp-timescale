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
   
#include "QueryBuilder.hpp"
#include <benchmark/benchmark.h>

//=============================================================================
//=============================================================================
void bmAllocateQueryBuilder(benchmark::State& state) 
{
    // Test - Testing the time it takes to allocate a QueryBuilder, mainly for future test
    // reference
    hdbpp_internal::LogConfigurator::initLoggingMetrics(false, false);
    hdbpp_internal::LogConfigurator::setLoggingLevel(spdlog::level::err);

    for (auto _ : state)
        hdbpp_internal::pqxx_conn::QueryBuilder query_builder;
}

BENCHMARK(bmAllocateQueryBuilder);

//=============================================================================
//=============================================================================
void bmTraitsComparator(benchmark::State& state) 
{
    // TEST - Test the AttributeTraits comparator used in the cache inside QueryBuilder,
    // the test is against a full map with every possible tango traits combination 
    std::map<hdbpp_internal::AttributeTraits, std::string> trait_cache;

    vector<Tango::CmdArgType> types {Tango::DEV_DOUBLE,
        Tango::DEV_FLOAT,
        Tango::DEV_STRING,
        Tango::DEV_LONG,
        Tango::DEV_ULONG,
        Tango::DEV_LONG64,
        Tango::DEV_ULONG64,
        Tango::DEV_SHORT,
        Tango::DEV_USHORT,
        Tango::DEV_BOOLEAN,
        Tango::DEV_UCHAR,
        Tango::DEV_STATE,
        Tango::DEV_ENCODED,
        Tango::DEV_ENUM};

    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM, Tango::IMAGE};

    for (auto &type : types)
    {
        for (auto &format : format_types)
        {
            for (auto &write : write_types)
            {
                // add to the cache for future hits
                trait_cache.emplace(
                    hdbpp_internal::AttributeTraits{write, format, type}, 
                    to_string(write) + to_string(format) + to_string(type));
            }
        }
    }

    hdbpp_internal::AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

    for (auto _ : state)
        trait_cache.find(traits);
}

BENCHMARK(bmTraitsComparator);

//=============================================================================
//=============================================================================
static void writeTypeArgs(benchmark::internal::Benchmark* b) 
{
    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};

    for (auto & write_type : write_types)
        b->Args({static_cast<int>(write_type)});
}

//=============================================================================
//=============================================================================
template<typename T>
void bmStoreDataEventQueryNoCache(benchmark::State& state) 
{
    // TEST - Testing how long it takes to build an Insert Data Event query with
    // an empty cache (this forces the full string to be built)
    hdbpp_internal::LogConfigurator::initLoggingMetrics(false, false);
    hdbpp_internal::LogConfigurator::setLoggingLevel(spdlog::level::err);

    hdbpp_internal::AttributeTraits traits 
        {static_cast<Tango::AttrWriteType>(state.range(0)), Tango::SCALAR, Tango::DEV_DOUBLE};

    for (auto _ : state)
    {
        // define the builder here so its cache is always fresh
        hdbpp_internal::pqxx_conn::QueryBuilder query_builder;
        query_builder.storeDataEventQuery<T>(traits);
    }
}

//=============================================================================
//=============================================================================
template<typename T>
void bmStoreDataEventQueryCache(benchmark::State& state) 
{
    // TEST - Testing the full lookup for an Insert Data QueryEvent query when the cache
    // map is fully populated 
    hdbpp_internal::LogConfigurator::initLoggingMetrics(false, false);
    hdbpp_internal::LogConfigurator::setLoggingLevel(spdlog::level::err);

    hdbpp_internal::AttributeTraits traits 
        {static_cast<Tango::AttrWriteType>(state.range(0)), Tango::SCALAR, Tango::DEV_DOUBLE};

    vector<Tango::CmdArgType> types {Tango::DEV_DOUBLE,
        Tango::DEV_FLOAT,
        Tango::DEV_STRING,
        Tango::DEV_LONG,
        Tango::DEV_ULONG,
        Tango::DEV_LONG64,
        Tango::DEV_ULONG64,
        Tango::DEV_SHORT,
        Tango::DEV_USHORT,
        Tango::DEV_BOOLEAN,
        Tango::DEV_UCHAR,
        Tango::DEV_STATE,
        Tango::DEV_ENCODED,
        Tango::DEV_ENUM};

    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM, Tango::IMAGE};

    hdbpp_internal::pqxx_conn::QueryBuilder query_builder;

    for (auto &type : types)
        for (auto &format : format_types)
            for (auto &write : write_types)
                query_builder.storeDataEventQuery<T>(hdbpp_internal::AttributeTraits{write, format, type});

    for (auto _ : state)
        query_builder.storeDataEventQuery<T>(traits);
}

BENCHMARK_TEMPLATE(bmStoreDataEventQueryNoCache, bool)->Apply(writeTypeArgs);
BENCHMARK_TEMPLATE(bmStoreDataEventQueryCache, bool)->Apply(writeTypeArgs);

BENCHMARK_MAIN();