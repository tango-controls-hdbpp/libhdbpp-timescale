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

        _logger->trace("Storing data event for attribute {} with traits {}, value_r valid: {}, value_w valid: {}", 
            full_attr_name, traits, value_r->size() > 0, value_w->size() > 0);

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
                    tx.conn().prepare(_query_builder.storeDataEventName(traits), _query_builder.storeDataEventQuery<T>(traits));
                }

                // get the pqxx prepared statement invocation object to allow us to
                // bind each parameter in turn, this gives us the flexibility to bind
                // conditional parameters (as long as the query string matches)
                pqxx::prepare::invocation inv = tx.prepared(_query_builder.storeDataEventName(traits));

                // this lambda stores the data value correctly into the invocation,
                // we must treat scalar/spectrum in different ways, one is a single
                // element and the other an array. Further, the unique_ptr may be
                // empty and signify a null should be stored in the column instead
                auto store_value = [&tx, &inv, &traits](auto &value) {
                    if (value && value->size() > 0)
                    {
                        // for a scalar, store the first element of the vector,
                        // we do not expect more than 1 element, for an array, store
                        // the entire vector
                        if (traits.isScalar())
                            inv((*value)[0]);
                        else
                            inv(*value);
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
