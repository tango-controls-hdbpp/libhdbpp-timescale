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

#include "DbConnection.hpp"
#include "HdbppDefines.hpp"
#include "QueryBuilder.hpp"
#include "TestHelpers.hpp"

#include <benchmark/benchmark.h>
#include <memory>

//=============================================================================
//=============================================================================
void clearTables()
{
    auto conn = make_unique<pqxx::connection>(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);

    auto traits_array = hdbpp_test::utils::getTraits();

    {
        auto query = string("TRUNCATE ");

        pqxx::work tx {*conn};

        for (auto &traits : traits_array)
        {
            query += hdbpp_internal::pqxx_conn::QueryBuilder::tableName(traits);
            query += ",";
        }

        query += hdbpp_internal::pqxx_conn::schema::ErrTableName + ",";
        query += hdbpp_internal::pqxx_conn::schema::ParamTableName + ",";
        query += hdbpp_internal::pqxx_conn::schema::HistoryEventTableName + ",";
        query += hdbpp_internal::pqxx_conn::schema::HistoryTableName + ",";
        query += hdbpp_internal::pqxx_conn::schema::ConfTableName + " RESTART IDENTITY";

        tx.exec(query);
        tx.commit();
    }
}

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmAllocateData(benchmark::State &state)
{
    // TEST - Test data allocation time, so we can subtract it from the db write
    // tests
    hdbpp_internal::LogConfigurator::initLogging("test");
    hdbpp_internal::AttributeTraits traits {Tango::READ_WRITE, Format, Type};

    for (auto _ : state)
    {
        for (int i = 0; i < 1000; i++)
        {
            if (Format == Tango::SPECTRUM)
            {
                auto value_r = hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size);
                auto value_w = hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size);
            }
            else
            {
                auto value_r = hdbpp_test::data_gen::generateData<Type>(traits);
                auto value_w = hdbpp_test::data_gen::generateData<Type>(traits);
            }
        }
    }
}

BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_SHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_LONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_LONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_USHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_ULONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_STRING, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_STATE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_BOOLEAN, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_LONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_DOUBLE, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_USHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_ULONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_STRING, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmAllocateData, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmDbMultipleSQLInsert(benchmark::State &state)
{
    // TEST - Test the write speed when pushing a single event at a time
    // to the database. This version inserts via strings
    hdbpp_internal::LogConfigurator::initLogging("test");
    clearTables();

    hdbpp_internal::AttributeTraits traits {Tango::READ, Format, Type};

    hdbpp_internal::pqxx_conn::DbConnection conn(hdbpp_internal::pqxx_conn::DbConnection::DbStoreMethod::InsertString);

    conn.connect(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);

    conn.storeAttribute(hdbpp_test::attr_name::TestAttrFinalName,
        hdbpp_test::attr_name::TestAttrCs,
        hdbpp_test::attr_name::TestAttrDomain,
        hdbpp_test::attr_name::TestAttrFamily,
        hdbpp_test::attr_name::TestAttrMember,
        hdbpp_test::attr_name::TestAttrName,
        0,
        traits);

    struct timeval tv
    {};

    for (auto _ : state)
    {
        for (int i = 0; i < 1000; i++)
        {
            gettimeofday(&tv, nullptr);
            double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

            if (Format == Tango::SPECTRUM)
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    traits);
            }
            else
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    traits);
            }
        }
    }

    conn.disconnect();
}

BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_SHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_LONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_LONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_USHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_ULONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_STRING, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_STATE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_BOOLEAN, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_LONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_DOUBLE, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_USHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_ULONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_STRING, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultipleSQLInsert, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmDbMultiplePreparedStatementInsert(benchmark::State &state)
{
    // TEST - Test the write speed when pushing a single event at a time
    // to the database. This version inserts via prepared statements
    hdbpp_internal::LogConfigurator::initLogging("test");
    clearTables();

    hdbpp_internal::AttributeTraits traits {Tango::READ, Format, Type};

    hdbpp_internal::pqxx_conn::DbConnection conn(
        hdbpp_internal::pqxx_conn::DbConnection::DbStoreMethod::PreparedStatement);

    conn.connect(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);

    conn.storeAttribute(hdbpp_test::attr_name::TestAttrFinalName,
        hdbpp_test::attr_name::TestAttrCs,
        hdbpp_test::attr_name::TestAttrDomain,
        hdbpp_test::attr_name::TestAttrFamily,
        hdbpp_test::attr_name::TestAttrMember,
        hdbpp_test::attr_name::TestAttrName,
        0,
        traits);

    struct timeval tv
    {};

    for (auto _ : state)
    {
        for (int i = 0; i < 1000; i++)
        {
            gettimeofday(&tv, nullptr);
            double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

            if (Format == Tango::SPECTRUM)
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    traits);
            }
            else
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    traits);
            }
        }
    }

    conn.disconnect();
}

BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_SHORT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_LONG, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_LONG64, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_USHORT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_ULONG, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_STRING, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_STATE, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_BOOLEAN, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_LONG64, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_DOUBLE, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_USHORT, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_ULONG64, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_STRING, Tango::SCALAR)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbMultiplePreparedStatementInsert, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmDbBatchedInsert(benchmark::State &state)
{
    // TEST - Test the write speed when pushing a multiple events at once to
    // the db
    hdbpp_internal::LogConfigurator::initLogging("test");
    clearTables();

    hdbpp_internal::AttributeTraits traits {Tango::READ, Format, Type};

    hdbpp_internal::pqxx_conn::DbConnection conn(
        hdbpp_internal::pqxx_conn::DbConnection::DbStoreMethod::PreparedStatement);

    conn.connect(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);
    conn.buffer(true);

    conn.storeAttribute(hdbpp_test::attr_name::TestAttrFinalName,
        hdbpp_test::attr_name::TestAttrCs,
        hdbpp_test::attr_name::TestAttrDomain,
        hdbpp_test::attr_name::TestAttrFamily,
        hdbpp_test::attr_name::TestAttrMember,
        hdbpp_test::attr_name::TestAttrName,
        0,
        traits);

    struct timeval tv
    {};

    for (auto _ : state)
    {
        for (int i = 0; i < 1000; i++)
        {
            gettimeofday(&tv, nullptr);
            double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

            if (Format == Tango::SPECTRUM)
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                    traits);
            }
            else
            {
                conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                    event_time,
                    1,
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    move(hdbpp_test::data_gen::generateData<Type>(traits)),
                    traits);
            }
        }

        conn.flush();
    }

    conn.buffer(false);
    conn.disconnect();
}

BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_SHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_LONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_LONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_USHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_ULONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_STRING, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_STATE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_BOOLEAN, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_LONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_DOUBLE, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_USHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_ULONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_STRING, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbBatchedInsert, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmDbSingleSQLInsert(benchmark::State &state)
{
    // TEST - Test the write speed when pushing a single event via sql
    hdbpp_internal::LogConfigurator::initLogging("test");
    clearTables();

    hdbpp_internal::AttributeTraits traits {Tango::READ, Format, Type};

    hdbpp_internal::pqxx_conn::DbConnection conn(hdbpp_internal::pqxx_conn::DbConnection::DbStoreMethod::InsertString);

    conn.connect(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);

    conn.storeAttribute(hdbpp_test::attr_name::TestAttrFinalName,
        hdbpp_test::attr_name::TestAttrCs,
        hdbpp_test::attr_name::TestAttrDomain,
        hdbpp_test::attr_name::TestAttrFamily,
        hdbpp_test::attr_name::TestAttrMember,
        hdbpp_test::attr_name::TestAttrName,
        0,
        traits);

    struct timeval tv
    {};

    for (auto _ : state)
    {
        gettimeofday(&tv, nullptr);
        double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

        if (Format == Tango::SPECTRUM)
        {
            conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                event_time,
                1,
                move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                traits);
        }
        else
        {
            conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                event_time,
                1,
                move(hdbpp_test::data_gen::generateData<Type>(traits)),
                move(hdbpp_test::data_gen::generateData<Type>(traits)),
                traits);
        }
    }

    conn.buffer(false);
    conn.disconnect();
}

BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_SHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_LONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_LONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_USHORT, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_ULONG, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_STRING, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_STATE, Tango::SPECTRUM, 512)->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_BOOLEAN, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_LONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_DOUBLE, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_USHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_ULONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_STRING, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSingleSQLInsert, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type, Tango::AttrDataFormat Format, int Size = 0>
void bmDbSinglePreparedStatementInsert(benchmark::State &state)
{
    // TEST - Test the write speed when pushing a single event via sql
    hdbpp_internal::LogConfigurator::initLogging("test");
    clearTables();

    hdbpp_internal::AttributeTraits traits {Tango::READ, Format, Type};

    hdbpp_internal::pqxx_conn::DbConnection conn(
        hdbpp_internal::pqxx_conn::DbConnection::DbStoreMethod::PreparedStatement);

    conn.connect(hdbpp_test::psql_connection::postgres_db::HdbppConnectionString);

    conn.storeAttribute(hdbpp_test::attr_name::TestAttrFinalName,
        hdbpp_test::attr_name::TestAttrCs,
        hdbpp_test::attr_name::TestAttrDomain,
        hdbpp_test::attr_name::TestAttrFamily,
        hdbpp_test::attr_name::TestAttrMember,
        hdbpp_test::attr_name::TestAttrName,
        0,
        traits);

    struct timeval tv
    {};

    for (auto _ : state)
    {
        gettimeofday(&tv, nullptr);
        double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

        if (Format == Tango::SPECTRUM)
        {
            conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                event_time,
                1,
                move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                move(hdbpp_test::data_gen::generateSpectrumData<Type>(false, Size)),
                traits);
        }
        else
        {
            conn.storeDataEvent(hdbpp_test::attr_name::TestAttrFinalName,
                event_time,
                1,
                move(hdbpp_test::data_gen::generateData<Type>(traits)),
                move(hdbpp_test::data_gen::generateData<Type>(traits)),
                traits);
        }
    }

    conn.buffer(false);
    conn.disconnect();
}

BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_BOOLEAN, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_SHORT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_LONG, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_LONG64, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_FLOAT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_DOUBLE, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_UCHAR, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_USHORT, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_ULONG, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_ULONG64, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_STRING, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_STATE, Tango::SPECTRUM, 512)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_BOOLEAN, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_SHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_LONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_LONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_FLOAT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_DOUBLE, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_UCHAR, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_USHORT, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_ULONG, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_ULONG64, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_STRING, Tango::SCALAR)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(bmDbSinglePreparedStatementInsert, Tango::DEV_STATE, Tango::SCALAR)->Unit(benchmark::kMillisecond);