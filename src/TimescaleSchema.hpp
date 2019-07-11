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

#ifndef _TIMESCALE_SCHEMA_HPP
#define _TIMESCALE_SCHEMA_HPP

#include <string>

namespace hdbpp
{
namespace pqxx_conn
{
    // general schema related strings
    const std::string SCHEMA_TABLE_PREFIX = "att_";

    // attribute type information
    const std::string TYPE_SCALAR = "scalar";
    const std::string TYPE_ARRAY = "array";
    const std::string TYPE_IMAGE = "image";
    const std::string TYPE_DEV_BOOLEAN = "devboolean";
    const std::string TYPE_DEV_UCHAR = "devuchar";
    const std::string TYPE_DEV_SHORT = "devshort";
    const std::string TYPE_DEV_USHORT = "devushort";
    const std::string TYPE_DEV_LONG = "devlong";
    const std::string TYPE_DEV_ULONG = "devulong";
    const std::string TYPE_DEV_LONG64 = "devlong64";
    const std::string TYPE_DEV_ULONG64 = "devulong64";
    const std::string TYPE_DEV_FLOAT = "devfloat";
    const std::string TYPE_DEV_DOUBLE = "devdouble";
    const std::string TYPE_DEV_STRING = "devstring";
    const std::string TYPE_DEV_STATE = "devstate";
    const std::string TYPE_DEV_ENCODED = "devencoded";
    const std::string TYPE_DEV_ENUM = "devenum";

    // att_conf table
    const std::string CONF_TABLE_NAME = "att_conf";
    const std::string CONF_COL_ID = "att_conf_id";
    const std::string CONF_COL_NAME = "att_name";
    const std::string CONF_COL_TYPE_ID = "att_conf_type_id";
    const std::string CONF_COL_FORMAT_TYPE_ID = "att_conf_format_id";
    const std::string CONF_COL_WRITE_TYPE_ID = "att_conf_write_id";
    const std::string CONF_COL_TABLE_NAME = "table_name";
    const std::string CONF_COL_CS_NAME = "cs_name";
    const std::string CONF_COL_DOMAIN = "domain";
    const std::string CONF_COL_FAMILY = "family";
    const std::string CONF_COL_MEMBER = "member";
    const std::string CONF_COL_LAST_NAME = "name";
    const std::string CONF_COL_TTL = "ttl";
    const std::string CONF_COL_HIDE = "hide";

    // att_conf_type table
    const std::string CONF_TYPE_TABLE_NAME = "att_conf_type";
    const std::string CONF_TYPE_COL_TYPE_ID = "att_conf_type_id";
    const std::string CONF_TYPE_COL_TYPE = "type";
    const std::string CONF_TYPE_COL_TYPE_NUM = "type_num";

    // att_conf_format table
    const std::string CONF_FORMAT_TABLE_NAME = "att_conf_format";
    const std::string CONF_FORMAT_COL_FORMAT_ID = "att_conf_format_id";
    const std::string CONF_FORMAT_COL_FORMAT = "format";
    const std::string CONF_FORMAT_COL_FORMAT_NUM = "format_num";

    // att_conf_write table
    const std::string CONF_WRITE_TABLE_NAME = "att_conf_write";
    const std::string CONF_WRITE_COL_WRITE_ID = "att_conf_write_id";
    const std::string CONF_WRITE_COL_WRITE = "write";
    const std::string CONF_WRITE_COL_WRITE_NUM = "write_num";

    // att_history table
    const std::string HISTORY_TABLE_NAME = "att_history";
    const std::string HISTORY_COL_ID = "att_conf_id";
    const std::string HISTORY_COL_EVENT_ID = "att_history_event_id";
    const std::string HISTORY_COL_TIME = "event_time";
    const std::string HISTORY_COL_DETAILS = "details";

    // att_history_event table
    const std::string HISTORY_EVENT_TABLE_NAME = "att_history_event";
    const std::string HISTORY_EVENT_COL_EVENT_ID = "att_history_event_id";
    const std::string HISTORY_EVENT_COL_EVENT = "event";

    // att_parameter table
    const std::string PARAM_TABLE_NAME = "att_parameter";
    const std::string PARAM_COL_ID = "att_conf_id";
    const std::string PARAM_COL_INS_TIME = "insert_time";
    const std::string PARAM_COL_EV_TIME = "recv_time";
    const std::string PARAM_COL_LABEL = "label";
    const std::string PARAM_COL_UNIT = "unit";
    const std::string PARAM_COL_STANDARDUNIT = "standard_unit";
    const std::string PARAM_COL_DISPLAYUNIT = "display_unit";
    const std::string PARAM_COL_FORMAT = "format";
    const std::string PARAM_COL_ARCHIVERELCHANGE = "archive_rel_change";
    const std::string PARAM_COL_ARCHIVEABSCHANGE = "archive_abs_change";
    const std::string PARAM_COL_ARCHIVEPERIOD = "archive_period";
    const std::string PARAM_COL_DESCRIPTION = "description";
    const std::string PARAM_COL_DETAILS = "details";

    // att_error_desc table
    const std::string ERR_TABLE_NAME = "att_error_desc";
    const std::string ERR_COL_ID = "att_error_desc_id";
    const std::string ERR_COL_ERROR_DESC = "error_desc";

    // data tables
    const std::string DAT_COL_ID = "att_conf_id";
    const std::string DAT_COL_INS_TIME = "insert_time";
    const std::string DAT_COL_RCV_TIME = "recv_time";
    const std::string DAT_COL_DATA_TIME = "data_time";
    const std::string DAT_COL_VALUE_R = "value_r";
    const std::string DAT_COL_VALUE_W = "value_w";
    const std::string DAT_COL_QUALITY = "quality";
    const std::string DAT_COL_ERROR_DESC_ID = "att_error_desc_id";
    const std::string DAT_COL_DETAILS = "details";

    // special fields for enums
    const std::string DAT_COL_DAT_COL_VALUE_R_LABEL = "value_r_label";
    const std::string DAT_COL_DAT_COL_VALUE_W_LABEL = "value_w_label";

    // special fields for image tables
    const std::string DAT_IMG_COL_DIMX_R = "dim_x_r";
    const std::string DAT_IMG_COL_DIMY_R = "dim_y_r";
    const std::string DAT_IMG_COL_DIMX_W = "dim_x_w";
    const std::string DAT_IMG_COL_DIMY_W = "dim_y_w";
} // namespace pqxx_conn
} // namespace hdbpp
#endif // _TIMESCALE_SCHEMA_HPP
