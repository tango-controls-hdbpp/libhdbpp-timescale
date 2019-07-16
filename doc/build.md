# Build Instructions

To build the shared library please read the following.

## Dependencies

The project has two types of dependencies, those required by the toolchain, and those to do the actual build. Other dependencies are integrated directly into the project as submodules. The following thirdparty modules exists:

* libpqxx - Modern C++ Postgresql library (Submodule)
* spdlog - Logging system (Submodule)
* Catch2 - Unit test subsystem (Submodule)
* libhdbpp - Part of the hdb++ library loading chain (Modified version of [original](https://github.com/tango-controls-hdbpp/libhdbpp) project. This will be pushed back to the original repository in time)

### Toolchain Dependencies

If wishing to build the project, ensure the following dependencies are met:

* CMake 3.6 or higher
* C++14 compatible compiler (code base is using c++14)

### Build Dependencies

Ensure the development version of the dependencies are installed. These are as follows:

* Tango Controls 9 or higher development headers and libraries
* omniORB release 4 or higher development headers and libraries
* libzmq3-dev or libzmq5-dev
* libpq-dev - Postgres C development library

## Building and Installation

To compile this library, first ensure it has been recursively cloned so all submodules are present in /thirdparty. The build system uses pkg-config to find some dependencies, for example Tango. If Tango is not installed to a standard location, set PKG_CONFIG_PATH, i.e.

```bash
export PKG_CONFIG_PATH=/non/standard/tango/install/location
```

CMake is also set to search any paths added to CMAKE_PREFIX_PATH. This can be set and passed into CMake instead.

Then to build just the library:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Build Flags

The following build flags are available

### Standard CMake Flags

The following is a list of common useful CMake flags and their use:

| Flag | Setting | Description |
|------|-----|-----|
| CMAKE_INSTALL_PREFIX | PATH | Standard CMake flag to modify the install prefix. |
| CMAKE_INCLUDE_PATH | PATH[S] | Standard CMake flag to add include paths to the search path. |
| CMAKE_LIBRARY_PATH | PATH[S] | Standard CMake flag to add paths to the library search path |
| CMAKE_BUILD_TYPE | Debug/Release | Build type to produce |

### Project Flags

| Flag | Setting | Default | Description |
|------|-----|-----|-----|
| HDBPP_TDB_BUILD_TESTS | ON/OFF | OFF | Build unit tests |
| HDBPP_TDB_BUILD_DEBUG_SYMBOLS | ON/OFF | OFF | Add additional debug systems to the library |
| HDBPP_TDB_ENABLE_CLANG | ON/OFF | OFF | Clang code static analysis, readability, and cppcore guideline enforcement |

## Running Tests

The project has extensive unit tests to ensure its functioning as expect. Build the project with testing enabled:

```bash
mkdir -p build
cd build
cmake -DHDBPP_TDB_BUILD_TESTS=ON ..
make
```

To run all unit tests, a postgresql database node is required with the project schema loaded up. To run all tests:

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
