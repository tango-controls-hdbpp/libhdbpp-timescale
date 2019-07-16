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

#ifndef _COLUMN_CACHE_HPP
#define _COLUMN_CACHE_HPP

#include "LibUtils.hpp"
#include "PqxxExtension.hpp"
#include "QueryBuilder.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>

namespace hdbpp_internal
{
namespace pqxx_conn
{
    template<typename TValue, typename TRef>
    class ColumnCache
    {
    public:
        ColumnCache(std::shared_ptr<pqxx::connection> conn,
            const std::string &table_name,
            const std::string &column_name,
            const std::string &reference);

        // query if the reference has a value, if its not cached it will be
        // loaded from the database
        bool valueExists(const TRef &reference);

        // get the value associated with the reference, throws and exception if it does not
        // exist either in the cache or database. The caller can check valueExists()
        // before calling this function to know if its valid to attempt to return
        // the value
        TValue value(const TRef &reference);

        // cache a value in the internal maps
        void cacheValue(const TValue &value, const TRef &reference);

        // fetch all values from the database and cache them for future look up
        void fetchAll();

        // utility functions
        void clear() noexcept { _values.clear(); }
        int size() const noexcept { return _values.size(); }
        void print(std::ostream &os) const noexcept;

    private:
        // the database connection passed on construction
        std::shared_ptr<pqxx::connection> _conn;

        // store the cache parameters for debug purposes
        std::string _table_name;
        std::string _column_name;
        std::string _reference;

        // prepared query names for this cache, used to lookup
        // prepared statements
        std::string _fetch_all_query_name;
        std::string _fetch_id_query_name;

        // cache of values to a reference, the unordered map is not sorted
        // so we do not loose time on each insert having it resorted
        std::unordered_map<TRef, TValue> _values;

        // logging subsystem
        std::shared_ptr<spdlog::logger> _logger;
    };

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    ColumnCache<TValue, TRef>::ColumnCache(std::shared_ptr<pqxx::connection> conn,
        const std::string &table_name,
        const std::string &column_name,
        const std::string &reference) :
        _conn(conn),
        _table_name(table_name),
        _column_name(column_name),
        _reference(reference)
    {
        assert(_conn != nullptr);
        assert(!_table_name.empty());
        assert(!_column_name.empty());
        assert(!_reference.empty());

        // create the query names
        _fetch_all_query_name = _column_name + _table_name + _reference + "_all";
        _fetch_id_query_name = _column_name + _table_name + _reference + "_id";

        _logger = spdlog::get(LibLoggerName);

        _logger->trace("Cache created for table: {} using columns {}/{}", _table_name, _column_name, _reference);
    }

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    void ColumnCache<TValue, TRef>::fetchAll()
    {
        assert(_conn != nullptr);

        // clear the cache, since we are about to load the entire
        // table from the database
        clear();

        try
        {
            pqxx::perform([this]() {
                // request the entire table, since we will cache it fully
                pqxx::work tx {*(_conn.get()), FetchAllValues};

                if (!tx.prepared(_fetch_all_query_name).exists())
                {
                    tx.conn().prepare(_fetch_all_query_name,
                        QueryBuilder::fetchAllValuesQuery(_column_name, _table_name, _reference));

                    _logger->trace("Created prepared statement for: {}", _fetch_all_query_name);
                }

                auto result = tx.exec_prepared(_fetch_all_query_name);
                tx.commit();

                // load each value from the table into the cache
                for (const auto &row : result)
                    _values.insert({row[1].template as<TRef>(), row[0].template as<TValue>()});

                _logger->debug("Loaded: {} values into cache", _values.size());
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            string msg {"The database transaction failed. Unable to fetchAll for column: " + _column_name +
                " in table: " + _table_name + ". Error: " + ex.base().what()};

            _logger->error("Error: An unexpected error occurred when trying to run the database query");
            _logger->error("Caught error: \"{}\"", ex.base().what());
            _logger->error("Throwing storage error with message: \"{}\"", msg);

            Tango::Except::throw_exception("Storage Error", msg, LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    bool ColumnCache<TValue, TRef>::valueExists(const TRef &reference)
    {
        assert(_conn != nullptr);

        // search for the value in loaded values, if we get a hit then there is no
        // need to go to the database
        auto value_iter = _values.find(reference);

        if (value_iter != _values.end())
        {
            // found a cached value, return asap
            return true;
        }
        else
        {
            try
            {
                // the value is not loaded, so next step is to check the database
                return pqxx::perform([this, &reference]() {
                    // lookup the value
                    pqxx::work tx {(*_conn), FetchValue};

                    if (!tx.prepared(_fetch_id_query_name).exists())
                    {
                        tx.conn().prepare(
                            _fetch_id_query_name, QueryBuilder::fetchValueQuery(_column_name, _table_name, _reference));

                        _logger->trace("Created prepared statement for: {}", _fetch_id_query_name);
                    }

                    auto result = tx.exec_prepared(_fetch_id_query_name, reference);
                    tx.commit();

                    // no result is not an error, the value simply does not exist and its
                    // up to the caller to deal with the situation
                    if (!result.empty())
                    {
                        // if there is a result, there should be a single result, anything
                        // else is unexpected and an error
                        if (result.size() == 1)
                        {
                            auto value = result.at(0).at(0).template as<TValue>();
                            _values.insert({reference, value});

                            _logger->debug("Cached value: \'{} \' with reference: \'{}\'", value, reference);
                            return true;
                        }
                        else
                        {
                            throw pqxx::unexpected_rows(
                                "More than one row returned for value lookup. Expected just one.");
                        }
                    }

                    return false;
                });
            }
            catch (const pqxx::pqxx_exception &ex)
            {
                string msg {"The database transaction failed. Unable to query column: " + _column_name +
                    " in table: " + _table_name + ". Error: " + ex.base().what()};

                _logger->error("Error: An unexpected error occurred when trying to run the database query");
                _logger->error("Caught error: \"{}\"", ex.base().what());
                _logger->error("Throwing storage error with message: \"{}\"", msg);

                Tango::Except::throw_exception("Storage Error", msg, LOCATION_INFO);
            }
        }

        return false;
    }

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    TValue ColumnCache<TValue, TRef>::value(const TRef &reference)
    {
        assert(_conn != nullptr);

        // run a check on if the value exists, this will attempt to load the value if
        // its not in the cache. If this fails, return a blank optional
        if (!valueExists(reference))
        {
            // this is pretty fatal, we can not store information if it does not exist
            string msg {"Unable to find a value in either the cache or database for reference: " + reference};
            _logger->error("Error: {}", msg);
            Tango::Except::throw_exception("Storage Error", msg, LOCATION_INFO);
        }

        // value exists, find and return it
        return _values.at(reference);
    }

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    void ColumnCache<TValue, TRef>::cacheValue(const TValue &value, const TRef &reference)
    {
        assert(_conn != nullptr);

        // ensure the value is not already cached, and if it is, throw an exception for
        // the caller to deal with
        if (_values.find(reference) != _values.end())
        {
            _logger->warn("Value already exists in cache, not caching. Value: {} with reference: {}", value, reference);
            return;
        }

        _values.insert({reference, value});
        _logger->debug("Cached new value: {} with reference: {} by request", value, reference);
    }

    //=============================================================================
    //=============================================================================
    template<typename TValue, typename TRef>
    void ColumnCache<TValue, TRef>::print(std::ostream &os) const noexcept
    {
        os << "ColumnCache(size: " << _values.size() << ", "
           << "_table_name: " << _table_name << ", "
           << "_column_name: " << _column_name << ", "
           << "_reference: " << _reference << ")";
    }

} // namespace pqxx_conn
} // namespace hdbpp_internal

#endif // _COLUMN_CACHE_HPP
