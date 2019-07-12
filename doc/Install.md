# Installation Instructions

All submodules are combined into the final library for ease of deployment. This means just the libhdbpp-timescale.so needs deploying to the target system.

## System Dependencies

The running system requires libpq5 installed to support the calls Postgresql

## Installation

After the build has completed, simply run:

```
make install
```

On installation the cql scripts under etc/ will be placed in <CMAKE_INSTALL_PREFIX>/shared/libhdb++cassandra/