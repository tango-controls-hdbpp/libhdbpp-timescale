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

#include <map>
#include <vector>

using namespace std;

namespace hdbpp_internal
{
namespace pqxx_conn
{
    namespace query_utils
    {
        // these specializations just return the correct postgres cast for the insert queries,
        // this is important for the custom types, since the library libpqxx and postgres will
        // not know how to store them.
        template<>
        std::string postgresCast<double>(bool is_array)
        {
            return is_array ? "float8[]" : "float8";
        }

        template<>
        std::string postgresCast<float>(bool is_array)
        {
            return is_array ? "float4[]" : "float4";
        }

        template<>
        std::string postgresCast<string>(bool is_array)
        {
            return is_array ? "text[]" : "text";
        }

        template<>
        std::string postgresCast<bool>(bool is_array)
        {
            return is_array ? "bool[]" : "bool";
        }

        template<>
        std::string postgresCast<int32_t>(bool is_array)
        {
            return is_array ? "int4[]" : "int4";
        }

        template<>
        std::string postgresCast<uint32_t>(bool is_array)
        {
            return is_array ? "ulong[]" : "ulong";
        }

        template<>
        std::string postgresCast<int64_t>(bool is_array)
        {
            return is_array ? "int8[]" : "int8";
        }

        template<>
        std::string postgresCast<uint64_t>(bool is_array)
        {
            return is_array ? "ulong64[]" : "ulong64";
        }

        template<>
        std::string postgresCast<int16_t>(bool is_array)
        {
            return is_array ? "int2[]" : "int2";
        }

        template<>
        std::string postgresCast<uint16_t>(bool is_array)
        {
            return is_array ? "ushort[]" : "ushort";
        }

        template<>
        std::string postgresCast<uint8_t>(bool is_array)
        {
            return is_array ? "uchar[]" : "uchar";
        }

        template<>
        std::string postgresCast<vector<uint8_t>>(bool is_array)
        {
            return is_array ? "bytea[]" : "bytea";
        }

        template<>
        std::string postgresCast<Tango::DevState>(bool is_array)
        {
            return is_array ? "int4[]" : "int4";
        }
    } // namespace query_utils

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeDataEventName(const AttributeTraits &traits)
    {
        // generic check and emplace for new items
        return handleCache(_data_event_query_names, traits, StoreDataEvent);
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeDataEventErrorName(const AttributeTraits &traits)
    {
        // generic check and emplace for new items
        return handleCache(_data_event_error_query_names, traits, StoreDataEventError);
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeAttributeStatement()
    {
        // clang-format off
        static string query =
            "INSERT INTO " + schema::ConfTableName + " (" +
                schema::ConfColName + "," +
                schema::ConfColTypeId + "," +
                schema::ConfColFormatTypeId + "," +
                schema::ConfColWriteTypeId + "," +
                schema::ConfColTableName + "," +
                schema::ConfColCsName + "," + 
                schema::ConfColDomain + "," +
                schema::ConfColFamily + "," +
                schema::ConfColMember + "," +
                schema::ConfColLastName + "," + 
                schema::ConfColHide + ") (" +
                "SELECT " + 
                    "$1," + 
                    schema::ConfTypeColTypeId + "," + 
                    schema::ConfFormatColFormatId + "," + 
                    schema::ConfWriteColWriteId + 
                    ",$2,$3,$4,$5,$6,$7,$8 " +
                "FROM " + 
                    schema::ConfTypeTableName + ", " +
                    schema::ConfFormatTableName + ", " +
                    schema::ConfWriteTableName + " " +
                "WHERE " + schema::ConfTypeTableName + "." + schema::ConfTypeColTypeNum + " = $9 " + 
                "AND " + schema::ConfFormatTableName + "." + schema::ConfFormatColFormatNum + " = $10 " + 
                "AND " + schema::ConfWriteTableName + "." + schema::ConfWriteColWriteNum + " = $11) " +
                "RETURNING " + schema::ConfColId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeHistoryStringStatement()
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + schema::HistoryEventTableName + " (" +
                schema::HistoryEventColEvent + 
                ") VALUES ($1) RETURNING " + schema::HistoryEventColEventId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeHistoryEventStatement()
    {
        // clang-format off
        static string query =
            "INSERT INTO " + schema::HistoryTableName + " (" + 
                schema::HistoryColId + "," +
                schema::HistoryColEventId + "," +
                schema::HistoryColTime + ") " +
                "SELECT " +
                    "$1," + schema::HistoryEventColEventId + ",CURRENT_TIMESTAMP(6)" +
                " FROM " + schema::HistoryEventTableName +
                " WHERE " + schema::HistoryEventColEvent + " = $2";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeParameterEventStatement()
    {
        // clang-format off
        static string query =
            "INSERT INTO " +
            schema::ParamTableName + " (" +
            schema::ParamColId + "," +
            schema::ParamColEvTime + "," +
            schema::ParamColLabel + "," +
            schema::ParamColUnit + "," +
            schema::ParamColStandardUnit + "," +
            schema::ParamColDisplayUnit + "," +
            schema::ParamColFormat + "," +
            schema::ParamColArchiveRelChange + "," +
            schema::ParamColArchiveAbsChange + "," +
            schema::ParamColArchivePeriod + "," +
            schema::ParamColDescription + ") " +
            "VALUES ($1, TO_TIMESTAMP($2), $3, $4, $5, $6, $7, $8, $9, $10, $11)";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeDataEventErrorStatement(const AttributeTraits &traits)
    {
        // search the cache for a previous entry
        auto result = _data_event_error_queries.find(traits);

        if (result == _data_event_error_queries.end())
        {
            auto param_number = 0;

            auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + schema::DatColId + "," +
                schema::DatColDataTime;

            // split to ensure increments are in the correct order
            query = query + "," + schema::DatColQuality + "," + schema::DatColErrorDescId + ") VALUES ($" +
                to_string(++param_number);

            query = query + ",TO_TIMESTAMP($" + to_string(++param_number) + ")";

            query = query + "," + "$" + to_string(++param_number);
            query = query + "," + "$" + to_string(++param_number) + ")";

            // cache the query string against the traits
            _data_event_error_queries.emplace(traits, query);

            spdlog::debug("Built new data event error query and cached it against traits: {}", traits);
            spdlog::debug("New data event error query is: {}", query);

            // now return it (must dereference the map again to get the static version)
            return _data_event_error_queries[traits];
        }

        // return the previously cached example
        return result->second;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeErrorStatement()
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + schema::ErrTableName + " (" +
                schema::ErrColErrorDesc + ") VALUES ($1) RETURNING " + schema::ErrColId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string QueryBuilder::fetchAllValuesStatement(
        const string &column_name, const string &table_name, const string &reference)
    {
        return "SELECT " + column_name + ", " + reference + " " + "FROM " + table_name;
    }

    //=============================================================================
    //=============================================================================
    const string QueryBuilder::fetchValueStatement(
        const string &column_name, const string &table_name, const string &reference)
    {
        return "SELECT " + column_name + " " + "FROM " + table_name + " WHERE " + reference + "=$1";
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::fetchLastHistoryEventStatement()
    {
        // clang-format off
        static string query = 
            "SELECT " + schema::HistoryEventColEvent +
                " FROM " + schema::HistoryTableName +
                " JOIN " + schema::HistoryEventTableName +
                " ON " + schema::HistoryEventTableName + "." + 
                    schema::HistoryEventColEventId + "=" + schema::HistoryTableName + "." + schema::HistoryColEventId +
                " WHERE " + schema::HistoryColId + " =$1" +
                " ORDER BY " + schema::HistoryColTime + " DESC LIMIT 1";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const std::string &QueryBuilder::fetchAttributeTraitsStatement()
    {
        // clang-format off
        static string query = 
            "SELECT " + 
                schema::ConfTypeColTypeNum + "," +
                schema::ConfFormatColFormatNum + "," +
                schema::ConfWriteColWriteNum + " " +
            "FROM " +
	            schema::ConfTypeTableName + " t," +
                schema::ConfFormatTableName + " f," + 
                schema::ConfWriteTableName + " w, " + 
                "(SELECT " + 
                    schema::ConfColTypeId + "," + 
                    schema::ConfColFormatTypeId + "," + 
                    schema::ConfColWriteTypeId + " " + 
                "FROM " + schema::ConfTableName + " WHERE " + schema::ConfColName + "=$1) AS tmp " + 
            "WHERE " +
	            "t." + schema::ConfColTypeId + "=tmp." + schema::ConfColTypeId + " " +
            "AND " +  
            	"f." + schema::ConfColFormatTypeId + "=tmp." + schema::ConfColFormatTypeId + " " +
            "AND " + 
	            "w." + schema::ConfColWriteTypeId  + "=tmp." + schema::ConfColWriteTypeId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    string QueryBuilder::tableName(const AttributeTraits &traits)
    {
        return schema::SchemaTablePrefix +
            [&traits]() {
                switch (traits.formatType())
                {
                    case Tango::SCALAR: return schema::TypeScalar;
                    case Tango::SPECTRUM: return schema::TypeArray;
                    case Tango::IMAGE: return schema::TypeImage;
                }

                return string("Unknown");
            }() +
            "_" + [&traits]() {
                switch (traits.type())
                {
                    case Tango::DEV_DOUBLE: return schema::TypeDevDouble;
                    case Tango::DEV_FLOAT: return schema::TypeDevFloat;
                    case Tango::DEV_STRING: return schema::TypeDevString;
                    case Tango::DEV_LONG: return schema::TypeDevLong;
                    case Tango::DEV_ULONG: return schema::TypeDevUlong;
                    case Tango::DEV_LONG64: return schema::TypeDevLong64;
                    case Tango::DEV_ULONG64: return schema::TypeDevUlong64;
                    case Tango::DEV_SHORT: return schema::TypeDevShort;
                    case Tango::DEV_USHORT: return schema::TypeDevUshort;
                    case Tango::DEV_BOOLEAN: return schema::TypeDevBoolean;
                    case Tango::DEV_UCHAR: return schema::TypeDevUchar;
                    case Tango::DEV_STATE: return schema::TypeDevState;
                    case Tango::DEV_ENCODED: return schema::TypeDevEncoded;
                    case Tango::DEV_ENUM: return schema::TypeDevEnum;
                }

                return string("Unknown");
            }();
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::handleCache(
        map<AttributeTraits, string> &cache, const AttributeTraits &traits, const string &stub)
    {
        auto result = cache.find(traits);

        if (result == cache.end())
        {
            // clang-format off
            string new_name = stub + 
                "_Write_" + to_string(traits.writeType()) +
                "_Format_" + to_string(traits.formatType()) + 
                "_Type_" + to_string(traits.type());
            // clang-format on

            // add to the cache for future hits
            cache.emplace(traits, new_name);

            spdlog::debug("New query name: {} cached against traits:", new_name, traits);
            return cache[traits];
        }

        return result->second;
    }

    //=============================================================================
    //=============================================================================
    void QueryBuilder::print(std::ostream &os) const noexcept
    {
        os << "QueryBuilder(cached "
           << "data_event: name/query " << _data_event_query_names.size() << "/" << _data_event_queries.size() << ", "
           << "data_event_error: name/query " << _data_event_error_query_names.size() << "/"
           << _data_event_error_queries.size() << ")";
    }
} // namespace pqxx_conn
} // namespace hdbpp_internal
