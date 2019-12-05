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

namespace hdbpp_internal
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
        struct Store
        {
            static void run(std::unique_ptr<std::vector<T>> &value, const AttributeTraits &traits, pqxx::prepare::invocation &inv, pqxx::work&)
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

        template<>
        struct Store<std::string>
        {
            static void run(std::unique_ptr<std::vector<std::string>> &value, const AttributeTraits &traits, pqxx::prepare::invocation &inv, pqxx::work &tx)
            {
                if (traits.isScalar())
                    inv((*value)[0]);
                else
                {
                    // a string needs quoting to be stored via this method, so it does not cause
                    // an error in the prepared statement
                    for (auto &str : *value)
                        str = tx.esc(str);

                    inv(*value);
                }
            }
        };

        //=============================================================================
        //=============================================================================
        template<>
        struct Store<bool>
        {
            static void run(std::unique_ptr<std::vector<bool>> &value, const AttributeTraits &traits, pqxx::prepare::invocation &inv, pqxx::work&)
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
        assert(traits.isValid());

        spdlog::trace("Storing data event for attribute {} with traits {}, value_r valid: {}, value_w valid: {}",
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

                // there is a single special case here, arrays of strings need a different syntax to store, 
                // to avoid the quoting. Its likely we will need more for DevEncoded and DevEnum
                if (_db_store_method == DbStoreMethod::InsertString || (traits.isArray() && traits.type() == Tango::DEV_STRING))
                {
                    auto query = _query_builder.storeDataEventString<T>(
                        pqxx::to_string(_conf_id_cache->value(full_attr_name)),
                        pqxx::to_string(event_time),
                        pqxx::to_string(quality),
                        value_r,
                        value_w,
                        traits);

                    tx.exec0(query);
                }
                else
                {
                    // prepare as a prepared statement, we are going to use these
                    // queries often
                    if (!tx.prepared(_query_builder.storeDataEventName(traits)).exists())
                    {
                        tx.conn().prepare(
                            _query_builder.storeDataEventName(traits), _query_builder.storeDataEventStatement<T>(traits));
                    }

                    // get the pqxx prepared statement invocation object to allow us to
                    // bind each parameter in turn, this gives us the flexibility to bind
                    // conditional parameters (as long as the query string matches)
                    auto inv = tx.prepared(_query_builder.storeDataEventName(traits));

                    // this lambda stores the data value correctly into the invocation,
                    // we must treat scalar/spectrum in different ways, one is a single
                    // element and the other an array. Further, the unique_ptr may be
                    // empty and signify a null should be stored in the column instead
                    auto store_value = [&tx, &traits,  &inv](auto &value) {
                        if (value && value->size() > 0)
                        {
                            store_data_utils::Store<T>::run(value, traits, inv, tx);
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
                }

                // commit the result
                tx.commit();
            });
        }
        catch (const pqxx::pqxx_exception &ex)
        {
            handlePqxxError("The attribute [" + full_attr_name + "] data event was not saved.",
                ex.base().what(),
                _query_builder.storeDataEventStatement<T>(traits),
                LOCATION_INFO);
        }
    }
} // namespace pqxx_conn
} // namespace hdbpp_internal
#endif // _PSQL_CONNECTION_TPP
