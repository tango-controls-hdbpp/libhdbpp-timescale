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

namespace hdbpp_internal
{
namespace pqxx_conn
{
    namespace schema
    {
        // general schema related strings
        const std::string SchemaTablePrefix = "att_";

        // attribute type information
        const std::string TypeScalar = "scalar";
        const std::string TypeArray = "array";
        const std::string TypeImage = "image";
        const std::string TypeDevBoolean = "devboolean";
        const std::string TypeDevUchar = "devuchar";
        const std::string TypeDevShort = "devshort";
        const std::string TypeDevUshort = "devushort";
        const std::string TypeDevLong = "devlong";
        const std::string TypeDevUlong = "devulong";
        const std::string TypeDevLong64 = "devlong64";
        const std::string TypeDevUlong64 = "devulong64";
        const std::string TypeDevFloat = "devfloat";
        const std::string TypeDevDouble = "devdouble";
        const std::string TypeDevString = "devstring";
        const std::string TypeDevState = "devstate";
        const std::string TypeDevEncoded = "devencoded";
        const std::string TypeDevEnum = "devenum";

        // att_conf table
        const std::string ConfTableName = "att_conf";
        const std::string ConfColId = "att_conf_id";
        const std::string ConfColName = "att_name";
        const std::string ConfColTypeId = "att_conf_type_id";
        const std::string ConfColFormatTypeId = "att_conf_format_id";
        const std::string ConfColWriteTypeId = "att_conf_write_id";
        const std::string ConfColTableName = "table_name";
        const std::string ConfColCsName = "cs_name";
        const std::string ConfColDomain = "domain";
        const std::string ConfColFamily = "family";
        const std::string ConfColMember = "member";
        const std::string ConfColLastName = "name";
        const std::string ConfColTtl = "ttl";
        const std::string ConfColHide = "hide";

        // att_conf_type table
        const std::string ConfTypeTableName = "att_conf_type";
        const std::string ConfTypeColTypeId = "att_conf_type_id";
        const std::string ConfTypeColType = "type";
        const std::string ConfTypeColTypeNum = "type_num";

        // att_conf_format table
        const std::string ConfFormatTableName = "att_conf_format";
        const std::string ConfFormatColFormatId = "att_conf_format_id";
        const std::string ConfFormatColFormat = "format";
        const std::string ConfFormatColFormatNum = "format_num";

        // att_conf_write table
        const std::string ConfWriteTableName = "att_conf_write";
        const std::string ConfWriteColWriteId = "att_conf_write_id";
        const std::string ConfWriteColWrite = "write";
        const std::string ConfWriteColWriteNum = "write_num";

        // att_history table
        const std::string HistoryTableName = "att_history";
        const std::string HistoryColId = "att_conf_id";
        const std::string HistoryColEventId = "att_history_event_id";
        const std::string HistoryColTime = "event_time";
        const std::string HistoryColDetails = "details";

        // att_history_event table
        const std::string HistoryEventTableName = "att_history_event";
        const std::string HistoryEventColEventId = "att_history_event_id";
        const std::string HistoryEventColEvent = "event";

        // att_parameter table
        const std::string ParamTableName = "att_parameter";
        const std::string ParamColId = "att_conf_id";
        const std::string ParamColInsTime = "insert_time";
        const std::string ParamColEvTime = "recv_time";
        const std::string ParamColLabel = "label";
        const std::string ParamColEnumLabels = "enum_labels";
        const std::string ParamColUnit = "unit";
        const std::string ParamColStandardUnit = "standard_unit";
        const std::string ParamColDisplayUnit = "display_unit";
        const std::string ParamColFormat = "format";
        const std::string ParamColArchiveRelChange = "archive_rel_change";
        const std::string ParamColArchiveAbsChange = "archive_abs_change";
        const std::string ParamColArchivePeriod = "archive_period";
        const std::string ParamColDescription = "description";
        const std::string ParamColDetails = "details";

        // att_error_desc table
        const std::string ErrTableName = "att_error_desc";
        const std::string ErrColId = "att_error_desc_id";
        const std::string ErrColErrorDesc = "error_desc";

        // data tables
        const std::string DatColId = "att_conf_id";
        const std::string DatColDataTime = "data_time";
        const std::string DatColValueR = "value_r";
        const std::string DatColValueW = "value_w";
        const std::string DatColQuality = "quality";
        const std::string DatColErrorDescId = "att_error_desc_id";
        const std::string DatColDetails = "details";

        // special fields for enums
        const std::string DatColDatColValueRLabel = "value_r_label";
        const std::string DatColDatColValueWLabel = "value_w_label";

        // special fields for image tables
        const std::string DatImgColDimxR = "dim_x_r";
        const std::string DatImgColDimyR = "dim_y_r";
        const std::string DatImgColDimxW = "dim_x_w";
        const std::string DatImgColDimyW = "dim_y_w";
    } // namespace schema
} // namespace pqxx_conn
} // namespace hdbpp_internal
#endif // _TIMESCALE_SCHEMA_HPP
