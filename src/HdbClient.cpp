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

#include "HdbppTimescaleDbApi.hpp"

#include "HdbClient.h"

using namespace std;

namespace hdbpp
{
//=============================================================================
//=============================================================================
HdbClient::HdbClient(const string &id, const vector<string> &configuration)
{
    auto db = new HdbppTimescaleDbApi(id, configuration);
    _db = unique_ptr<AbstractDB>(db);
}

//=============================================================================
//=============================================================================
void HdbClient::insert_event(Tango::EventData *event, const HdbEventDataType &data_type)
{
    _db->insert_event(event, data_type);
}

//=============================================================================
//=============================================================================
void HdbClient::insert_events(vector<tuple<Tango::EventData *, HdbEventDataType>> events)
{
    _db->insert_events(events);
}

//=============================================================================
//=============================================================================
void HdbClient::insert_param_event(Tango::AttrConfEventData *data, const HdbEventDataType &data_type)
{
    _db->insert_param_event(data, data_type);
}

//=============================================================================
//=============================================================================
void HdbClient::add_attribute(const string &name, int type, int format, int write_type)
{
    _db->add_attribute(name, type, format, write_type);
}

//=============================================================================
//=============================================================================
void HdbClient::update_ttl(const string &name, unsigned int ttl)
{
    _db->update_ttl(name, ttl);
}

//=============================================================================
//=============================================================================
void HdbClient::insert_history_event(const string &name, unsigned char event)
{
    _db->insert_history_event(move(name), event);
}

//=============================================================================
//=============================================================================
bool HdbClient::supported(HdbppFeatures feature)
{
    return _db->supported(feature);
}

} // namespace hdbpp
