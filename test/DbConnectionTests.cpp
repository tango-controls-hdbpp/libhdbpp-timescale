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
#include "LibUtils.hpp"
#include "QueryBuilder.hpp"
#include "TestHelpers.hpp"
#include "TimescaleSchema.hpp"
#include "catch2/catch.hpp"

#include <cfloat>
#include <locale>
#include <pqxx/pqxx>
#include <string>
#include <tuple>

using namespace std;
using namespace pqxx;
using namespace hdbpp_internal;
using namespace hdbpp_internal::pqxx_conn;
using namespace hdbpp_test;
using namespace hdbpp_test::psql_connection;
using namespace hdbpp_test::data_gen;

namespace pqxx_conn_test
{
// generic compare for most types
template<typename T>
bool compareData(T lhs, T rhs)
{
    // just to help debug
    REQUIRE(lhs == rhs);
    return lhs == rhs;
}

// float needs a specialised compare to ensure its close enough
template<>
bool compareData<float>(float lhs, float rhs)
{
    // Calculate the difference.
    float diff = fabs(lhs - rhs);
    lhs = fabs(lhs);
    rhs = fabs(rhs);

    // Find the largest
    float largest = (rhs > lhs) ? rhs : lhs;
    return (diff <= largest * 0.0001);
}

// double needs a specialised compare to ensure its close enough
template<>
bool compareData<double>(double lhs, double rhs)
{
    // Calculate the difference.
    double diff = fabs(lhs - rhs);
    lhs = fabs(lhs);
    rhs = fabs(rhs);

    // Find the largest
    double largest = (rhs > lhs) ? rhs : lhs;
    return (diff <= largest * 0.0001);
}

template<typename T>
bool compareVector(const vector<T> &lhs, const vector<T> &rhs)
{
    // just to help debug
    REQUIRE(lhs == rhs);
    return lhs == rhs;
}

template<>
bool compareVector<float>(const vector<float> &lhs, const vector<float> &rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (unsigned int i = 0; i < lhs.size(); i++)
        if (!compareData(lhs[i], rhs[i]))
            return false;

    return true;
}

template<>
bool compareVector<double>(const vector<double> &lhs, const vector<double> &rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (unsigned int i = 0; i < lhs.size(); i++)
        if (!compareData(lhs[i], rhs[i]))
            return false;

    return true;
}

class DbConnectionTestsFixture
{
private:
    DbConnection::DbStoreMethod _db_access = DbConnection::DbStoreMethod::PreparedStatement;
    std::unique_ptr<DbConnection> _test_conn;

    static std::unique_ptr<pqxx::connection> _verify_conn;
    static QueryBuilder _query_builder;

protected:
    void resetDbAccess(DbConnection::DbStoreMethod db_access)
    {
        _db_access = db_access;
        _test_conn.reset(nullptr);
    }

    DbConnection &testConn();
    pqxx::connection &verifyConn();

    void clearTables();
    void storeAttribute(const AttributeTraits &traits, unsigned int ttl = 0);
    string storeAttributeByTraits(const AttributeTraits &traits, unsigned int ttl = 0);

    template<Tango::CmdArgType Type>
    tuple<vector<typename TangoTypeTraits<Type>::type>, vector<typename TangoTypeTraits<Type>::type>>
        storeTestEventData(const string &att_name, const AttributeTraits &traits, int quality = Tango::ATTR_VALID);

    template<typename T>
    void checkStoreTestEventData(
        const string &att_name, const AttributeTraits &traits, const tuple<vector<T>, vector<T>> &data);

    QueryBuilder &queryBuilder() { return _query_builder; }
    vector<AttributeTraits> getTraits() const;
    vector<AttributeTraits> getTraitsImplemented() const;

public:
    DbConnectionTestsFixture() = default;
};

std::unique_ptr<pqxx::connection> DbConnectionTestsFixture::_verify_conn = std::unique_ptr<pqxx::connection> {};

//=============================================================================
//=============================================================================
DbConnection &DbConnectionTestsFixture::testConn()
{
    if (_test_conn == nullptr)
    {
        _test_conn = make_unique<DbConnection>(_db_access);
        REQUIRE_NOTHROW(_test_conn->connect(postgres_db::HdbppConnectionString));
    }

    return *_test_conn;
}

//=============================================================================
//=============================================================================
pqxx::connection &DbConnectionTestsFixture::verifyConn()
{
    if (_verify_conn == nullptr)
        _verify_conn = make_unique<pqxx::connection>(postgres_db::HdbppConnectionString);

    return *_verify_conn;
}

//=============================================================================
//=============================================================================
void DbConnectionTestsFixture::clearTables()
{
    vector<AttributeTraits> traits_array = getTraits();

    {
        string query = "TRUNCATE ";

        work tx {verifyConn()};

        for (auto &traits : traits_array)
        {
            query += QueryBuilder::tableName(traits);
            query += ",";
        }

        query += schema::ErrTableName + ",";
        query += schema::ParamTableName + ",";
        query += schema::HistoryEventTableName + ",";
        query += schema::HistoryTableName + ",";
        query += schema::ConfTableName + " RESTART IDENTITY";

        REQUIRE_NOTHROW(tx.exec(query));
        tx.commit();
    }
}

//=============================================================================
//=============================================================================
void DbConnectionTestsFixture::storeAttribute(const AttributeTraits &traits, unsigned int ttl)
{
    REQUIRE_NOTHROW(testConn().storeAttribute(attr_name::TestAttrFinalName,
        attr_name::TestAttrCs,
        attr_name::TestAttrDomain,
        attr_name::TestAttrFamily,
        attr_name::TestAttrMember,
        attr_name::TestAttrName,
        ttl,
        traits));
}

//=============================================================================
//=============================================================================
string DbConnectionTestsFixture::storeAttributeByTraits(const AttributeTraits &traits, unsigned int ttl)
{
    auto name = attr_name::TestAttrFinalName + "_" + tangoEnumToString(traits.type()) + "_" +
        tangoEnumToString(traits.writeType()) + "_" + tangoEnumToString(traits.formatType());

    REQUIRE_NOTHROW(testConn().storeAttribute(name,
        attr_name::TestAttrCs,
        attr_name::TestAttrDomain,
        attr_name::TestAttrFamily,
        attr_name::TestAttrMember,
        attr_name::TestAttrName,
        ttl,
        traits));

    return name;
}

//=============================================================================
//=============================================================================
vector<AttributeTraits> DbConnectionTestsFixture::getTraits() const
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
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM};

    // loop for every combination of type in Tango
    for (auto &type : types)
        for (auto &format : format_types)
            for (auto &write : write_types)
                traits_array.emplace_back(AttributeTraits {write, format, type});

    return traits_array;
}

//=============================================================================
//=============================================================================
vector<AttributeTraits> DbConnectionTestsFixture::getTraitsImplemented() const
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
    vector<Tango::AttrDataFormat> format_types {Tango::SCALAR, Tango::SPECTRUM};

    // loop for every combination of type in Tango
    for (auto &type : types)
        for (auto &format : format_types)
            for (auto &write : write_types)
                traits_array.emplace_back(AttributeTraits {write, format, type});

    return traits_array;
}

//=============================================================================
//=============================================================================
template<Tango::CmdArgType Type>
tuple<vector<typename TangoTypeTraits<Type>::type>, vector<typename TangoTypeTraits<Type>::type>>
    DbConnectionTestsFixture::storeTestEventData(const string &att_name, const AttributeTraits &traits, int quality)
{
    struct timeval tv
    {};
    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    auto r = generateData<Type>(traits, !traits.hasReadData());
    auto w = generateData<Type>(traits, !traits.hasWriteData());

    // make a copy for the consistency check
    auto ret = make_tuple((*r), (*w));

    REQUIRE_NOTHROW(testConn().storeDataEvent(att_name, event_time, quality, move(r), move(w), traits));

    return ret;
}

//=============================================================================
//=============================================================================
template<typename T>
void DbConnectionTestsFixture::checkStoreTestEventData(
    const string &att_name, const AttributeTraits &traits, const tuple<vector<T>, vector<T>> &data)
{
    pqxx::work tx {verifyConn()};

    // get the attribute id
    auto attr_row(
        tx.exec1("SELECT * FROM " + schema::ConfTableName + " WHERE " + schema::ConfColName + "='" + att_name + "'"));

    // now get the last row stored
    auto data_row(tx.exec1("SELECT * FROM " + QueryBuilder::tableName(traits) + " WHERE " + schema::DatColId + "=" +
        to_string(attr_row.at(schema::ConfColId).as<int>()) + " " + " ORDER BY " + schema::DatColDataTime +
        " DESC LIMIT 1"));

    tx.commit();

    REQUIRE(data_row.at(schema::DatColId).as<int>() == attr_row.at(schema::ConfColId).as<int>());

    if (traits.isScalar() && traits.hasReadData())
    {
        REQUIRE(data_row.at(schema::DatColValueR).size() > 0);
        REQUIRE(compareData(data_row.at(schema::DatColValueR).as<T>(), get<0>(data)[0]) == true);
    }

    if (traits.isArray() && traits.hasReadData())
    {
        REQUIRE(data_row.at(schema::DatColValueR).size() > 0);
        REQUIRE(compareVector(data_row.at(schema::DatColValueR).as<vector<T>>(), get<0>(data)) == true);
    }

    if (traits.isScalar() && traits.hasWriteData())
    {
        REQUIRE(data_row.at(schema::DatColValueW).size() > 0);
        REQUIRE(compareData(data_row.at(schema::DatColValueW).as<T>(), get<1>(data)[0]) == true);
    }

    if (traits.isArray() && traits.hasWriteData())
    {
        REQUIRE(data_row.at(schema::DatColValueW).size() > 0);
        REQUIRE(compareVector(data_row.at(schema::DatColValueW).as<vector<T>>(), get<1>(data)) == true);
    }
}
}; // namespace pqxx_conn_test

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "The DbConnection class can open a valid connection to a postgresql node",
    "[db-access][hdbpp-db-access][db-connection]")
{
    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);
    REQUIRE_NOTHROW(conn.connect(postgres_db::ConnectionString));
    REQUIRE(conn.isOpen());
    REQUIRE_NOTHROW(conn.disconnect());
    REQUIRE(conn.isClosed());
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "The DbConnection class handles a bad connection attempts with an exception",
    "[db-access][hdbpp-db-access][db-connection]")
{
    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);
    REQUIRE_THROWS_AS(conn.connect("user=postgres password=password host=unknown"), Tango::DevFailed);
    REQUIRE_THROWS_AS(conn.connect("user=invalid password=password host=hdb1"), Tango::DevFailed);
    REQUIRE_THROWS_AS(conn.connect("user=postgres password=invalid host=hdb1"), Tango::DevFailed);
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing Attributes in the database succeeds",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

    REQUIRE_NOTHROW(clearTables());
    REQUIRE_NOTHROW(storeAttribute(traits, 99));

    {
        pqxx::work tx {verifyConn()};
        auto attr_row(tx.exec1("SELECT * FROM " + schema::ConfTableName));

        auto type_row(tx.exec1("SELECT " + schema::ConfTypeColTypeId + " FROM " + schema::ConfTypeTableName +
            " WHERE " + schema::ConfTypeColTypeNum + " = " + std::to_string(traits.type())));

        auto format_row(tx.exec1("SELECT " + schema::ConfFormatColFormatId + " FROM " + schema::ConfFormatTableName +
            " WHERE " + schema::ConfFormatColFormatNum + " = " + std::to_string(traits.formatType())));

        auto access_row(tx.exec1("SELECT " + schema::ConfWriteColWriteId + " FROM " + schema::ConfWriteTableName +
            " WHERE " + schema::ConfWriteColWriteNum + " = " + std::to_string(traits.writeType())));

        tx.commit();

        REQUIRE(attr_row.at(schema::ConfColName).as<string>() == attr_name::TestAttrFQDName);
        REQUIRE(attr_row.at(schema::ConfColCsName).as<string>() == attr_name::TestAttrCs);
        REQUIRE(attr_row.at(schema::ConfColDomain).as<string>() == attr_name::TestAttrDomain);
        REQUIRE(attr_row.at(schema::ConfColFamily).as<string>() == attr_name::TestAttrFamily);
        REQUIRE(attr_row.at(schema::ConfColMember).as<string>() == attr_name::TestAttrMember);
        REQUIRE(attr_row.at(schema::ConfColLastName).as<string>() == attr_name::TestAttrName);
        REQUIRE(attr_row.at(schema::ConfColTableName).as<string>() == QueryBuilder().tableName(traits));
        REQUIRE(attr_row.at(schema::ConfColTypeId).as<int>() == type_row.at(schema::ConfTypeColTypeId).as<int>());
        REQUIRE(attr_row.at(schema::ConfColTtl).as<int>() == 99);

        REQUIRE(attr_row.at(schema::ConfColFormatTypeId).as<int>() ==
            format_row.at(schema::ConfFormatColFormatId).as<int>());

        REQUIRE(
            attr_row.at(schema::ConfColWriteTypeId).as<int>() == access_row.at(schema::ConfWriteColWriteId).as<int>());
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing the same attribute in the database twice fails",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_NOTHROW(storeAttribute(traits));

    REQUIRE_THROWS_AS(testConn().storeAttribute(attr_name::TestAttrFinalName,
                          attr_name::TestAttrCs,
                          attr_name::TestAttrDomain,
                          attr_name::TestAttrFamily,
                          attr_name::TestAttrMember,
                          attr_name::TestAttrName,
                          0,
                          traits),
        Tango::DevFailed);

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing Attributes in a disconnected state",
    "[db-access][hdbpp-db-access][db-connection]")
{
    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};

    REQUIRE_THROWS_AS(conn.storeAttribute(attr_name::TestAttrFinalName,
                          attr_name::TestAttrCs,
                          attr_name::TestAttrDomain,
                          attr_name::TestAttrFamily,
                          attr_name::TestAttrMember,
                          attr_name::TestAttrName,
                          0,
                          traits),
        Tango::DevFailed);

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing Attributes in the database in uppercase",
    "[db-access][hdbpp-db-access][db-connection]")
{
    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_STRING};

    REQUIRE_NOTHROW(clearTables());

    auto param_to_upper = [](auto param) {
        locale loc;
        string tmp;

        for (string::size_type i = 0; i < param.length(); ++i)
            tmp += toupper(param[i], loc);

        return tmp;
    };

    testConn().storeAttribute(param_to_upper(attr_name::TestAttrFinalName),
        param_to_upper(attr_name::TestAttrCs),
        param_to_upper(attr_name::TestAttrDomain),
        param_to_upper(attr_name::TestAttrFamily),
        param_to_upper(attr_name::TestAttrMember),
        param_to_upper(attr_name::TestAttrName),
        100,
        traits);

    {
        pqxx::work tx {verifyConn()};
        auto attr_row(tx.exec1("SELECT * FROM " + schema::ConfTableName));

        auto type_row(tx.exec1("SELECT " + schema::ConfTypeColTypeId + " FROM " + schema::ConfTypeTableName +
            " WHERE " + schema::ConfTypeColTypeNum + " = " + std::to_string(traits.type())));

        auto format_row(tx.exec1("SELECT " + schema::ConfFormatColFormatId + " FROM " + schema::ConfFormatTableName +
            " WHERE " + schema::ConfFormatColFormatNum + " = " + std::to_string(traits.formatType())));

        auto access_row(tx.exec1("SELECT " + schema::ConfWriteColWriteId + " FROM " + schema::ConfWriteTableName +
            " WHERE " + schema::ConfWriteColWriteNum + " = " + std::to_string(traits.writeType())));

        tx.commit();

        REQUIRE(attr_row.at(schema::ConfColName).as<string>() == param_to_upper(attr_name::TestAttrFQDName));
        REQUIRE(attr_row.at(schema::ConfColCsName).as<string>() == param_to_upper(attr_name::TestAttrCs));
        REQUIRE(attr_row.at(schema::ConfColDomain).as<string>() == param_to_upper(attr_name::TestAttrDomain));
        REQUIRE(attr_row.at(schema::ConfColFamily).as<string>() == param_to_upper(attr_name::TestAttrFamily));
        REQUIRE(attr_row.at(schema::ConfColMember).as<string>() == param_to_upper(attr_name::TestAttrMember));
        REQUIRE(attr_row.at(schema::ConfColLastName).as<string>() == param_to_upper(attr_name::TestAttrName));
        REQUIRE(attr_row.at(schema::ConfColTableName).as<string>() == QueryBuilder().tableName(traits));

        REQUIRE(attr_row.at(schema::ConfColTypeId).as<int>() == type_row.at(schema::ConfTypeColTypeId).as<int>());

        REQUIRE(attr_row.at(schema::ConfColFormatTypeId).as<int>() ==
            format_row.at(schema::ConfFormatColFormatId).as<int>());

        REQUIRE(
            attr_row.at(schema::ConfColWriteTypeId).as<int>() == access_row.at(schema::ConfWriteColWriteId).as<int>());
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing a series of the same History Events in the database successfully",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_NOTHROW(storeAttribute(traits));

    REQUIRE_NOTHROW(testConn().storeHistoryEvent(attr_name::TestAttrFQDName, events::PauseEvent));

    {
        pqxx::work tx {verifyConn()};
        auto event_row(tx.exec1("SELECT * FROM " + schema::HistoryEventTableName));
        auto history_row(tx.exec1("SELECT * FROM " + schema::HistoryTableName));
        auto attr_row(tx.exec1("SELECT * FROM " + schema::ConfTableName));
        tx.commit();

        // check event type
        REQUIRE(event_row.at(schema::HistoryEventColEvent).as<string>() == events::PauseEvent);

        // check event id matches event table id
        REQUIRE(event_row.at(schema::HistoryEventColEventId).as<int>() ==
            history_row.at(schema::HistoryColEventId).as<int>());

        // check attribute id match
        REQUIRE(attr_row.at(schema::ConfColId).as<int>() == history_row.at(schema::HistoryColId).as<int>());
    }

    REQUIRE_NOTHROW(testConn().storeHistoryEvent(attr_name::TestAttrFQDName, events::PauseEvent));

    {
        pqxx::work tx {verifyConn()};
        auto event_result(tx.exec1("SELECT * FROM " + schema::HistoryEventTableName));
        auto history_row(tx.exec_n(2, "SELECT * FROM " + schema::HistoryTableName));
        auto attr_row(tx.exec1("SELECT * FROM " + schema::ConfTableName));
        tx.commit();

        REQUIRE(event_result.at(schema::HistoryEventColEvent).as<string>() == events::PauseEvent);

        // check event type
        for (const auto &row : history_row)
        {
            REQUIRE(attr_row.at(schema::ConfColId).as<int>() == row.at(schema::HistoryColId).as<int>());

            // check event id matches event table id
            REQUIRE(
                row.at(schema::HistoryColEventId).as<int>() == event_result.at(schema::HistoryColEventId).as<int>());
        }
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing a series of different History Events in the database successfully",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_NOTHROW(storeAttribute(traits));

    vector<string> events {events::StartEvent, events::PauseEvent};

    REQUIRE_NOTHROW(testConn().storeHistoryEvent(attr_name::TestAttrFQDName, events[0]));
    REQUIRE_NOTHROW(testConn().storeHistoryEvent(attr_name::TestAttrFQDName, events[1]));

    {
        pqxx::work tx {verifyConn()};
        auto result(tx.exec_n(2, "SELECT * FROM " + schema::HistoryEventTableName));
        tx.commit();

        int i = 0;

        // check event type
        for (auto row : result)
            REQUIRE(row.at(schema::HistoryEventColEvent).as<string>() == events[i++]);
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing History Events unrelated to any known Attribute",
    "[db-access][hdbpp-db-access][db-connection]")
{
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_THROWS_AS(testConn().storeHistoryEvent(attr_name::TestAttrFQDName, events::PauseEvent), Tango::DevFailed);
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing History Events in a disconnected state",
    "[db-access][hdbpp-db-access][db-connection]")
{
    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);
    REQUIRE_THROWS_AS(conn.storeHistoryEvent(attr_name::TestAttrFQDName, events::PauseEvent), Tango::DevFailed);
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing Parameter Events in the database",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_NOTHROW(storeAttribute(traits));

    REQUIRE_NOTHROW(testConn().storeParameterEvent(attr_name::TestAttrFinalName,
        event_time,
        attr_info::AttrInfoLabel,
        attr_info::AttrInfoEnumLabels,
        attr_info::AttrInfoUnit,
        attr_info::AttrInfoStandardUnit,
        attr_info::AttrInfoDisplayUnit,
        attr_info::AttrInfoFormat,
        attr_info::AttrInfoRel,
        attr_info::AttrInfoAbs,
        attr_info::AttrInfoPeriod,
        attr_info::AttrInfoDescription));

    {
        pqxx::work tx {verifyConn()};
        auto attr_row(tx.exec1("SELECT * FROM " + schema::ConfTableName));
        auto param_row(tx.exec1("SELECT * FROM " + schema::ParamTableName));
        tx.commit();

        // TODO check event time
        //REQUIRE(param_row.at(schema::ParamColEvTime).as<double>() == event_time);
        REQUIRE(param_row.at(schema::ParamColLabel).as<string>() == attr_info::AttrInfoLabel);
        // TODO check enum labels
        REQUIRE(param_row.at(schema::ParamColEnumLabels).as<vector<string>>() == attr_info::AttrInfoEnumLabels);
        REQUIRE(param_row.at(schema::ParamColUnit).as<string>() == attr_info::AttrInfoUnit);
        REQUIRE(param_row.at(schema::ParamColStandardUnit).as<string>() == attr_info::AttrInfoStandardUnit);
        REQUIRE(param_row.at(schema::ParamColDisplayUnit).as<string>() == attr_info::AttrInfoDisplayUnit);
        REQUIRE(param_row.at(schema::ParamColFormat).as<string>() == attr_info::AttrInfoFormat);
        REQUIRE(param_row.at(schema::ParamColArchiveRelChange).as<string>() == attr_info::AttrInfoRel);
        REQUIRE(param_row.at(schema::ParamColArchiveAbsChange).as<string>() == attr_info::AttrInfoAbs);
        REQUIRE(param_row.at(schema::ParamColArchivePeriod).as<string>() == attr_info::AttrInfoPeriod);
        REQUIRE(param_row.at(schema::ParamColDescription).as<string>() == attr_info::AttrInfoDescription);

        // check attribute id match
        REQUIRE(attr_row.at(schema::ConfColId).as<int>() == param_row.at(schema::ParamColId).as<int>());
    }

    REQUIRE_NOTHROW(testConn().storeParameterEvent(attr_name::TestAttrFinalName,
        event_time,
        attr_info::AttrInfoLabel,
        attr_info::AttrInfoEnumLabels,
        attr_info::AttrInfoUnit,
        attr_info::AttrInfoStandardUnit,
        attr_info::AttrInfoDisplayUnit,
        attr_info::AttrInfoFormat,
        attr_info::AttrInfoRel,
        attr_info::AttrInfoAbs,
        attr_info::AttrInfoPeriod,
        attr_info::AttrInfoDescription));

    {
        pqxx::work tx {verifyConn()};
        auto result(tx.exec_n(2, "SELECT * FROM " + schema::ParamTableName));
        tx.commit();

        REQUIRE(result.size() == 2);
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing Parameter Events in a disconnected state",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    DbConnection conn(DbConnection::DbStoreMethod::PreparedStatement);

    REQUIRE_THROWS_AS(conn.storeParameterEvent(attr_name::TestAttrFinalName,
                          event_time,
                          attr_info::AttrInfoLabel,
                          attr_info::AttrInfoEnumLabels,
                          attr_info::AttrInfoUnit,
                          attr_info::AttrInfoStandardUnit,
                          attr_info::AttrInfoDisplayUnit,
                          attr_info::AttrInfoFormat,
                          attr_info::AttrInfoRel,
                          attr_info::AttrInfoAbs,
                          attr_info::AttrInfoPeriod,
                          attr_info::AttrInfoDescription),
        Tango::DevFailed);

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing events which has no data",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    vector<DbConnection::DbStoreMethod> access_methods {
        DbConnection::DbStoreMethod::PreparedStatement, DbConnection::DbStoreMethod::InsertString};

    vector<Tango::AttrWriteType> write_types {Tango::READ, Tango::WRITE, Tango::READ_WRITE, Tango::READ_WITH_WRITE};

    for (auto access : access_methods)
    {
        resetDbAccess(access);
        REQUIRE_NOTHROW(clearTables());

        for (auto write_type : write_types)
        {
            AttributeTraits traits {write_type, Tango::SCALAR, Tango::DEV_DOUBLE};
            auto att_name = storeAttributeByTraits(traits);

            REQUIRE_NOTHROW(testConn().storeDataEvent(att_name,
                event_time,
                Tango::ATTR_VALID,
                move(make_unique<std::vector<double>>()),
                move(make_unique<std::vector<double>>()),
                traits));

            {
                pqxx::work tx {verifyConn()};

                string query = "SELECT * FROM ";
                query += schema::ConfTableName;
                query += " WHERE ";
                query += schema::ConfColName;
                query += "='";
                query += att_name;
                query += "'";

                // get the attribute id
                auto attr_row(tx.exec1(query));

                query = "SELECT * FROM ";
                query += QueryBuilder::tableName(traits);
                query += " WHERE ";
                query += schema::DatColId;
                query += "=";
                query += to_string(attr_row.at(schema::ConfColId).as<int>());
                query += " ";
                query += " ORDER BY ";
                query += schema::DatColDataTime;
                query += " DESC LIMIT 1";

                // now get the last row stored
                auto data_row(tx.exec1(query));

                tx.commit();

                REQUIRE(data_row.at(schema::DatColId).as<int>() == attr_row.at(schema::ConfColId).as<int>());

                if (traits.hasReadData())
                    REQUIRE(data_row.at(schema::DatColValueR).is_null() == true);

                if (traits.hasWriteData())
                    REQUIRE(data_row.at(schema::DatColValueW).is_null() == true);
            }
        }
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing event data for all Tango type combinations in the database (prepared statements)",
    "[db-access][hdbpp-db-access][db-connection]")
{
    auto traits_array = getTraitsImplemented();
    REQUIRE_NOTHROW(clearTables());
    resetDbAccess(DbConnection::DbStoreMethod::PreparedStatement);

    for (auto &traits : traits_array)
    {
        INFO("Inserting data for traits: " << traits);
        auto name = storeAttributeByTraits(traits);

        switch (traits.type())
        {
            case Tango::DEV_BOOLEAN:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_BOOLEAN>(name, traits));
                break;

            case Tango::DEV_SHORT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_SHORT>(name, traits));
                break;

            case Tango::DEV_LONG:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_LONG>(name, traits));
                break;

            case Tango::DEV_LONG64:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_LONG64>(name, traits));
                break;

            case Tango::DEV_FLOAT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_FLOAT>(name, traits));
                break;

            case Tango::DEV_DOUBLE:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_DOUBLE>(name, traits));
                break;

            case Tango::DEV_UCHAR:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_UCHAR>(name, traits));
                break;

            case Tango::DEV_USHORT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_USHORT>(name, traits));
                break;

            case Tango::DEV_ULONG:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_ULONG>(name, traits));
                break;

            case Tango::DEV_ULONG64:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_ULONG64>(name, traits));
                break;

            case Tango::DEV_STRING:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_STRING>(name, traits));
                break;

            case Tango::DEV_STATE:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_STATE>(name, traits));
                break;

            default: throw "Should not be here!";
        }
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing event data for all Tango type combinations in the database (insert strings)",
    "[db-access][hdbpp-db-access][db-connection]")
{
    auto traits_array = getTraitsImplemented();
    REQUIRE_NOTHROW(clearTables());
    resetDbAccess(DbConnection::DbStoreMethod::InsertString);

    for (auto &traits : traits_array)
    {
        INFO("Inserting data for traits: " << traits);
        auto name = storeAttributeByTraits(traits);

        switch (traits.type())
        {
            case Tango::DEV_BOOLEAN:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_BOOLEAN>(name, traits));
                break;

            case Tango::DEV_SHORT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_SHORT>(name, traits));
                break;

            case Tango::DEV_LONG:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_LONG>(name, traits));
                break;

            case Tango::DEV_LONG64:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_LONG64>(name, traits));
                break;

            case Tango::DEV_FLOAT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_FLOAT>(name, traits));
                break;

            case Tango::DEV_DOUBLE:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_DOUBLE>(name, traits));
                break;

            case Tango::DEV_UCHAR:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_UCHAR>(name, traits));
                break;

            case Tango::DEV_USHORT:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_USHORT>(name, traits));
                break;

            case Tango::DEV_ULONG:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_ULONG>(name, traits));
                break;

            case Tango::DEV_ULONG64:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_ULONG64>(name, traits));
                break;

            case Tango::DEV_STRING:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_STRING>(name, traits));
                break;

            case Tango::DEV_STATE:
                checkStoreTestEventData(name, traits, storeTestEventData<Tango::DEV_STATE>(name, traits));
                break;

            default: throw "Should not be here!";
        }
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing complex arrays of strings containing postgres escape characters",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    string s1 = "test brackets } {} with comma,";
    string s2 = "quotes '' and commas, and 'quoted, comma', escaped \"double quote\"";
    string s3 = R"(test two slash \ test four slash \\)";
    string s4 = "line feed \n and return \r";

    auto value_r = std::make_unique<std::vector<std::string>>();
    value_r->push_back(s1);
    value_r->push_back(s2);
    value_r->push_back(s3);
    value_r->push_back(s4);

    auto value_w = std::make_unique<std::vector<std::string>>();
    value_w->push_back(s1);
    value_w->push_back(s2);
    value_w->push_back(s3);
    value_w->push_back(s4);

    auto original_values = make_tuple((*value_r), (*value_w));

    AttributeTraits traits {Tango::READ_WRITE, Tango::SPECTRUM, Tango::DEV_STRING};
    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);

    // this only ever stores by insert string, so no need to test prepared statements.
    REQUIRE_NOTHROW(
        testConn().storeDataEvent(name, event_time, Tango::ATTR_VALID, move(value_r), move(value_w), traits));

    checkStoreTestEventData(name, traits, original_values);

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing complex strings containing postgres escape characters",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    vector<string> values = {"test brackets } {} with comma,",
        "quotes '' and commas, and 'quoted, comma', escaped \"double quote\"",
        R"(test two slash \ test four slash \\)",
        "line feed \n and return \r"};

    AttributeTraits traits {Tango::READ_WRITE, Tango::SCALAR, Tango::DEV_STRING};

    vector<DbConnection::DbStoreMethod> access_methods {
        DbConnection::DbStoreMethod::PreparedStatement, DbConnection::DbStoreMethod::InsertString};

    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);

    for (auto access : access_methods)
    {
        resetDbAccess(access);

        for (auto &str : values)
        {
            gettimeofday(&tv, nullptr);
            double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

            auto value_r = std::make_unique<std::vector<std::string>>();
            value_r->push_back(str);

            auto value_w = std::make_unique<std::vector<std::string>>();
            value_w->push_back(str);

            auto original_values = make_tuple((*value_r), (*value_w));

            REQUIRE_NOTHROW(
                testConn().storeDataEvent(name, event_time, Tango::ATTR_VALID, move(value_r), move(value_w), traits));

            checkStoreTestEventData(name, traits, original_values);
        }
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing data events in a disconnected state",
    "[db-access][hdbpp-db-access][db-connection]")
{
    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);

    testConn().disconnect();

    REQUIRE_THROWS_AS(testConn().storeDataEvent(name,
                          event_time,
                          Tango::ATTR_VALID,
                          move(make_unique<std::vector<double>>()),
                          move(make_unique<std::vector<double>>()),
                          traits),
        Tango::DevFailed);

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Storing data events as errors",
    "[db-access][hdbpp-db-access][db-connection]")
{
    string error_msg = "A Test Error, 'Message'";

    struct timeval tv
    {};

    gettimeofday(&tv, nullptr);
    double event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);

    REQUIRE_NOTHROW(testConn().storeDataEventError(name, event_time, Tango::ATTR_VALID, error_msg, traits));

    {
        pqxx::work tx {verifyConn()};

        string query = "SELECT * FROM ";
        query += schema::ConfTableName;
        query += " WHERE ";
        query += schema::ConfColName;
        query += "='";
        query += name;
        query += "'";

        // get the attribute id
        pqxx::row attr_row;
        REQUIRE_NOTHROW(attr_row = tx.exec1(query));

        query = "SELECT * FROM ";
        query += QueryBuilder::tableName(traits);
        query += " WHERE ";
        query += schema::DatColId;
        query += "=";
        query += to_string(attr_row.at(schema::ConfColId).as<int>());
        query += " ";
        query += " ORDER BY ";
        query += schema::DatColDataTime;
        query += " DESC LIMIT 1";

        // now get the last row stored
        pqxx::row data_row;
        REQUIRE_NOTHROW(data_row = tx.exec1(query));

        query = "SELECT * FROM ";
        query += schema::ErrTableName;
        query += " WHERE ";
        query += schema::ErrColId + "=";
        query += to_string(data_row.at(schema::DatColErrorDescId).as<int>());

        pqxx::row error_row;
        REQUIRE_NOTHROW(error_row = tx.exec1(query));

        tx.commit();

        REQUIRE(error_row.at(schema::ErrColErrorDesc).as<string>() == error_msg);
    }

    gettimeofday(&tv, nullptr);
    event_time = tv.tv_sec + tv.tv_usec / 1.0e6;

    REQUIRE_NOTHROW(testConn().storeDataEventError(name, event_time, Tango::ATTR_VALID, error_msg, traits));

    {
        pqxx::work tx {verifyConn()};

        string query = "SELECT * FROM ";
        query += schema::ConfTableName;
        query += " WHERE ";
        query += schema::ConfColName;
        query += "='";
        query += name;
        query += "'";

        // get the attribute id
        pqxx::row attr_row;
        REQUIRE_NOTHROW(attr_row = tx.exec1(query));

        query = "SELECT * FROM ";
        query += QueryBuilder::tableName(traits);
        query += " WHERE ";
        query += schema::DatColId;
        query += "=";
        query += to_string(attr_row.at(schema::ConfColId).as<int>());
        query += " ";
        query += " ORDER BY ";
        query += schema::DatColDataTime;
        query += " DESC LIMIT 2";

        // now get the last row stored
        pqxx::result data_result;
        REQUIRE_NOTHROW(data_result = tx.exec(query));
        tx.commit();

        REQUIRE(data_result[0].at(schema::DatColErrorDescId).as<int>() ==
            data_result[1].at(schema::DatColErrorDescId).as<int>());
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Fetching the last history event after it has just been stored",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);
    REQUIRE_NOTHROW(testConn().storeHistoryEvent(name, events::PauseEvent));
    string event;
    REQUIRE_NOTHROW(event = testConn().fetchLastHistoryEvent(name));
    REQUIRE(event == events::PauseEvent);
    REQUIRE_NOTHROW(testConn().storeHistoryEvent(name, events::StartEvent));
    REQUIRE_NOTHROW(event = testConn().fetchLastHistoryEvent(name));
    REQUIRE(event == events::StartEvent);
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "When no history events have been stored, no error is thrown requesting the last event",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    storeAttribute(traits);
    string event;
    REQUIRE_NOTHROW(event = testConn().fetchLastHistoryEvent(attr_name::TestAttrFQDName));
    REQUIRE(event.empty());
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "The archive state of an attribute can be determined by fetchAttributeArchived()",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE(!testConn().fetchAttributeArchived(attr_name::TestAttrFQDName));
    storeAttribute(traits);
    REQUIRE(testConn().fetchAttributeArchived(attr_name::TestAttrFQDName));
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "fetchAttributeTraits() throws an exception when the attribute is not archived",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_THROWS(testConn().fetchAttributeTraits(attr_name::TestAttrFQDName));
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "The type traits of an archived attribute can be returned by fetchAttributeTraits()",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    storeAttribute(traits);
    REQUIRE(testConn().fetchAttributeTraits(attr_name::TestAttrFQDName) == traits);
    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "Upating an attribute ttl in the database",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    unsigned int new_ttl = 100;

    REQUIRE_NOTHROW(clearTables());
    auto name = storeAttributeByTraits(traits);
    REQUIRE_NOTHROW(testConn().storeAttributeTtl(name, new_ttl));

    {
        pqxx::work tx {verifyConn()};

        string query = "SELECT ttl FROM ";
        query += schema::ConfTableName;
        query += " WHERE ";
        query += schema::ConfColName;
        query += "='";
        query += name;
        query += "'";

        // get the attribute ttl
        pqxx::row attr_row;
        REQUIRE_NOTHROW(attr_row = tx.exec1(query));
        tx.commit();

        REQUIRE(attr_row.at(0).as<unsigned int>() == new_ttl);
    }

    SUCCEED("Passed");
}

TEST_CASE_METHOD(pqxx_conn_test::DbConnectionTestsFixture,
    "storeAttributeTtl() throws an exception when the attribute is not archived",
    "[db-access][hdbpp-db-access][db-connection]")
{
    AttributeTraits traits {Tango::READ, Tango::SCALAR, Tango::DEV_DOUBLE};
    REQUIRE_NOTHROW(clearTables());
    REQUIRE_THROWS(testConn().storeAttributeTtl(attr_name::TestAttrFQDName, 100));
    SUCCEED("Passed");
}
