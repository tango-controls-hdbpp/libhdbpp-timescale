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

#include "LibUtils.hpp"

#include <cassert>
#include <experimental/optional>
#include <iostream>

using namespace std;

namespace hdbpp_internal
{
namespace pqxx_conn
{
    //=============================================================================
    //=============================================================================
    DbConnection::DbConnection(DbStoreMethod db_store_method) : _db_store_method(db_store_method) {}

    //=============================================================================
    //=============================================================================
    void DbConnection::connect(const string &connect_string)
    {
        spdlog::info("Connecting to postgres database with string: \"{}\"", connect_string);

        // construct the database connection
        try
        {
            // disconnect existing connections
            if (_conn && _conn->is_open())
                _conn->disconnect();

            // the connection is wrapped as a shared pointer to help manage its
            // lifetime between objects
            _conn = make_shared<pqxx::connection>(connect_string);

            // mark the connected flag as true to cache this state
            _connected = true;
            spdlog::info("Connected to postgres successfully");
        }
        catch (const pqxx::broken_connection &ex)
        {
            string msg {"Failed to connect to database. Exception: "};
            msg += ex.what();

            spdlog::error("Error: Connecting to postgres database with connect string: \"{}\"", connect_string);
            spdlog::error("Caught error: \"{}\"", ex.what());
            spdlog::error("Throwing connection error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Connection Error", msg, LOCATION_INFO);
        }

        // now create and connect the cache objects to the database connection, this
        // will destroy any existing cache objects managed by the unique pointers
        _conf_id_cache = make_unique<ColumnCache<int, std::string>>(
            _conn, schema::ConfTableName, schema::ConfColId, schema::ConfColName);

        _error_desc_id_cache = make_unique<ColumnCache<int, std::string>>(
            _conn, schema::ErrTableName, schema::ErrColId, schema::ErrColErrorDesc);

        _event_id_cache = make_unique<ColumnCache<int, std::string>>(
            _conn, schema::HistoryEventTableName, schema::HistoryEventColEventId, schema::HistoryEventColEvent);
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::disconnect()
    {
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        _conf_id_cache->clear();
        _error_desc_id_cache->clear();
        _event_id_cache->clear();

        // disconnect as requested, this will stop access to all functions
        _conn->disconnect();

        // stop attempts to use the connection
        _connected = false;
        spdlog::debug("Disconnected from the postgres database");
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeAttribute(const string &full_attr_name,
        const string &control_system,
        const string &att_domain,
        const string &att_family,
        const string &att_member,
        const string &att_name,
        const AttributeTraits &traits)
    {
        assert(!full_attr_name.empty());
        assert(!control_system.empty());
        assert(!att_domain.empty());
        assert(!att_family.empty());
        assert(!att_member.empty());
        assert(!att_name.empty());
        assert(traits.isValid());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        spdlog::trace("Storing new attribute {} of type {}", full_attr_name, traits);

        checkConnection(LOCATION_INFO);

        // if the attribute has already been configured, then we can not add it again,
        // this is an error case
        if (_conf_id_cache->valueExists(full_attr_name))
        {
            string msg {
                "This attribute [" + full_attr_name + "] already exists in the database. Unable to add it again."};

            spdlog::error("Error: The attribute already exists in the database and can not be added again");
            spdlog::error("Attribute details. Name: {} traits: {}", full_attr_name, traits);
            spdlog::error("Throwing consistency error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Consistency Error", msg, LOCATION_INFO);
        }

        try
        {
            // create and perform a pqxx transaction
            auto conf_id = pqxx::perform([&, this]() {
                pqxx::work tx {(*_conn), StoreAttribute};

                if (!tx.prepared(StoreAttribute).exists())
                {
                    tx.conn().prepare(StoreAttribute, QueryBuilder::storeAttributeStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreAttribute);
                }

                // execute the statement with the expectation that we get a row back
                auto row = tx.exec_prepared1(StoreAttribute,
                    full_attr_name,
                    QueryBuilder::tableName(traits),
                    control_system,
                    att_domain,
                    att_family,
                    att_member,
                    att_name,
                    false,
                    static_cast<unsigned int>(traits.type()),
                    static_cast<unsigned int>(traits.formatType()),
                    static_cast<unsigned int>(traits.writeType()));

                tx.commit();

                // we should have a single row with a single result, this is the new attribute id,
                // return it so we can cache it
                return row.at(0).as<int>();
            });

            spdlog::debug("Stored new attribute {} of type {} with db id: {}", full_attr_name, traits, conf_id);

            // cache the new conf id for future use
            _conf_id_cache->cacheValue(conf_id, full_attr_name);
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] was not saved.",
                ex.base().what(),
                QueryBuilder::storeAttributeStatement(),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeHistoryEvent(const string &full_attr_name, const string &event)
    {
        assert(!full_attr_name.empty());
        assert(!event.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        spdlog::trace("Storing history event {} for attribute {}", event, full_attr_name);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        // now check if this event exists in the cache/table
        if (!_event_id_cache->valueExists(event))
            storeEvent(full_attr_name, event);

        if (!_event_id_cache->valueExists(event))
        {
            string msg {
                "The event [" + event + "] is missing in both the cache and database, this is an unrecoverable error."};

            spdlog::error(
                "Event found missing, this occurred when storing event: {} for attribute: {}", event, full_attr_name);

            spdlog::error("Throwing consistency error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Consistency Error", msg, LOCATION_INFO);
        }

        try
        {
            // create and perform a pqxx transaction
            pqxx::perform([&full_attr_name, &event, this]() {
                pqxx::work tx {(*_conn), StoreHistoryEvent};

                if (!tx.prepared(StoreHistoryEvent).exists())
                {
                    tx.conn().prepare(StoreHistoryEvent, QueryBuilder::storeHistoryEventStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreHistoryEvent);
                }

                // expect no result, this is an insert only query
                tx.exec_prepared0(StoreHistoryEvent, _conf_id_cache->value(full_attr_name), event);
                tx.commit();
            });

            spdlog::debug("Stored event {} and for attribute {}", event, full_attr_name);
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] event [" + event + "] was not saved.",
                ex.base().what(),
                QueryBuilder::storeHistoryEventStatement(),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeParameterEvent(const string &full_attr_name,
        double event_time,
        const string &label,
        const string &unit,
        const string &standard_unit,
        const string &display_unit,
        const string &format,
        const string &archive_rel_change,
        const string &archive_abs_change,
        const string &archive_period,
        const string &description)
    {
        assert(!full_attr_name.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        spdlog::trace("Storing parameter event for attribute {}", full_attr_name);

        auto check_parameter = [](auto &name, auto &value) {
            if (value.empty())
                spdlog::warn("Parameter {} is empty. Please set in the device server", name);
        };

        check_parameter("label", label);
        check_parameter("unit", unit);
        check_parameter("standard_unit", standard_unit);
        check_parameter("display_unit", display_unit);
        check_parameter("archive_rel_change", archive_rel_change);
        check_parameter("archive_abs_change", archive_abs_change);
        check_parameter("archive_period", archive_period);
        check_parameter("description", description);

        spdlog::trace("Parmater event data: event_time {}, label {}, unit {}, standard_unit {}, display_unit {}, "
                      "format {}, archive_rel_change {}, archive_abs_change {}, archive_period {}, description {}",
            event_time,
            label,
            unit,
            standard_unit,
            display_unit,
            format,
            archive_rel_change,
            archive_abs_change,
            archive_period,
            description);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        try
        {
            // create and perform a pqxx transaction
            pqxx::perform([&, this]() {
                pqxx::work tx {(*_conn), StoreParameterEvent};

                if (!tx.prepared(StoreParameterEvent).exists())
                {
                    tx.conn().prepare(StoreParameterEvent, QueryBuilder::storeParameterEventStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreParameterEvent);
                }

                // no result expected
                tx.exec_prepared0(StoreParameterEvent,
                    _conf_id_cache->value(full_attr_name),
                    event_time,
                    label,
                    unit,
                    standard_unit,
                    display_unit,
                    format,
                    archive_rel_change,
                    archive_abs_change,
                    archive_period,
                    description);

                tx.commit();
            });

            spdlog::debug("Stored parameter event and for attribute {}", full_attr_name);
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] parameter event was not saved.",
                ex.base().what(),
                QueryBuilder::storeParameterEventStatement(),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeDataEventError(const std::string &full_attr_name,
        double event_time,
        int quality,
        const std::string &error_msg,
        const AttributeTraits &traits)
    {
        assert(!full_attr_name.empty());
        assert(!error_msg.empty());
        assert(traits.isValid());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        spdlog::trace("Storing error message event for attribute {}. Quality: {}. Error message: \"{}\"",
            full_attr_name,
            quality,
            error_msg);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        // first ensure the error message has an id inm the database, otherwise
        // we can not store data against it
        if (!_error_desc_id_cache->valueExists(error_msg))
            storeErrorMsg(full_attr_name, error_msg);

        // double check it really exists....
        if (!_error_desc_id_cache->valueExists(error_msg))
        {
            string msg {"The error message [" + error_msg +
                "] is missing in both the cache and database, this is an unrecoverable error."};

            spdlog::error("Error message found missing, this occurred when storing msg: \"{}\" for attribute: {}",
                error_msg,
                full_attr_name);

            spdlog::error("Throwing consistency error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Consistency Error", msg, LOCATION_INFO);
        }

        try
        {
            // create and perform a pqxx transaction
            pqxx::perform([&, this]() {
                pqxx::work tx {(*_conn), StoreDataEventError};

                if (!tx.prepared(_query_builder.storeDataEventErrorName(traits)).exists())
                {
                    tx.conn().prepare(_query_builder.storeDataEventErrorName(traits),
                        _query_builder.storeDataEventErrorStatement(traits));

                    spdlog::trace("Created prepared statement for: {}", _query_builder.storeDataEventErrorName(traits));
                }

                // no result expected
                tx.exec_prepared0(_query_builder.storeDataEventErrorName(traits),
                    _conf_id_cache->value(full_attr_name),
                    event_time,
                    quality,
                    _error_desc_id_cache->value(error_msg));

                tx.commit();
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] error message [" + error_msg + "] was not saved.",
                ex.base().what(),
                _query_builder.storeDataEventErrorName(traits),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeAttributeTtl(const std::string &full_attr_name, unsigned int ttl)
    {
        assert(!full_attr_name.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);


        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        spdlog::trace("Setting ttl for attribute: {} to: {}", full_attr_name, ttl);

        try
        {
            // create and perform a pqxx transaction
            pqxx::perform([&, this]() {
                pqxx::work tx {(*_conn), StoreTtl};

                if (!tx.prepared(StoreTtl).exists())
                {
                    tx.conn().prepare(StoreTtl, QueryBuilder::storeTtlStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreTtl);
                }

                // no result expected
                tx.exec_prepared0(StoreTtl, ttl, _conf_id_cache->value(full_attr_name));
                tx.commit();
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] ttl [" + std::to_string(ttl) + "] was not saved.",
                ex.base().what(),
                QueryBuilder::storeTtlStatement(),
                LOCATION_INFO);
        }
    }


    //=============================================================================
    //=============================================================================
    string DbConnection::fetchLastHistoryEvent(const string &full_attr_name)
    {
        assert(!full_attr_name.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        spdlog::trace("Fetching last history event for attribute: {}", full_attr_name);

        // the result
        string last_event;

        try
        {
            // create and perform a pqxx transaction
            last_event = pqxx::perform([&full_attr_name, this]() {
                // declare the work transaction for this event
                pqxx::work tx {(*_conn), FetchLastHistoryEvent};

                if (!tx.prepared(FetchLastHistoryEvent).exists())
                    tx.conn().prepare(FetchLastHistoryEvent, QueryBuilder::fetchLastHistoryEventStatement());

                // unless this is the first time this attribute event history has
                // been queried, then we expect something back
                auto result = tx.exec_prepared(FetchLastHistoryEvent, _conf_id_cache->value(full_attr_name));

                // if there is a result, there should be a single result to look at
                if (result.size() == 1)
                    return result.at(0).at(0).as<string>();

                // return a blank string, no event
                return string();
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("Can not return last event for attribute [" + full_attr_name + "].",
                ex.base().what(),
                QueryBuilder::fetchLastHistoryEventStatement(),
                LOCATION_INFO);
        }

        return last_event;
    }

    //=============================================================================
    //=============================================================================
    bool DbConnection::fetchAttributeArchived(const std::string &full_attr_name)
    {
        assert(!full_attr_name.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        if (_conf_id_cache->valueExists(full_attr_name))
        {
            spdlog::trace("Query attribute archived returns true for: {}", full_attr_name);
            return true;
        }

        spdlog::trace("Query attribute archived returns false for: {}", full_attr_name);
        return false;
    }

    //=============================================================================
    //=============================================================================
    AttributeTraits DbConnection::fetchAttributeTraits(const std::string &full_attr_name)
    {
        assert(!full_attr_name.empty());
        assert(_conn != nullptr);
        assert(_conf_id_cache != nullptr);
        assert(_error_desc_id_cache != nullptr);
        assert(_event_id_cache != nullptr);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        spdlog::trace("Fetching attribute traits for attribute: {}", full_attr_name);

        AttributeTraits traits;

        try
        {
            // create and perform a pqxx transaction
            traits = pqxx::perform([&full_attr_name, this]() {
                // declare the work transaction for this event
                pqxx::work tx {(*_conn), FetchAttributeTraits};

                if (!tx.prepared(FetchAttributeTraits).exists())
                    tx.conn().prepare(FetchAttributeTraits, QueryBuilder::fetchAttributeTraitsStatement());

                // always expect a result, the type info for the attribute
                auto row = tx.exec_prepared1(FetchAttributeTraits, full_attr_name);

                // expect a result, so construct an AttributeTraits from it
                return AttributeTraits {static_cast<Tango::AttrWriteType>(row.at(2).as<int>()),
                    static_cast<Tango::AttrDataFormat>(row.at(1).as<int>()),
                    static_cast<Tango::CmdArgType>(row.at(0).as<int>())};
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("Can not return the type traits for attribute [" + full_attr_name + "].",
                ex.base().what(),
                QueryBuilder::fetchAttributeTraitsStatement(),
                LOCATION_INFO);
        }

        return traits;
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeEvent(const std::string &full_attr_name, const std::string &event)
    {
        spdlog::debug("Event {} needs adding to the database, by request of attribute {}", event, full_attr_name);

        try
        {
            // since it does not exist, we must add it before storing history
            // events based on it
            auto event_id = pqxx::perform([&full_attr_name, &event, this]() {
                pqxx::work tx {(*_conn), StoreHistoryString};

                if (!tx.prepared(StoreHistoryString).exists())
                {
                    tx.conn().prepare(StoreHistoryString, QueryBuilder::storeHistoryStringStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreHistoryString);
                }

                auto row = tx.exec_prepared1(StoreHistoryString, event);
                tx.commit();

                // we should have a single row with a single result, so attempt to return it
                return row.at(0).as<int>();
            });

            spdlog::debug(
                "Stored event {} for attribute {} and got database id for it: {}", event, full_attr_name, event_id);

            // cache the new event id for future use
            _event_id_cache->cacheValue(event_id, event);
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The event [" + event + "] for attribute [" + full_attr_name + "] was not saved.",
                ex.base().what(),
                QueryBuilder::storeHistoryStringStatement(),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::storeErrorMsg(const std::string &full_attr_name, const std::string &error_msg)
    {
        spdlog::debug(
            "Error message \"{}\" needs adding to the database, by request of attribute {}", error_msg, full_attr_name);

        try
        {
            // add the error message to the database
            auto error_id = pqxx::perform([&full_attr_name, &error_msg, this]() {
                pqxx::work tx {(*_conn), StoreErrorString};

                if (!tx.prepared(StoreErrorString).exists())
                {
                    tx.conn().prepare(StoreErrorString, QueryBuilder::storeErrorStatement());
                    spdlog::trace("Created prepared statement for: {}", StoreErrorString);
                }

                // expect a single row returned
                auto row = tx.exec_prepared1(StoreErrorString, error_msg);
                tx.commit();

                // we should have a single row with a single result, so attempt to return it
                return row.at(0).as<int>();
            });

            spdlog::debug("Stored error message \"{}\" for attribute {} and got database id for it: {}",
                error_msg,
                full_attr_name,
                error_id);

            // cache the new error id for future use
            _error_desc_id_cache->cacheValue(error_id, error_msg);
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The error string [" + error_msg + "] for attribute [" + full_attr_name + "] was not saved",
                ex.base().what(),
                QueryBuilder::storeErrorStatement(),
                LOCATION_INFO);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::checkAttributeExists(const std::string &full_attr_name, const std::string &location)
    {
        // check the attribute has been configured and added to the database,
        // if it has not then we can not use it for operations
        if (!_conf_id_cache->valueExists(full_attr_name))
        {
            string msg {"This attribute [" + full_attr_name +
                "] does not exist in the database. Unable to work with this attribute until it is added."};

            spdlog::error("Error: The attribute does not exist in the database, add it first.");
            spdlog::error("Attribute details. Name: {}", full_attr_name);
            spdlog::error("Throwing consistency error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Consistency Error", msg, location);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::checkConnection(const std::string &location)
    {
        if (isClosed())
        {
            string msg {
                "Connection to database is closed. Ensure it has been opened before trying to use the connection."};

            spdlog::error(
                "Error: The DbConnection is showing a closed connection status, open it before using store functions");

            spdlog::error("Throwing connection error with message: \"{}\"", msg);
            Tango::Except::throw_exception("Connection Error", msg, location);
        }
    }

    //=============================================================================
    //=============================================================================
    void DbConnection::handlePqxxError(
        const string &msg, const string &what, const string &query, const std::string &location)
    {
        string full_msg {"The database transaction failed. " + msg};
        spdlog::error("Error: An unexpected error occurred when trying to run the database query");
        spdlog::error("Caught error at: {} Error: \"{}\"", location, what);
        spdlog::error("Error: Failed query: {}", query);
        spdlog::error("Throwing storage error with message: \"{}\"", full_msg);
        Tango::Except::throw_exception("Storage Error", full_msg, location);
    }
} // namespace pqxx_conn
} // namespace hdbpp_internal
