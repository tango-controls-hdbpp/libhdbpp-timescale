# Installation Instructions

All submodules are combined into the final library for ease of deployment. This means just the libhdbpp-timescale.so binary needs deploying to the target system.

## System Dependencies

The running system requires libpq5 installed to support the calls Postgresql. On Debian/Ubuntu this can be deployed as follows:

```bash
sudo apt-get install libpq5
```

## Installation

After the build has completed, simply run:

```
sudo make install
```

The shared library will be installed to /usr/local/lib on Debian/Ubuntu systems.