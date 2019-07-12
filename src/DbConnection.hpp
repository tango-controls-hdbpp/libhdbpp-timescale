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

#ifndef _PSQL_CONNECTION_HPP
#define _PSQL_CONNECTION_HPP

#include "AttributeTraits.hpp"
#include "ColumnCache.hpp"
#include "ConnectionBase.hpp"
#include "HdbppTxFactory.hpp"
#include "QueryBuilder.hpp"
#include "TimescaleSchema.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>

namespace hdbpp
{
namespace pqxx_conn
{
    // The DbConnection represents a direct connection to a database, in this case
    // postgresql. The API is fixed by the transaction classes usage and CRTP
    class DbConnection : public ConnectionBase, public HdbppTxFactory<DbConnection>
    {
    public:
        // TODO add options fields - json string
        // TODO add error feedback
        // TODO add fetch DataType function

        DbConnection();
        virtual ~DbConnection() {}

        // connection API
        void connect(const string &connect_string) override;
        void disconnect() override;
        bool isOpen() const noexcept override { return _connected; }
        bool isClosed() const noexcept override { return !isOpen(); }

        // storage API

        // store a new attribute and its conf data into the database
        void storeAttribute(const std::string &full_attr_name,
            const std::string &control_system,
            const std::string &att_domain,
            const std::string &att_family,
            const std::string &att_member,
            const std::string &att_name,
            const AttributeTraits &traits);

        // store a new history event in the database
        void storeHistoryEvent(const std::string &full_attr_name, const std::string &event);

        // store a parameter event in the database
        void storeParameterEvent(const std::string &full_attr_name,
            double event_time,
            const std::string &label,
            const std::string &unit,
            const std::string &standard_unit,
            const std::string &display_unit,
            const std::string &format,
            const std::string &archive_rel_change,
            const std::string &archive_abs_change,
            const std::string &archive_period,
            const std::string &description);

        // this function can store the event data for all the supported
        // tango types. The data is passed in a unique pointer so the function
        // can take ownership of the data.
        template<typename T>
        void storeDataEvent(const std::string &full_attr_name,
            double event_time,
            int quality,
            std::unique_ptr<vector<T>> value_r,
            std::unique_ptr<vector<T>> value_w,
            const AttributeTraits &traits);

        // store a data error event in the data tables
        void storeDataEventError(const std::string &full_attr_name,
            double event_time,
            int quality,
            const std::string &error_msg,
            const AttributeTraits &traits);

        // fetch API

        // get the last history event for the given attribute
        std::string fetchLastHistoryEvent(const std::string &full_attr_name);

        // check if the given attribute is stored in the database
        bool fetchAttributeArchived(const std::string &full_attr_name);

        // get the AttributeTraits of an attribute in the database
        AttributeTraits fetchAttributeTraits(const std::string &full_attr_name);

    private:
        void storeEvent(const std::string &full_attr_name, const std::string &event);
        void storeErrorMsg(const std::string &full_attr_name, const std::string &error_msg);

        void checkAttributeExists(const std::string &full_attr_name, const std::string &location);
        void checkConnection(const std::string &location);

        void handlePqxxError(
            const std::string &msg, const std::string &what, const std::string &query, const std::string &location);

        // this object builds and caches queries for the database
        QueryBuilder _query_builder;

        // keep a copy of the connection string for connect/reconnect purposes
        std::string _connection_string;

        // this is the connection to the database, wrapped in a shared pointer so it
        // can be easily shared between several classes.
        std::shared_ptr<pqxx::connection> _conn;

        // cache the state to save asking the actual connection over and over
        bool _connected = false;

        // cache some database entries to speed up lookup, for example
        // various string ids and type ids

        // map the attribute database id to its name
        std::unique_ptr<ColumnCache<int, std::string>> _conf_id_cache;

        // cache the some id's for quick lookup
        std::unique_ptr<ColumnCache<int, std::string>> _error_desc_id_cache;
        std::unique_ptr<ColumnCache<int, std::string>> _event_id_cache;
        std::unique_ptr<ColumnCache<int, int>> _type_id_cache;

        // logging subsystem
        std::shared_ptr<spdlog::logger> _logger;
    };
} // namespace pqxx_conn
} // namespace hdbpp

// include the template implementations for DbConnection
#include "DbConnection.tpp"

#endif // _PSQL_CONNECTION_HPP
