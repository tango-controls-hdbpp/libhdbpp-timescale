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

#ifndef _QUERY_BUILDER_HPP
#define _QUERY_BUILDER_HPP

#include "AttributeTraits.hpp"
#include "HdbppDefines.hpp"
#include "PqxxExtension.hpp"
#include "TimescaleSchema.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <string>

namespace std
{
// This is a custom comparator for the AttributeTraits class to allow its
// use as a key in a map. The traits class is used as a key to reference
// cached queries
template<>
struct less<hdbpp_internal::AttributeTraits>
{
    auto operator()(const hdbpp_internal::AttributeTraits &lhs, const hdbpp_internal::AttributeTraits &rhs) -> bool
    {
        auto a = lhs.type();
        auto b = lhs.writeType();
        auto c = lhs.formatType();
        auto x = rhs.type();
        auto y = rhs.writeType();
        auto z = rhs.formatType();
        return tie(a, b, c) < tie(x, y, z);
    }
};
} // namespace std

namespace hdbpp_internal
{
namespace pqxx_conn
{
    namespace query_utils
    {
        // This function generates the postgres cast for the event data insert
        // queries, it is specialized for all possible tango types
        template<typename T>
        auto postgresCast(bool is_array) -> std::string;

        // Convert the given data into a string suitable for storing in the database. These calls
        // are used to build the string version of the insert command, they are required since we
        // need to specialise for strings (to ensure we do not store escape characters) and bools
        // (which are in fact a bitfield internally and wont convert in the via the pqxx routines )
        template<typename T>
        struct DataToString
        {
            static auto run(const std::unique_ptr<std::vector<T>> &value, const AttributeTraits &traits) -> std::string
            {
                if (traits.isScalar())
                    return pqxx::to_string((*value)[0]);

                return "'" + pqxx::to_string(value) + "'";
            }
        };

        // Convert a vector<bool> to a postgres array
        template<>
        struct DataToString<bool>
        {
            static auto run(const std::unique_ptr<std::vector<bool>> &value, const AttributeTraits &traits) -> std::string
            {
                // a vector<bool> is not actually a vector<bool>, rather its some kind of bitfield. When
                // trying to return an element, we appear to get some kind of bitfield reference,
                // so we return the value to a local variable to remove the reference to the bitfield and
                // this ensure its actually a bool passed into the conversion framework
                if (traits.isScalar())
                {
                    bool v = (*value)[0];
                    return pqxx::to_string(v);
                }

                // handled by our own extensions in PqxxExtensions.hpp
                return "'" + pqxx::to_string(value) + "'";
            }
        };

        // This specialisation for strings uses the ARRAY syntax and dollar quoting to
        // ensure arrays of strings are stored without escape characters
        template<>
        struct DataToString<std::string>
        {
            static auto run(
                const std::unique_ptr<std::vector<std::string>> &value, const AttributeTraits &traits) -> std::string
            {
                // arrays of strings need both the ARRAY keywords and dollar escaping, this is so we
                // do not have to rely on the postgres escape functions that double and then store
                // escaped characters. This is a mess when extracting the array of strings.
                if (traits.isScalar())
                {
                    // use dollars to ensure it saves
                    return "$$" + pqxx::to_string((*value)[0]) + "$$";
                }

                auto iter = value->begin();
                std::string result = "ARRAY[";

                result = result + "$$" + pqxx::to_string((*iter)) + "$$";

                for (++iter; iter != value->end(); ++iter)
                {
                    result += ",";
                    result += "$$" + pqxx::to_string((*iter)) + "$$";
                }

                result += "]";
                return result;
            }
        };
    }; // namespace query_utils

    // these are used as transactions names for pqxx, some are used to as prepared
    // statement names, where the name required are simple. Anything that has to
    // generate a name uses an entry in QueryBuilder
    const string StoreAttribute = "StoreAttribute";
    const string StoreHistoryString = "StoreHistoryString";
    const string StoreHistoryEvent = "StoreHistoryEvent";
    const string StoreParameterEvent = "StoreParameterEvent";
    const string StoreDataEvent = "StoreDataEvent";
    const string StoreDataEvents = "StoreDataEvents";
    const string StoreDataEventError = "StoreDataEventError";
    const string StoreErrorString = "StoreErrorString";
    const string StoreTtl = "StoreTtl";
    const string FetchLastHistoryEvent = "FetchLastHistoryEvent";
    const string FetchAttributeTraits = "FetchAttributeTraits";
    const string FetchValue = "FetchKey";
    const string FetchAllValues = "FetchAllKeys";

    // Most of this class is static, its a simple query builder and cacher. The non-static
    // methods build and cache more complex query strings for event data.
    class QueryBuilder
    {
    public:
        // Static Prepared statement strings
        // these builder functions require no caching, so can be simple static
        // functions

        static auto tableName(const AttributeTraits &traits) -> std::string;
        static auto storeAttributeStatement() -> const std::string &;
        static auto storeHistoryEventStatement() -> const std::string &;
        static auto storeHistoryStringStatement() -> const std::string &;
        static auto storeParameterEventStatement() -> const std::string &;
        static auto storeErrorStatement() -> const std::string &;
        static auto storeTtlStatement() -> const std::string &;
        static auto fetchLastHistoryEventStatement() -> const std::string &;
        static auto fetchAttributeTraitsStatement() -> const std::string &;

        static auto fetchValueStatement(
            const std::string &column_name, const std::string &table_name, const std::string &reference) -> const std::string;

        static auto fetchAllValuesStatement(
            const std::string &column_name, const std::string &table_name, const std::string &reference) -> const std::string;

        // Non-static prepared statements
        // these builder functions cache the built queries, therefore they
        // are not static like the others sincethey require data storage

        auto storeDataEventName(const AttributeTraits &traits) -> const std::string &;
        auto storeDataEventErrorName(const AttributeTraits &traits) -> const std::string &;

        // Builds a prepared statement for the given traits, the statement is cached
        // internally to improve execution time
        template<typename T>
        auto storeDataEventStatement(const AttributeTraits &traits) -> const std::string &;

        // A variant of storeDataEventStatement that builds a string based on the
        // parameters, this is then passed back to the caller to be executed. No
        // internal caching, so its less efficient, but can be chained in a pipe
        // to batch data to the database.
        template<typename T>
        static auto storeDataEventString(const std::string &id,
            const std::string &event_time,
            const std::string &quality,
            const std::unique_ptr<vector<T>> &value_r,
            const std::unique_ptr<vector<T>> &value_w,
            const AttributeTraits &traits) -> std::string;

        // Builds a prepared statement for data event errors
        auto storeDataEventErrorStatement(const AttributeTraits &traits) -> const std::string &;

        // A vareint of storeDataEventErrorStatement that build and returns a string
        // instead of a prepared statement
        static auto storeDataEventErrorString(const std::string &id,
            const std::string &event_time,
            const std::string &quality,
            const std::string &err_id,
            const AttributeTraits &traits) -> std::string;

        // Utility
        void print(std::ostream &os) const noexcept;

    private:
        // generic function to handle caching items into the cache maps
        auto handleCache(
            std::map<AttributeTraits, std::string> &cache, const AttributeTraits &traits, const std::string &stub) -> const std::string &;

        // cached query names, these are built from the traits object
        std::map<AttributeTraits, std::string> _data_event_query_names;
        std::map<AttributeTraits, std::string> _data_event_error_query_names;

        // cached insert query strings built from the traits object
        std::map<AttributeTraits, std::string> _data_event_queries;
        std::map<AttributeTraits, std::string> _data_event_error_queries;
    };

    //=============================================================================
    //=============================================================================
    template<typename T>
    auto QueryBuilder::storeDataEventStatement(const AttributeTraits &traits) -> const std::string &
    {
        // search the cache for a previous entry
        auto result = _data_event_queries.find(traits);

        if (result == _data_event_queries.end())
        {
            // no cache hit, the insert statement must be built for this
            // attribute traits and then cached.
            auto param_number = 0;

            auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + schema::DatColId + "," +
                schema::DatColDataTime;

            if (traits.hasReadData())
                query = query + "," + schema::DatColValueR;

            if (traits.hasWriteData())
                query = query + "," + schema::DatColValueW;

            // split to ensure increments are in the correct order
            query = query + "," + schema::DatColQuality + ") VALUES ($" + to_string(++param_number);
            query = query + ",TO_TIMESTAMP($" + to_string(++param_number) + ")";

            // add the read parameter with cast
            if (traits.hasReadData())
                query = query + "," + "$" + to_string(++param_number) +
                    "::" + query_utils::postgresCast<T>(traits.isArray());

            // add the write parameter with cast
            if (traits.hasWriteData())
                query = query + "," + "$" + to_string(++param_number) +
                    "::" + query_utils::postgresCast<T>(traits.isArray());

            query = query + "," + "$" + to_string(++param_number) + ")";

            // cache the query string against the traits
            _data_event_queries.emplace(traits, query);

            spdlog::debug("Built new data event query and cached it against traits: {}", traits);
            spdlog::debug("New data event query is: {}", query);

            // now return it (must dereference the map again to get the static version)
            return _data_event_queries[traits];
        }

        // return the previously cached example
        return result->second;
    }

    template<typename T>
    auto QueryBuilder::storeDataEventString(const std::string &id,
        const std::string &event_time,
        const std::string &quality,
        const std::unique_ptr<vector<T>> &value_r,
        const std::unique_ptr<vector<T>> &value_w,
        const AttributeTraits &traits) -> std::string
    {
        auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + schema::DatColId + "," +
            schema::DatColDataTime;

        if (traits.hasReadData())
            query = query + "," + schema::DatColValueR;

        if (traits.hasWriteData())
            query = query + "," + schema::DatColValueW;

        // split to ensure increments are in the correct order
        query = query + "," + schema::DatColQuality + ") VALUES ('" + id + "'";
        query = query + ",TO_TIMESTAMP(" + event_time + ")";

        // add the read parameter with cast
        if (traits.hasReadData())
        {
            if (value_r->empty())
            {
                query = query + ",NULL";
            }
            else
            {
                query = query + "," + query_utils::DataToString<T>::run(value_r, traits) +
                    "::" + query_utils::postgresCast<T>(traits.isArray());
            }
        }

        // add the write parameter with cast
        if (traits.hasWriteData())
        {
            if (value_w->empty())
            {
                query = query + ",NULL";
            }
            else
            {
                query = query + "," + query_utils::DataToString<T>::run(value_w, traits) +
                    "::" + query_utils::postgresCast<T>(traits.isArray());
            }
        }

        query = query + "," + quality + ")";

        // now return the built query
        return query;
    }

} // namespace pqxx_conn
} // namespace hdbpp_internal
#endif // _QUERY_BUILDER_HPP
