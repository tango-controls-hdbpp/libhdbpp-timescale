# libhdbpp-timescale

[![TangoControls](https://img.shields.io/badge/-Tango--Controls-7ABB45.svg?style=flat&logo=%20data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAACAAAAAkCAYAAADo6zjiAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAsFJREFUWIXtl01IFVEYht9zU%2FvTqOxShLowlOgHykWUGEjUKqiocB1FQURB0KJaRdGiaFM7gzZRLWpTq2olhNQyCtpYCP1gNyIoUTFNnxZzRs8dzvw4Q6564XLnfOf73vedc2a%2BmZEKALgHrC3CUUR8CxZFeEoFalsdM4uLmMgFoIlZLJp3A9ZE4S2oKehhlaR1BTnyg2ocnW%2FxsxEDhbYij4EPVncaeASMAavnS%2FwA8NMaqACNQCew3f4as3KZOYh2SuqTVJeQNiFpn6QGSRVjTH9W%2FiThvcCn6H6n4BvQDvQWFT%2BSIDIFDAKfE3KOAQeBfB0XGPeQvgE67P8ZoB44DvTHmFgJdOQRv%2BUjc%2BavA9siNTWemgfA3TwGquCZ3w8szFIL1ALngIZorndvgJOR0GlP2gtJkzH%2Bd0fGFxW07NqY%2FCrx5QRXcYjbCbmxF1dkBSbi8kpACah3Yi2Sys74cVyxMWY6bk5BTwgRe%2BYlSzLmxNpU3aBeJogk4XWWpJKUeiap3RJYCpQj4QWZDQCuyIAk19Auj%2BAFYGZZjTGjksaBESB8P9iaxUBIaJzjZcCQcwHdj%2BS2Al0xPOeBYYKHk4vfmQ3Y8YkIwRUb7wQGU7j2ePrA1URx93ayd8UpD8klyPbSQfCOMIO05MbI%2BDvwBbjsMdGTwlX21AAMZzEerkaI9zFkP4AeYCPBg6gNuEb6I%2FthFgN1KSQupqzoRELOSed4DGiJala1UmOMr2U%2Bl%2FTWEy9Japa%2Fy41IWi%2FJ3d4%2FkkaAw0Bz3AocArqApwTvet3O3GbgV8qqjAM7bf4N4KMztwTodcYVyelywKSCD5V3xphNXoezuTskNSl4bgxJ6jPGVJJqbN0aSV%2Bd0M0aO7FCs19Jo2lExphXaTkxdRVgQFK7DZVDZ8%2BcpdmQh3wuILh7ut3AEyt%2B51%2BL%2F0cUfwFOX0t0StltmQAAAABJRU5ErkJggg%3D%3D)](http://www.tango-controls.org) [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![](https://img.shields.io/github/release/tango-controls-hdbpp/libhdbpp-timescale.svg)](https://github.com/tango-controls-hdbpp/libhdbpp-timescale/releases)

- [libhdbpp-timescale](#libhdbpp-timescale)
  - [v0.9.0 To v0.10.0 Update](#v090-To-v0100-Update)
  - [Cloning](#Cloning)
  - [Bug Reports + Feature Requests](#Bug-Reports--Feature-Requests)
  - [Documentation](#Documentation)
  - [Building](#Building)
    - [Dependencies](#Dependencies)
      - [Toolchain Dependencies](#Toolchain-Dependencies)
      - [Build Dependencies](#Build-Dependencies)
    - [Building Process](#Building-Process)
    - [Build Flags](#Build-Flags)
      - [Standard CMake Flags](#Standard-CMake-Flags)
      - [Project Flags](#Project-Flags)
    - [Running Tests](#Running-Tests)
      - [Unit Tests](#Unit-Tests)
      - [Benchmark Tests](#Benchmark-Tests)
  - [Installing](#Installing)
    - [System Dependencies](#System-Dependencies)
    - [Installation](#Installation)
  - [Configuration](#Configuration)
    - [Library Configuration Parameters](#Library-Configuration-Parameters)
    - [Configuration Example](#Configuration-Example)
  - [License](#License)

HDB++ backend library for the TimescaleDb extenstion to Postgresql. This library is loaded by libhdbpp to archive events from a Tango Controls system. Currently in a pre v1 release phase.

The library requires a correctly configured [TimescaleDb](https://www.timescale.com/) installation. 

The library has been build against a number of other projects, these have been integrated into the repository as sub modules. This may change at a later date if the build system is improved. Current sub modules are:

* libpqxx - Modern C++ Postgresql library
* spdlog - Logging system
* Catch2 - Unit test subsystem

## v0.9.0 To v0.10.0 Update 

This revision changes how both scalar and spectrum strings are stored. In 0.9.0 strings were escaped and quoted before being stored in the database. This had the effect that when the strings were retrieved they were still escaped/quoted. For consistency scalar strings were stored escaped/quoted.

To fix this, spectrum's of strings are now stored via insert strings using both the ARRAY syntax and dollar escape method. This means when they are retrieved frm the database they are longer escaped/quoted. To match this, scalar strings are also no longer stored escaped/quoted.

## Cloning

Currently this project is configured to express its dependencies as submodules. This may change in future if there is time to explore, for example, the Meson build system. To successfully clone the project and all its dependencies use the following:

```bash
git clone --recurse-submodules https://github.com/tango-controls-hdbpp/libhdbpp-timescale.git
```

## Bug Reports + Feature Requests

Please file the bug reports and feature requests in the issue tracker

## Documentation

* See the [doc](doc/README.md) directory for documentation on various topics, including building and installing.
* General documentations on the Tango HDB system can be found [here](http://tango-controls.readthedocs.io/en/latest/administration/services/hdbpp/index.html#hdb-an-archiving-historian-service)

## Building

To build the shared library please read the following.

### Dependencies

The project has two types of dependencies, those required by the toolchain, and those to do the actual build. Other dependencies are integrated directly into the project as submodules. The following thirdparty modules exists:

* libpqxx - Modern C++ Postgresql library (submodule)
* spdlog - Logging system (submodule)
* Catch2 - Unit test subsystem (submodule)
* libhdbpp - Configuration can now fetch [original](https://github.com/tango-controls-hdbpp/libhdbpp) to aid development. See build flags.

#### Toolchain Dependencies

If wishing to build the project, ensure the following dependencies are met:

* CMake 3.11 or higher (for FetchContent)
* C++14 compatible compiler (code base is using c++14)

#### Build Dependencies

Ensure the development version of the dependencies are installed. These are as follows:

* Tango Controls 9 or higher development headers and libraries
* omniORB release 4 or higher development headers and libraries
* libzmq3-dev or libzmq5-dev
* libpq-dev - Postgres C development library

### Building Process

To compile this library, first ensure it has been recursively cloned so all submodules are present in /thirdparty. The build system uses pkg-config to find some dependencies, for example Tango. If Tango is not installed to a standard location, set PKG_CONFIG_PATH, i.e.

```bash
export PKG_CONFIG_PATH=/non/standard/tango/install/location
```

Then to build just the library:

```bash
mkdir -p build
cd build
cmake ..
make
```

The pkg-config path can also be set with the cmake argument CMAKE_PREFIX_PATH. This can be set on the command line at configuration time, i.e.:

```bash
...
cmake -DCMAKE_PREFIX_PATH=/non/standard/tango/install/location ..
...
```

### Build Flags

The following build flags are available

#### Standard CMake Flags

The following is a list of common useful CMake flags and their use:

| Flag | Setting | Description |
|------|-----|-----|
| CMAKE_INSTALL_PREFIX | PATH | Standard CMake flag to modify the install prefix. |
| CMAKE_INCLUDE_PATH | PATH[S] | Standard CMake flag to add include paths to the search path. |
| CMAKE_LIBRARY_PATH | PATH[S] | Standard CMake flag to add paths to the library search path |
| CMAKE_BUILD_TYPE | Debug/Release | Build type to produce |

#### Project Flags

| Flag | Setting | Default | Description |
|------|-----|-----|-----|
| BUILD_UNIT_TESTS | ON/OFF | OFF | Build unit tests |
| BUILD_BENCHMARK_TESTS | ON/OFF | OFF | Build benchmark tests (Forces a Release build) |
| ENABLE_CLANG | ON/OFF | OFF | Clang code static analysis, readability, and cppcore guideline enforcement |
| FETCH_LIBHDBPP | ON/OFF | OFF | Enable to have the build fetch and use a local version of libhdbpp |
| FETCH_LIBHDBPP_TAG | | master | When FETCH_LIBHDBPP is enabled, this is the git tag to fetch |

### Running Tests

#### Unit Tests

The project has extensive unit tests to ensure its functioning as expect. Build the project with testing enabled:

```bash
mkdir -p build
cd build
cmake -DBUILD_UNIT_TESTS=ON ..
make
```

To run all unit tests, a postgresql database node is required with the project schema loaded up. There is a default connection string inside test/TestHelpers.hpp:

```
user=postgres host=localhost port=5432 dbname=hdb password=password
```

If you run the hdb timescale docker image associated with this project locally then this will connect automatically. If you wish to use a different database, edit the string in test/TestHelpers.hpp.

To run all tests:

```bash
./test/unit-tests
```

To look at the available tests and tags, should you wish to run a subset of the test suite (for example, you do not have a postgresql node to test against), then tests and be listed:

```bash
./bin/unit-tests --list-tests
```

Or:

```bash
./bin/unit-tests --list-tags
```

To see more options for the unit-test command line binary:

```bash
./bin/unit-tests --help
```

#### Benchmark Tests

These are a work in progress to explore future optimisation point. If built, they can be run as follows:

```bash
mkdir -p build
cd build
cmake -DBUILD_BENCHMARK_TESTS=ON ..
make
```

```bash
./benchmark/benchmark-tests
```

## Installing

All submodules are combined into the final library for ease of deployment. This means just the libhdbpp-timescale.so binary needs deploying to the target system.

### System Dependencies

The running system requires libpq5 installed to support the calls Postgresql. On Debian/Ubuntu this can be deployed as follows:

```bash
sudo apt-get install libpq5
```

### Installation

After the build has completed, simply run:

```
sudo make install
```

The shared library will be installed to /usr/local/lib on Debian/Ubuntu systems.

## Configuration

### Library Configuration Parameters

Configuration parameters are as follows:

| Parameter | Mandatory | Default | Description |
|------|-----|-----|-----|
| libname | true | None | Must be "libhdb++timescale.so" |
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

### Configuration Example

Short example LibConfiguration property value on an EventSubscriber or ConfigManager. You will HAVE to change the various parts to match your system:

```bash
connect_string=user=hdb-user password=password host=hdb-database port=5432 dbname=hdb
logging_level=debug
log_file=true
log_syslog=false
log_console=false
libname=libhdb++timescale.so
log_file_name=/tmp/hdb/es-name.log
```

## License

The source code is released under the LGPL3 license and a copy of this license is provided with the code.
