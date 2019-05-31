DROP DATABASE IF EXISTS hdbpp_test;

-- Create the hdb database and use it
CREATE DATABASE hdbpp_test;
\c hdbpp_test

-- Add the timescaledb extension (Important)
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

-------------------------------------------------------------------------------
CREATE DOMAIN uchar AS numeric(3) -- ALT smallint
    CHECK(VALUE >= 0 AND VALUE <= 255);

CREATE DOMAIN ushort AS numeric(5)  -- ALT integer
    CHECK(VALUE >= 0 AND VALUE <= 65535);

CREATE DOMAIN ulong AS numeric(10) -- ALT bigint
    CHECK(VALUE >= 0 AND VALUE <= 4294967295);

CREATE DOMAIN ulong64 AS numeric(20)
    CHECK(VALUE >= 0 AND VALUE <= 18446744073709551615);

-------------------------------------------------------------------------------
DROP TABLE IF EXISTS att_conf_data_type;

CREATE TABLE IF NOT EXISTS att_conf_data_type (
    att_conf_data_type_id serial NOT NULL,
    data_type text NOT NULL,
    tango_data_type smallint NOT NULL,
    PRIMARY KEY (att_conf_data_type_id)
);

COMMENT ON TABLE att_conf_data_type is 'Attribute types description';

INSERT INTO att_conf_data_type (data_type, tango_data_type) VALUES
('att_scalar_devboolean_ro', 1),('att_scalar_devboolean_rw', 1),('att_att_array_devboolean_ro', 1),('att_array_devboolean_rw', 1),
('att_scalar_devuchar_ro', 22),('att_scalar_devuchar_rw', 22),('att_array_devuchar_ro', 22),('att_array_devuchar_rw', 22),
('att_scalar_devshort_ro', 2),('att_scalar_devshort_rw', 2),('att_array_devshort_ro', 2),('att_array_devshort_rw', 2),
('att_scalar_devushort_ro', 6),('att_scalar_devushort_rw', 6),('att_array_devushort_ro', 6),('att_array_devushort_rw', 6),
('att_scalar_devlong_ro', 3),('att_scalar_devlong_rw', 3),('att_array_devlong_ro', 3),('att_array_devlong_rw', 3),
('att_scalar_devulong_ro', 7),('att_scalar_devulong_rw', 7),('att_array_devulong_ro', 7),('att_array_devulong_rw', 7),
('att_scalar_devlong64_ro', 23),('att_scalar_devlong64_rw', 23),('att_array_devlong64_ro', 23),('att_array_devlong64_rw', 23),
('att_scalar_devulong64_ro', 24),('att_scalar_devulong64_rw', 24),('att_array_devulong64_ro', 24),('att_array_devulong64_rw', 24),
('att_scalar_devfloat_ro', 4),('att_scalar_devfloat_rw', 4),('att_array_devfloat_ro', 4),('att_array_devfloat_rw', 4),
('att_scalar_devdouble_ro', 5),('att_scalar_devdouble_rw', 5),('att_array_devdouble_ro', 5),('att_array_devdouble_rw', 5),
('att_scalar_devstring_ro', 8),('att_scalar_devstring_rw', 8),('att_array_devstring_ro', 8),('att_array_devstring_rw', 8),
('att_scalar_devstate_ro', 19),('att_scalar_devstate_rw', 19),('att_array_devstate_ro', 19),('att_array_devstate_rw', 19),
('att_scalar_devencoded_ro', 28),('att_scalar_devencoded_rw', 28),('att_array_devencoded_ro', 28),('att_array_devencoded_rw', 28),
('att_scalar_devenum_ro', 29),('att_scalar_devenum_rw', 29),('att_array_devenum_ro', 29),('att_array_devenum_rw', 29);

CREATE TABLE IF NOT EXISTS att_conf (
    att_conf_id serial NOT NULL,
    att_name text NOT NULL,
    att_conf_data_type_id integer NOT NULL,
    cs_name text NOT NULL DEFAULT '',
    domain text NOT NULL DEFAULT '',
    family text NOT NULL DEFAULT '',
    member text NOT NULL DEFAULT '',
    name text NOT NULL DEFAULT '',
    ttl int, -- TODO
    PRIMARY KEY (att_conf_id),
    FOREIGN KEY (att_conf_data_type_id) REFERENCES att_conf_data_type (att_conf_data_type_id),
    UNIQUE (att_name)
);

COMMENT ON TABLE att_conf is 'Attribute Configuration Table';
CREATE INDEX IF NOT EXISTS att_conf_att_conf_id_idx ON att_conf (att_conf_id);
CREATE INDEX IF NOT EXISTS att_conf_att_conf_data_type_id_idx ON att_conf (att_conf_data_type_id);

-------------------------------------------------------------------------------
DROP TABLE IF EXISTS att_history_event;

CREATE TABLE IF NOT EXISTS att_history_event (
    att_history_event_id serial NOT NULL,
    event text NOT NULL,
    PRIMARY KEY (att_history_event_id)
);

COMMENT ON TABLE att_history_event IS 'Attribute history events description';
CREATE INDEX IF NOT EXISTS att_history_att_history_event_id_idx ON att_history_event (att_history_event_id);

CREATE TABLE IF NOT EXISTS att_history (
    att_conf_id integer NOT NULL,
    att_history_event_id integer NOT NULL,
    event_time timestamp(6) with time zone,
    PRIMARY KEY (att_conf_id, event_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_history_event_id) REFERENCES att_history_event (att_history_event_id)
);

COMMENT ON TABLE att_history is 'Attribute Configuration Events History Table';
CREATE INDEX IF NOT EXISTS att_history_att_conf_id_inx ON att_history (att_conf_id);

-------------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS att_parameter (
    att_conf_id integer NOT NULL,
    recv_time timestamp with time zone NOT NULL,
    insert_time timestamp with time zone,
    label text NOT NULL DEFAULT '',
    unit text NOT NULL DEFAULT '',
    standard_unit text NOT NULL DEFAULT '',
    display_unit text NOT NULL DEFAULT '',
    format text NOT NULL DEFAULT '',
    archive_rel_change text NOT NULL DEFAULT '',
    archive_abs_change text NOT NULL DEFAULT '',
    archive_period text NOT NULL DEFAULT '',
    description text NOT NULL DEFAULT '',
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id)
);

COMMENT ON TABLE att_parameter IS 'Attribute configuration parameters';
CREATE INDEX IF NOT EXISTS att_parameter_recv_time_idx ON att_parameter (recv_time);
CREATE INDEX IF NOT EXISTS att_parameter_att_conf_id_idx ON  att_parameter (att_conf_id);
SELECT create_hypertable('att_parameter', 'recv_time', chunk_time_interval => interval '28 day', create_default_indexes => FALSE);

-------------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS att_error_desc (
    att_error_desc_id serial NOT NULL,
    error_desc text NOT NULL,
    PRIMARY KEY (att_error_desc_id),
    UNIQUE (error_desc)
);

COMMENT ON TABLE att_error_desc IS 'Error Description Table';
CREATE INDEX IF NOT EXISTS att_error_desc_att_error_desc_id_idx ON att_error_desc (att_error_desc_id);

-------------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS att_scalar_devboolean_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r boolean,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devboolean_ro IS 'Scalar Boolean ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devboolean_ro_att_conf_id_idx ON att_scalar_devboolean_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devboolean_ro_att_conf_id_data_time_idx ON att_scalar_devboolean_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devboolean_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devboolean_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r boolean,
    value_w boolean,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devboolean_rw IS 'Scalar Boolean ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devboolean_rw_att_conf_id_idx ON att_scalar_devboolean_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devboolean_rw_att_conf_id_data_time_idx ON att_scalar_devboolean_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devboolean_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devboolean_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r boolean[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devboolean_ro IS 'Array Boolean ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devboolean_ro_att_conf_id_idx ON att_array_devboolean_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devboolean_ro_att_conf_id_data_time_idx ON att_array_devboolean_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devboolean_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devboolean_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r boolean[],
    value_w boolean[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devboolean_rw IS 'Array Boolean ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devboolean_rw_att_conf_id_idx ON att_array_devboolean_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devboolean_rw_att_conf_id_data_time_idx ON att_array_devboolean_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devboolean_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devuchar_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r uchar,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devuchar_ro IS 'Scalar UChar ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devuchar_ro_att_conf_id_idx ON att_scalar_devuchar_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devuchar_ro_att_conf_id_data_time_idx ON att_scalar_devuchar_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devuchar_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devuchar_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r uchar,
    value_w uchar,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devuchar_rw IS 'Scalar UChar ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devuchar_rw_att_conf_id_idx ON att_scalar_devuchar_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devuchar_rw_att_conf_id_data_time_idx ON att_scalar_devuchar_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devuchar_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devuchar_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r uchar[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devuchar_ro IS 'Array UChar ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devuchar_ro_att_conf_id_idx ON att_array_devuchar_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devuchar_ro_att_conf_id_data_time_idx ON att_array_devuchar_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devuchar_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devuchar_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r uchar[],
    value_w uchar[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devuchar_rw IS 'Array UChar ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devuchar_rw_att_conf_id_idx ON att_array_devuchar_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devuchar_rw_att_conf_id_data_time_idx ON att_array_devuchar_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devuchar_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devshort_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r smallint,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devshort_ro IS 'Scalar Short ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devshort_ro_att_conf_id_idx ON att_scalar_devshort_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devshort_ro_att_conf_id_data_time_idx ON att_scalar_devshort_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devshort_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devshort_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r smallint,
    value_w smallint,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devshort_rw IS 'Scalar Short ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devshort_rw_att_conf_id_idx ON att_scalar_devshort_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devshort_rw_att_conf_id_data_time_idx ON att_scalar_devshort_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devshort_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devshort_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r smallint[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devshort_ro IS 'Array Short ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devshort_ro_att_conf_id_idx ON att_array_devshort_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devshort_ro_att_conf_id_data_time_idx ON att_array_devshort_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devshort_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devshort_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r smallint[],
    value_w smallint[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devshort_rw IS 'Array Short ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devshort_rw_att_conf_id_idx ON att_array_devshort_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devshort_rw_att_conf_id_data_time_idx ON att_array_devshort_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devshort_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devushort_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ushort,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devushort_ro IS 'Scalar UShort ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devushort_ro_att_conf_id_idx ON att_scalar_devushort_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devushort_ro_att_conf_id_data_time_idx ON att_scalar_devushort_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devushort_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devushort_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ushort,
    value_w ushort,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devushort_rw IS 'Scalar UShort ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devushort_rw_att_conf_id_idx ON att_scalar_devushort_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devushort_rw_att_conf_id_data_time_idx ON att_scalar_devushort_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devushort_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devushort_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ushort[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devushort_ro IS 'Array UShort ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devushort_ro_att_conf_id_idx ON att_array_devushort_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devushort_ro_att_conf_id_data_time_idx ON att_array_devushort_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devushort_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devushort_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ushort[],
    value_w ushort[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devushort_rw IS 'Array UShort ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devushort_rw_att_conf_id_idx ON att_array_devushort_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devushort_rw_att_conf_id_data_time_idx ON att_array_devushort_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devushort_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devlong_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devlong_ro IS 'Scalar Long ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devlong_ro_att_conf_id_idx ON att_scalar_devlong_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devlong_ro_att_conf_id_data_time_idx ON att_scalar_devlong_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devlong_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devlong_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    value_w integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devlong_rw IS 'Scalar Long ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devlong_rw_att_conf_id_idx ON att_scalar_devlong_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devlong_rw_att_conf_id_data_time_idx ON att_scalar_devlong_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devlong_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devlong_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devlong_ro IS 'Array Long ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devlong_ro_att_conf_id_idx ON att_array_devlong_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devlong_ro_att_conf_id_data_time_idx ON att_array_devlong_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devlong_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devlong_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    value_w integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devlong_rw IS 'Array Long ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devlong_rw_att_conf_id_idx ON att_array_devlong_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devlong_rw_att_conf_id_data_time_idx ON att_array_devlong_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devlong_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devulong_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devulong_ro IS 'Scalar ULong ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devulong_ro_att_conf_id_idx ON att_scalar_devulong_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devulong_ro_att_conf_id_data_time_idx ON att_scalar_devulong_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devulong_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devulong_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong,
    value_w ulong,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devulong_rw IS 'Scalar ULong ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devulong_rw_att_conf_id_idx ON att_scalar_devulong_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devulong_rw_att_conf_id_data_time_idx ON att_scalar_devulong_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devulong_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devulong_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devulong_ro IS 'Array ULong ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devulong_ro_att_conf_id_idx ON att_array_devulong_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devulong_ro_att_conf_id_data_time_idx ON att_array_devulong_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devulong_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devulong_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong[],
    value_w ulong[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devulong_rw IS 'Array ULong ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devulong_rw_att_conf_id_idx ON att_array_devulong_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devulong_rw_att_conf_id_data_time_idx ON att_array_devulong_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devulong_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devlong64_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bigint,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devlong64_ro IS 'Scalar Long64 ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devlong64_ro_att_conf_id_idx ON att_scalar_devlong64_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devlong64_ro_att_conf_id_data_time_idx ON att_scalar_devlong64_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devlong64_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devlong64_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bigint,
    value_w bigint,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devlong64_rw IS 'Scalar Long64 ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devlong64_rw_att_conf_id_idx ON att_scalar_devlong64_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devlong64_rw_att_conf_id_data_time_idx ON att_scalar_devlong64_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devlong64_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devlong64_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bigint[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devlong64_ro IS 'Array Long64 ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devlong64_ro_att_conf_id_idx ON att_array_devlong64_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devlong64_ro_att_conf_id_data_time_idx ON att_array_devlong64_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devlong64_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devlong64_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bigint[],
    value_w bigint[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devlong64_rw IS 'Array Long64 ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devlong64_rw_att_conf_id_idx ON att_array_devlong64_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devlong64_rw_att_conf_id_data_time_idx ON att_array_devlong64_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devlong64_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devulong64_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong64,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devulong64_ro IS 'Scalar ULong64 ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devulong64_ro_att_conf_id_idx ON att_scalar_devulong64_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devulong64_ro_att_conf_id_data_time_idx ON att_scalar_devulong64_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devulong64_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devulong64_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong64,
    value_w ulong64,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devulong64_rw IS 'Scalar ULong64 ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devulong64_rw_att_conf_id_idx ON att_scalar_devulong64_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devulong64_rw_att_conf_id_data_time_idx ON att_scalar_devulong64_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devulong64_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devulong64_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong64[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devulong64_ro IS 'Array ULong64 ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devulong64_ro_att_conf_id_idx ON att_array_devulong64_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devulong64_ro_att_conf_id_data_time_idx ON att_array_devulong64_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devulong64_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devulong64_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r ulong64[],
    value_w ulong64[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devulong64_rw IS 'Array ULong64 ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devulong64_rw_att_conf_id_idx ON att_array_devulong64_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devulong64_rw_att_conf_id_data_time_idx ON att_array_devulong64_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devulong64_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devfloat_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r real,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devfloat_ro IS 'Scalar Float ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devfloat_ro_att_conf_id_idx ON att_scalar_devfloat_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devfloat_ro_att_conf_id_data_time_idx ON att_scalar_devfloat_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devfloat_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devfloat_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r real,
    value_w real,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devfloat_rw IS 'Scalar Float ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devfloat_rw_att_conf_id_idx ON att_scalar_devfloat_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devfloat_rw_att_conf_id_data_time_idx ON att_scalar_devfloat_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devfloat_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devfloat_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r real[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devfloat_ro IS 'Array Float ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devfloat_ro_att_conf_id_idx ON att_array_devfloat_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devfloat_ro_att_conf_id_data_time_idx ON att_array_devfloat_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devfloat_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devfloat_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r real[],
    value_w real[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devfloat_rw IS 'Array Float ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devfloat_rw_att_conf_id_idx ON att_array_devfloat_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devfloat_rw_att_conf_id_data_time_idx ON att_array_devfloat_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devfloat_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devdouble_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r double precision,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devdouble_ro IS 'Scalar Double ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devdouble_ro_att_conf_id_idx ON att_scalar_devdouble_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devdouble_ro_att_conf_id_data_time_idx ON att_scalar_devdouble_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devdouble_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devdouble_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r double precision,
    value_w double precision,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devdouble_rw IS 'Scalar Double ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devdouble_rw_att_conf_id_idx ON att_scalar_devdouble_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devdouble_rw_att_conf_id_data_time_idx ON att_scalar_devdouble_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devdouble_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devdouble_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r double precision[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devdouble_ro IS 'Array Double ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devdouble_ro_att_conf_id_idx ON att_array_devdouble_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devdouble_ro_att_conf_id_data_time_idx ON att_array_devdouble_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devdouble_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devdouble_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r double precision[],
    value_w double precision[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devdouble_rw IS 'Array Double ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devdouble_rw_att_conf_id_idx ON att_array_devdouble_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devdouble_rw_att_conf_id_data_time_idx ON att_array_devdouble_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devdouble_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devstring_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r text,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devstring_ro IS 'Scalar String ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devstring_ro_att_conf_id_idx ON att_scalar_devstring_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devstring_ro_att_conf_id_data_time_idx ON att_scalar_devstring_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devstring_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devstring_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r text,
    value_w text,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devstring_rw IS 'Scalar String ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devstring_rw_att_conf_id_idx ON att_scalar_devstring_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devstring_rw_att_conf_id_data_time_idx ON att_scalar_devstring_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devstring_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devstring_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r text[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devstring_ro IS 'Array String ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devstring_ro_att_conf_id_idx ON att_array_devstring_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devstring_ro_att_conf_id_data_time_idx ON att_array_devstring_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devstring_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devstring_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r text[],
    value_w text[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devstring_rw IS 'Array String ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devstring_rw_att_conf_id_idx ON att_array_devstring_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devstring_rw_att_conf_id_data_time_idx ON att_array_devstring_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devstring_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devstate_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devstate_ro IS 'Scalar State ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devstate_ro_att_conf_id_idx ON att_scalar_devstate_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devstate_ro_att_conf_id_data_time_idx ON att_scalar_devstate_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devstate_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devstate_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    value_w integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devstate_rw IS 'Scalar State ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devstate_rw_att_conf_id_idx ON att_scalar_devstate_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devstate_rw_att_conf_id_data_time_idx ON att_scalar_devstate_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devstate_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devstate_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devstate_ro IS 'Array State ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devstate_ro_att_conf_id_idx ON att_array_devstate_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devstate_ro_att_conf_id_data_time_idx ON att_array_devstate_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devstate_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devstate_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    value_w integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devstate_rw IS 'Array State ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devstate_rw_att_conf_id_idx ON att_array_devstate_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devstate_rw_att_conf_id_data_time_idx ON att_array_devstate_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devstate_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devenum_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devenum_ro IS 'Scalar Enum ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devenum_ro_att_conf_id_idx ON att_scalar_devenum_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devenum_ro_att_conf_id_data_time_idx ON att_scalar_devenum_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devenum_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devenum_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer,
    value_w integer,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_scalar_devenum_rw IS 'Scalar Enum ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devenum_rw_att_conf_id_idx ON att_scalar_devenum_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devenum_rw_att_conf_id_data_time_idx ON att_scalar_devenum_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devenum_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devenum_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devenum_ro IS 'Array Enum ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devenum_ro_att_conf_id_idx ON att_array_devenum_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devenum_ro_att_conf_id_data_time_idx ON att_array_devenum_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devenum_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devenum_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r integer[],
    value_w integer[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);

COMMENT ON TABLE att_array_devenum_rw IS 'Array Enum ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devenum_rw_att_conf_id_idx ON att_array_devenum_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devenum_rw_att_conf_id_data_time_idx ON att_array_devenum_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devenum_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devencoded_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bytea,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);
COMMENT ON TABLE att_scalar_devenum_ro IS 'Scalar DevEncoded ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devencoded_ro_att_conf_id_idx ON att_scalar_devencoded_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devencoded_ro_att_conf_id_data_time_idx ON att_scalar_devencoded_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devencoded_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_scalar_devencoded_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bytea,
    value_w bytea,
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);
COMMENT ON TABLE att_scalar_devenum_rw IS 'Scalar DevEncoded ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_scalar_devencoded_rw_att_conf_id_idx ON att_scalar_devencoded_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_scalar_devencoded_rw_att_conf_id_data_time_idx ON att_scalar_devencoded_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_scalar_devencoded_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devencoded_ro (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bytea[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);
COMMENT ON TABLE att_array_devenum_ro IS 'Array DevEncoded ReadOnly Values Table';
CREATE INDEX IF NOT EXISTS att_array_devencoded_ro_att_conf_id_idx ON att_array_devencoded_ro (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devencoded_ro_att_conf_id_data_time_idx ON att_array_devencoded_ro (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devencoded_ro', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);

CREATE TABLE IF NOT EXISTS att_array_devencoded_rw (
    att_conf_id integer NOT NULL,
    data_time timestamp with time zone NOT NULL,
    recv_time timestamp with time zone,
    insert_time timestamp with time zone,
    value_r bytea[],
    value_w bytea[],
    quality smallint,
    att_error_desc_id integer,
    PRIMARY KEY (att_conf_id, data_time),
    FOREIGN KEY (att_conf_id) REFERENCES att_conf (att_conf_id),
    FOREIGN KEY (att_error_desc_id) REFERENCES att_error_desc (att_error_desc_id)
);
COMMENT ON TABLE att_array_devenum_rw IS 'Array DevEncoded ReadWrite Values Table';
CREATE INDEX IF NOT EXISTS att_array_devencoded_rw_att_conf_id_idx ON att_array_devencoded_rw (att_conf_id);
CREATE INDEX IF NOT EXISTS att_array_devencoded_rw_att_conf_id_data_time_idx ON att_array_devencoded_rw (att_conf_id,data_time DESC);
SELECT create_hypertable('att_array_devencoded_rw', 'data_time', chunk_time_interval => interval '1 day', create_default_indexes => FALSE);