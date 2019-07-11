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

namespace hdbpp
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
    const string &QueryBuilder::storeAttributeQuery()
    {
        // clang-format off
        static string query =
            "INSERT INTO " + CONF_TABLE_NAME + " (" +
                CONF_COL_NAME + "," +
                CONF_COL_TYPE_ID + "," +
                CONF_COL_FORMAT_TYPE_ID + "," +
                CONF_COL_WRITE_TYPE_ID + "," +
                CONF_COL_TABLE_NAME + "," +
                CONF_COL_CS_NAME + "," + 
                CONF_COL_DOMAIN + "," +
                CONF_COL_FAMILY + "," +
                CONF_COL_MEMBER + "," +
                CONF_COL_LAST_NAME + "," + 
                CONF_COL_HIDE + ") (" +
                "SELECT " + 
                    "$1," + 
                    CONF_TYPE_COL_TYPE_ID + "," + 
                    CONF_FORMAT_COL_FORMAT_ID + "," + 
                    CONF_WRITE_COL_WRITE_ID + 
                    ",$2,$3,$4,$5,$6,$7,$8 " +
                "FROM " + 
                    CONF_TYPE_TABLE_NAME + ", " +
                    CONF_FORMAT_TABLE_NAME + ", " +
                    CONF_WRITE_TABLE_NAME + " " +
                "WHERE " + CONF_TYPE_TABLE_NAME + "." + CONF_TYPE_COL_TYPE_NUM + " = $9 " + 
                "AND " + CONF_FORMAT_TABLE_NAME + "." + CONF_FORMAT_COL_FORMAT_NUM + " = $10 " + 
                "AND " + CONF_WRITE_TABLE_NAME + "." + CONF_WRITE_COL_WRITE_NUM + " = $11) " +
                "RETURNING " + CONF_COL_ID;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeHistoryStringQuery()
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + HISTORY_EVENT_TABLE_NAME + " (" +
                HISTORY_EVENT_COL_EVENT + 
                ") VALUES ($1) RETURNING " + HISTORY_EVENT_COL_EVENT_ID;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeHistoryEventQuery()
    {
        // clang-format off
        static string query =
            "INSERT INTO " + HISTORY_TABLE_NAME + " (" + 
                HISTORY_COL_ID + "," +
                HISTORY_COL_EVENT_ID + "," +
                HISTORY_COL_TIME + ") " +
                "SELECT " +
                    "$1," + HISTORY_EVENT_COL_EVENT_ID + ",CURRENT_TIMESTAMP(6)" +
                " FROM " + HISTORY_EVENT_TABLE_NAME +
                " WHERE " + HISTORY_EVENT_COL_EVENT + " = $2";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeParameterEventQuery()
    {
        // clang-format off
        static string query =
            "INSERT INTO " +
            PARAM_TABLE_NAME + " (" +
            PARAM_COL_ID + "," +
            PARAM_COL_EV_TIME + "," +
            PARAM_COL_LABEL + "," +
            PARAM_COL_UNIT + "," +
            PARAM_COL_STANDARDUNIT + "," +
            PARAM_COL_DISPLAYUNIT + "," +
            PARAM_COL_FORMAT + "," +
            PARAM_COL_ARCHIVERELCHANGE + "," +
            PARAM_COL_ARCHIVEABSCHANGE + "," +
            PARAM_COL_ARCHIVEPERIOD + "," +
            PARAM_COL_DESCRIPTION + ") " +
            "VALUES ($1, TO_TIMESTAMP($2), $3, $4, $5, $6, $7, $8, $9, $10, $11)";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeDataEventErrorQuery(const AttributeTraits &traits)
    {
        // search the cache for a previous entry
        auto result = _data_event_error_queries.find(traits);

        if (result == _data_event_error_queries.end())
        {
            auto param_number = 0;
            auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + DAT_COL_ID + "," + DAT_COL_DATA_TIME;

            // split to ensure increments are in the correct order
            query = query + "," + DAT_COL_QUALITY + "," + DAT_COL_ERROR_DESC_ID + ") VALUES ($" +
                to_string(++param_number);

            query = query + ",TO_TIMESTAMP($" + to_string(++param_number) + ")";

            query = query + "," + "$" + to_string(++param_number);
            query = query + "," + "$" + to_string(++param_number) + ")";

            // cache the query string against the traits
            _data_event_error_queries.emplace(traits, query);

            _logger->debug("Built new data event error query and cached it against traits: {}", traits);
            _logger->debug("New data event error query is: {}", query);

            // now return it (must dereference the map again to get the static version)
            return _data_event_error_queries[traits];
        }

        // return the previously cached example
        return result->second;
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::storeErrorQuery()
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + ERR_TABLE_NAME + " (" +
                ERR_COL_ERROR_DESC + ") VALUES ($1) RETURNING " + ERR_COL_ID;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const string QueryBuilder::fetchAllValuesQuery(
        const string &column_name, const string &table_name, const string &reference)
    {
        return "SELECT " + column_name + ", " + reference + " " + "FROM " + table_name;
    }

    //=============================================================================
    //=============================================================================
    const string QueryBuilder::fetchValueQuery(
        const string &column_name, const string &table_name, const string &reference)
    {
        return "SELECT " + column_name + " " + "FROM " + table_name + " WHERE " + reference + "=$1";
    }

    //=============================================================================
    //=============================================================================
    const string &QueryBuilder::fetchLastHistoryEventQuery()
    {
        // clang-format off
        static string query = 
            "SELECT " + HISTORY_EVENT_COL_EVENT +
                " FROM " + HISTORY_TABLE_NAME +
                " JOIN " + HISTORY_EVENT_TABLE_NAME +
                " ON " + HISTORY_EVENT_TABLE_NAME + "." + 
                    HISTORY_EVENT_COL_EVENT_ID + "=" + HISTORY_TABLE_NAME + "." + HISTORY_COL_EVENT_ID +
                " WHERE " + HISTORY_COL_ID + " =$1" +
                " ORDER BY " + HISTORY_COL_TIME + " DESC LIMIT 1";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    const std::string &QueryBuilder::fetchAttributeTraitsQuery()
    {
        // clang-format off
        static string query = 
            "SELECT " + 
                CONF_TYPE_COL_TYPE_NUM + "," +
                CONF_FORMAT_COL_FORMAT_NUM + "," +
                CONF_WRITE_COL_WRITE_NUM + " " +
            "FROM " +
	            CONF_TYPE_TABLE_NAME + " t," +
                CONF_FORMAT_TABLE_NAME + " f," + 
                CONF_WRITE_TABLE_NAME + " w, " + 
                "(SELECT " + 
                    CONF_COL_TYPE_ID + "," + 
                    CONF_COL_FORMAT_TYPE_ID + "," + 
                    CONF_COL_WRITE_TYPE_ID + " " + 
                "FROM " + CONF_TABLE_NAME + " WHERE " + CONF_COL_NAME + "=$1) AS tmp " + 
            "WHERE " +
	            "t." + CONF_COL_TYPE_ID + "=tmp." + CONF_COL_TYPE_ID + " " +
            "AND " +  
            	"f." + CONF_COL_FORMAT_TYPE_ID + "=tmp." + CONF_COL_FORMAT_TYPE_ID + " " +
            "AND " + 
	            "w." + CONF_COL_WRITE_TYPE_ID  + "=tmp." + CONF_COL_WRITE_TYPE_ID;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    string QueryBuilder::tableName(const AttributeTraits &traits)
    {
        return SCHEMA_TABLE_PREFIX +
            [&traits]() {
                switch (traits.formatType())
                {
                    case Tango::SCALAR: return TYPE_SCALAR;
                    case Tango::SPECTRUM: return TYPE_ARRAY;
                    case Tango::IMAGE: return TYPE_IMAGE;
                }

                return string("Unknown");
            }() +
            "_" + [&traits]() {
                switch (traits.type())
                {
                    case Tango::DEV_DOUBLE: return TYPE_DEV_DOUBLE;
                    case Tango::DEV_FLOAT: return TYPE_DEV_FLOAT;
                    case Tango::DEV_STRING: return TYPE_DEV_STRING;
                    case Tango::DEV_LONG: return TYPE_DEV_LONG;
                    case Tango::DEV_ULONG: return TYPE_DEV_ULONG;
                    case Tango::DEV_LONG64: return TYPE_DEV_LONG64;
                    case Tango::DEV_ULONG64: return TYPE_DEV_ULONG64;
                    case Tango::DEV_SHORT: return TYPE_DEV_SHORT;
                    case Tango::DEV_USHORT: return TYPE_DEV_USHORT;
                    case Tango::DEV_BOOLEAN: return TYPE_DEV_BOOLEAN;
                    case Tango::DEV_UCHAR: return TYPE_DEV_UCHAR;
                    case Tango::DEV_STATE: return TYPE_DEV_STATE;
                    case Tango::DEV_ENCODED: return TYPE_DEV_ENCODED;
                    case Tango::DEV_ENUM: return TYPE_DEV_ENUM;
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

            _logger->debug("New query name: {} cached against traits:", new_name, traits);
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
} // namespace hdbpp
