# libhdbpp-timescale

Under construction... 

**Under development and not yet ready for deployment**

HDB++ backend library for the TimescaleDb extenstion to Postgresql. This library is loaded by libhdbpp to archive events from a Tango Controls system.

- [libhdbpp-timescale](#libhdbpp-timescale)
  - [Cloning](#Cloning)
  - [Dependencies](#Dependencies)
    - [Project Dependencies](#Project-Dependencies)
    - [Build Dependencies](#Build-Dependencies)
    - [Submodules](#Submodules)

## Cloning

Currently this project is configured to express its dependencies as submodules. This may change in future if there is time to explore, for example, the Meson build system. To successfully clone the project and all its dependencies use the following:

```bash
git clone --recurse-submodules https://github.com/tango-controls-hdbpp/libhdbpp-timescale.git
```

## Dependencies

### Project Dependencies

Ensure the development version of the dependencies are installed. These are as follows:

* Tango Controls 9 or higher.
* omniORB release 4 or higher.
* libzmq - libzmq3-dev or libzmq5-dev.
* libpq - Postgres C library (version?)

### Build Dependencies

If wishing to build the project, ensure the following dependencies are met:

* CMake 3.6 or higher
* C++14 compatible compiler (code base is using c++14)
* libpq-dev - Postgres C development library (version?)

### Submodules

* libhdbpp - Currently added to repo to aid building since this version has been modified build system that supplies a CMake header only target.. This will be turned into a pull request to libhdbpp.
* libpqxx - C++ wrapper around libpq providing a modern and clean interface.
* spdlog - modern logging system to debug the library
