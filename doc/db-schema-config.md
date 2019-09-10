# Database Schema Configuration

Schema setup and management is a very important aspect to running the HDB++ system with TimescaleDb. The following presents guidelines and a setup plan, but it is not exhaustive and additional information is welcome.

Some of the information assumes familiarity with TimescaleDb terms and technologies. Please to TimescaleDb [documentation](www.timescaledb.com) for more information.

- [Database Schema Configuration](#Database-Schema-Configuration)
  - [Hypperchunk Sizes](#Hypperchunk-Sizes)
  - [Schema Import](#Schema-Import)
    - [Admin User](#Admin-User)
    - [Table Creation](#Table-Creation)
    - [Users](#Users)
  - [Clean-up](#Clean-up)
  - [Clustering](#Clustering)

## Hypperchunk Sizes

The [schema](../db-schema/schema.sql) file has default values set for all hyper table chunk sizes. It is assumed initial deployment data load will be smaller than the final fully operational system, so chunk sizes are as follows:

- 28 days for all data tables, except:
- 14 days for att_scalar_devdouble, since this appears to be used more often than other tables.

These values can, and should be, adjusted to the deployment situation. Please see the TimescaleDb [documentation](www.timescaledb.com) for information on choosing chunk sizes.

Important: These are initial values, the expectation is the database will be monitored and values adjusted as it takes on its full load.

## Schema Import

General setup steps.

### Admin User

Rather than create and manage the tables via a superuser, we create and admin user and have them create the tables:

```sql
CREATE ROLE hdb_admin WITH LOGIN PASSWORD 'hdbpp';
ALTER USER hdb_admin CREATEDB;
ALTER USER hdb_admin CREATEROLE;
ALTER USER hdb_admin SUPERUSER;
```

Note the SUPERUSER role will be stripped after the tables are set up.

### Table Creation

Now import the schema.sql as the hdb_admin user. From pqsl:

```bash
psql -U hdb_admin -h HOST -p PORT-f schema.sql  -d template1
```

Note: we use database template1 since hdb_admin currently has no database to connect to.

We should now have a hdb database owned by hdb_admin.

### Users

Next we need to set up the users (this may require some improvements, pull requests welcome). Connect as a superuser and create two roles, a readonly and a readwrite role:

```sql
-- Roles
CREATE ROLE readonly;
CREATE ROLE readwrite;

-- Permissions - readonly
GRANT CONNECT ON DATABASE hdb TO readonly;
GRANT USAGE ON SCHEMA public TO readonly;
GRANT SELECT ON ALL TABLES IN SCHEMA public TO readonly;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT ON TABLES TO readonly;

-- Permissions - readwrite
GRANT CONNECT ON DATABASE hdb TO readwrite;
GRANT USAGE ON SCHEMA public TO readwrite;
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO readwrite;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT, INSERT, UPDATE, DELETE ON TABLES TO readwrite;
GRANT USAGE ON ALL SEQUENCES IN SCHEMA public TO readwrite;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT USAGE ON SEQUENCES TO readwrite;
GRANT ALL ON SCHEMA public TO readwrite;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO readwrite;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO readwrite;

-- Users
CREATE ROLE hdb_cfg_man WITH LOGIN PASSWORD 'hdbpp';
GRANT readwrite TO hdb_cfg_man;

CREATE ROLE hdb_event_sub WITH LOGIN PASSWORD 'hdbpp';
GRANT readwrite TO hdb_event_sub;

CREATE ROLE hdb_java_reporter WITH LOGIN PASSWORD 'hdbpp';
GRANT readonly TO hdb_java_reporter;
```

Here we created three users that external applications will use to connect to the database. You may create as many and in what ever role you want.

## Clean-up

Finally, strip the SUPERUSER trait from hdb_admin:

```sql
ALTER USER hdb_admin NOSUPERUSER;
```

## Clustering

To get the levels of performance required to make the solution viable we MUST cluster on the composite index of each data table. the file [cluster.sql](../db-schema/cluster.sql) contains the commands that must be run after the database has been setup.

Without this step, select performance will degrade on large tables.

As data is added, the tables will require the new data to be clustered on the index. You may choose the period and time when to do this. The process does lock the tables. Options:

- Manually
- Cron job

TimescaleDb supports a more fine grained cluster process. A tool is being developed to utilities this and run as a process to cluster on the index at regular intervals.
