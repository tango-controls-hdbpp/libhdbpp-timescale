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

#ifndef _PSQL_CONNECTION_TPP
#define _PSQL_CONNECTION_TPP

#include "PqxxExtension.hpp"

namespace hdbpp
{
namespace pqxx_conn
{
    // This namespace containers a collection of specialisations for the storeDataEvent() function. These
    // specialisations can not be handled by the main function, so we break them out here.
    namespace store_data_utils
    {
        //=============================================================================
        //=============================================================================
        template<typename T>
        struct Preprocess
        {
            static void run(std::unique_ptr<std::vector<T>> &, pqxx::work &) {}
        };

        //=============================================================================
        //=============================================================================
        template<>
        struct Preprocess<std::string>
        {
            static void run(std::unique_ptr<std::vector<std::string>> &value, pqxx::work &tx)
            {
                for (auto &str : *value)
                    str = tx.quote(str);
            }
        };

        //=============================================================================
        //=============================================================================
        template<typename T>
        struct Store
        {
            static void run(
                std::unique_ptr<std::vector<T>> &value, pqxx::prepare::invocation &inv, const AttributeTraits &traits)
            {
                // for a scalar, store the first element of the vector,
                // we do not expect more than 1 element, for an array, store
                // the entire vector
                if (traits.isScalar())
                    inv((*value)[0]);
                else
                    inv(*value);
            }
        };

        //=============================================================================
        //=============================================================================
        template<>
        struct Store<bool>
        {
            static void run(std::unique_ptr<std::vector<bool>> &value,
                pqxx::prepare::invocation &inv,
                const AttributeTraits &traits)
            {
                // a vector<bool> is not actually a vector<bool>, rather its some kind of bitfield. When
                // trying to return an element, we appear to get some kind of bitfield reference (?),
                // and since we have no way to handle this in the string_traits templates, it causes
                // errors. Here we work around it, by creating a local variable and passing that instead
                if (traits.isScalar())
                {
                    bool v = (*value)[0];
                    inv(v);
                }
                else
                    inv(*value);
            }
        };
    } // namespace store_data_utils

    //=============================================================================
    //=============================================================================
    template<typename T>
    void DbConnection::storeDataEvent(const std::string &full_attr_name,
        double event_time,
        int quality,
        std::unique_ptr<vector<T>> value_r,
        std::unique_ptr<vector<T>> value_w,
        const AttributeTraits &traits)
    {
        assert(!full_attr_name.empty());
        assert(!traits.isValid());

        _logger->trace("Storing data event for attribute {} with traits {}, value_r valid: {}, value_w valid: {}",
            full_attr_name,
            traits,
            value_r->size() > 0,
            value_w->size() > 0);

        checkConnection(LOCATION_INFO);
        checkAttributeExists(full_attr_name, LOCATION_INFO);

        try
        {
            return pqxx::perform([&, this]() {
                pqxx::work tx {(*_conn), StoreDataEvent};

                // prepare as a prepared statement, we are going to use these
                // queries often
                if (!tx.prepared(_query_builder.storeDataEventName(traits)).exists())
                {
                    tx.conn().prepare(
                        _query_builder.storeDataEventName(traits), _query_builder.storeDataEventQuery<T>(traits));
                }

                // get the pqxx prepared statement invocation object to allow us to
                // bind each parameter in turn, this gives us the flexibility to bind
                // conditional parameters (as long as the query string matches)
                auto inv = tx.prepared(_query_builder.storeDataEventName(traits));

                // this lambda stores the data value correctly into the invocation,
                // we must treat scalar/spectrum in different ways, one is a single
                // element and the other an array. Further, the unique_ptr may be
                // empty and signify a null should be stored in the column instead
                auto store_value = [&tx, &inv, &traits](auto &value) {
                    if (value && value->size() > 0)
                    {
                        // this ensures strings are quoted and escaped, other types are ignored
                        store_data_utils::Preprocess<T>::run(value, tx);
                        store_data_utils::Store<T>::run(value, inv, traits);
                    }
                    else
                    {
                        // no value was given for this field, simply add a null
                        // instead, this allows invalid quality attributes to be saved
                        // with no data
                        inv();
                    }
                };

                // bind all the parameters
                inv(_conf_id_cache->value(full_attr_name));
                inv(event_time);

                if (traits.hasReadData())
                    store_value(value_r);

                if (traits.hasWriteData())
                    store_value(value_w);

                inv(quality);

                // execute
                inv.exec();

                // commit the result
                tx.commit();
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] data event was not saved.",
                ex.base().what(),
                _query_builder.storeDataEventQuery<T>(traits),
                LOCATION_INFO);
        }
    }

} // namespace pqxx_conn
} // namespace hdbpp
#endif // _PSQL_CONNECTION_TPP
