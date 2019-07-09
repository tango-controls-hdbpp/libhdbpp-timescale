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
#include "TimescaleSchema.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <string>

namespace std
{
// This is a custom compariator for the AttributeTraits class to allow its
// use as a key in a map. The traits class is used as a key to reference
// cached queries
template<>
struct less<hdbpp::AttributeTraits>
{
    bool operator()(const hdbpp::AttributeTraits &lhs, const hdbpp::AttributeTraits &rhs) const
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

namespace hdbpp
{
namespace pqxx_conn
{
    namespace query_utils
    {
        // This function generates the postgres cast for the event data insert
        // queries, it is specialized for all possible tango types in the source file
        template<typename T>
        std::string postgresCast(bool is_array);
    }; // namespace query_utils

    // these are used as transactions names for pqxx, some are used to as prepared
    // statement names, where the name required are simple. Anything that has to
    // generate a name uses an entry in QueryBuilder
    const string StoreAttribute = "StoreAttribute";
    const string StoreHistoryString = "StoreHistoryString";
    const string StoreHistoryEvent = "StoreHistoryEvent";
    const string StoreParameterEvent = "StoreParameterEvent";
    const string StoreDataEvent = "StoreDataEvent";
    const string StoreDataEventError = "StoreDataEventError";
    const string StoreErrorString = "StoreErrorString";
    const string FetchLastHistoryEvent = "FetchLastHistoryEvent";
    const string FetchAttributeTraits = "FetchAttributeTraits";
    const string FetchValue = "FetchKey";
    const string FetchAllValues = "FetchAllKeys";

    // Most of this class is static, its a simple query builder and cacher. The non-static
    // methods build and cache more complex query strings for event data.
    class QueryBuilder
    {
    public:
        QueryBuilder() { _logger = spdlog::get(LibLoggerName); }

        // Non-static methods

        // these builder functions cache the built query names, therefore they
        // are not static like the others
        const std::string &storeDataEventName(const AttributeTraits &traits);
        const std::string &storeDataEventErrorName(const AttributeTraits &traits);

        // like the query name functions, these cache data internally to speed up the
        // process of putting data int the db
        template<typename T>
        const std::string &storeDataEventQuery(const AttributeTraits &traits);

        const std::string &storeDataEventErrorQuery(const AttributeTraits &traits);
        std::string tableName(const AttributeTraits &traits);

        void print(std::ostream &os) const noexcept;

        // Static methods

        static const std::string &storeAttributeQuery();
        static const std::string &storeHistoryEventQuery();
        static const std::string &storeHistoryStringQuery();
        static const std::string &storeParameterEventQuery();
        static const std::string &storeErrorQuery();
        static const std::string &fetchLastHistoryEventQuery();
        static const std::string &fetchAttributeTraitsQuery();

        // these query strings are built each call, so are cached in the class
        // that requests them
        static const std::string fetchValueQuery(
            const std::string &column_name, const std::string &table_name, const std::string &reference);

        static const std::string fetchAllValuesQuery(
            const std::string &column_name, const std::string &table_name, const std::string &reference);

    private:
        // generic function to handle caching items into the cache maps
        const string &handleCache(
            std::map<AttributeTraits, std::string> &cache, const AttributeTraits &traits, const std::string &stub);

        // cached query names, these are built from the traits object
        std::map<AttributeTraits, std::string> _data_event_query_names;
        std::map<AttributeTraits, std::string> _data_event_error_query_names;

        // cached insert query strings built from the traits object
        map<AttributeTraits, std::string> _data_event_queries;
        map<AttributeTraits, std::string> _data_event_error_queries;

        // logging subsystem
        std::shared_ptr<spdlog::logger> _logger;
    };

    //=============================================================================
    //=============================================================================
    template<typename T>
    const string &QueryBuilder::storeDataEventQuery(const AttributeTraits &traits)
    {
        // search the cache for a previous entry
        auto result = _data_event_queries.find(traits);

        if (result == _data_event_queries.end())
        {
            // no cache hit, the insert statement must be built for this
            // attribute traits and then cached.
            auto param_number = 0;

            auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + DAT_COL_ID + "," + DAT_COL_DATA_TIME;

            if (traits.hasReadData())
                query = query + "," + DAT_COL_VALUE_R;

            if (traits.hasWriteData())
                query = query + "," + DAT_COL_VALUE_W;

            // split to ensure increments are in the correct order
            query = query + "," + DAT_COL_QUALITY + ") VALUES ($" + to_string(++param_number);
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

            _logger->debug("Built new data event query and cached it against traits: {}", traits);
            _logger->debug("New data event query is: {}", query);

            // now return it (must dereference the map again to get the static version)
            return _data_event_queries[traits];
        }

        // return the previously cached example
        return result->second;
    }
} // namespace pqxx_conn
} // namespace hdbpp
#endif // _QUERY_BUILDER_HPP
