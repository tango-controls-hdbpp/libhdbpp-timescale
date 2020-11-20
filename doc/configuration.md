# Configuration

This is the configuration to use for [event subscribers](https://github.com/tango-controls-hdbpp/hdbpp-es) to use this backend

## Library Configuration Parameters

Configuration parameters are as follows:

| Parameter | Mandatory | Default | Description |
|------|-----|-----|-----|
| libname | true | None | Must be "libhdb++timescale.so", unless the event subscriber is directly built against this backend, in this case this is not needed. |
| connect_string | true | None | Postgres connection string, eg user=postgres host=localhost port=5432 dbname=hdb password=password |
| logging_level | false | error | Logging level. See table below |
| log_file | false | false | Enable logging to file |
| log_console | false | false | Enable logging to the console |
| log_syslog | false | false | Enable logging to syslog |
| log_file_name | false | None | When logging to file, this is the path and name of file to use. Ensure the path exists otherwise this is an error conditions. |

The logging_level parameter is case insensitive. Logging levels are as follows:

| Level | Description |
|------|-----|
| error | Log only error level events (recommended unless debugging) |
| warning | Log only warning level events |
| info | Log only warning level events |
| debug | Log only warning level events. Good for early install debugging |
| trace | Trace level logging. Excessive level of debug, good for involved debugging |
| disabled | Disable logging subsystem |

## Configuration Example

Short example LibConfiguration property value on an EventSubscriber. You will HAVE to change the various parts to match your system:

```
connect_string=user=hdb-user password=password host=hdb-database port=5432 dbname=hdb
logging_level=debug
log_file=true
log_syslog=false
log_console=false
libname=libhdb++timescale.so
log_file_name=/tmp/hdb/es-name.log
````
