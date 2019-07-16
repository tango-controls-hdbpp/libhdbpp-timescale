# libhdbpp-timescale

[![TangoControls](https://img.shields.io/badge/-Tango--Controls-7ABB45.svg?style=flat&logo=%20data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAACAAAAAkCAYAAADo6zjiAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAsFJREFUWIXtl01IFVEYht9zU%2FvTqOxShLowlOgHykWUGEjUKqiocB1FQURB0KJaRdGiaFM7gzZRLWpTq2olhNQyCtpYCP1gNyIoUTFNnxZzRs8dzvw4Q6564XLnfOf73vedc2a%2BmZEKALgHrC3CUUR8CxZFeEoFalsdM4uLmMgFoIlZLJp3A9ZE4S2oKehhlaR1BTnyg2ocnW%2FxsxEDhbYij4EPVncaeASMAavnS%2FwA8NMaqACNQCew3f4as3KZOYh2SuqTVJeQNiFpn6QGSRVjTH9W%2FiThvcCn6H6n4BvQDvQWFT%2BSIDIFDAKfE3KOAQeBfB0XGPeQvgE67P8ZoB44DvTHmFgJdOQRv%2BUjc%2BavA9siNTWemgfA3TwGquCZ3w8szFIL1ALngIZorndvgJOR0GlP2gtJkzH%2Bd0fGFxW07NqY%2FCrx5QRXcYjbCbmxF1dkBSbi8kpACah3Yi2Sys74cVyxMWY6bk5BTwgRe%2BYlSzLmxNpU3aBeJogk4XWWpJKUeiap3RJYCpQj4QWZDQCuyIAk19Auj%2BAFYGZZjTGjksaBESB8P9iaxUBIaJzjZcCQcwHdj%2BS2Al0xPOeBYYKHk4vfmQ3Y8YkIwRUb7wQGU7j2ePrA1URx93ayd8UpD8klyPbSQfCOMIO05MbI%2BDvwBbjsMdGTwlX21AAMZzEerkaI9zFkP4AeYCPBg6gNuEb6I%2FthFgN1KSQupqzoRELOSed4DGiJala1UmOMr2U%2Bl%2FTWEy9Japa%2Fy41IWi%2FJ3d4%2FkkaAw0Bz3AocArqApwTvet3O3GbgV8qqjAM7bf4N4KMztwTodcYVyelywKSCD5V3xphNXoezuTskNSl4bgxJ6jPGVJJqbN0aSV%2Bd0M0aO7FCs19Jo2lExphXaTkxdRVgQFK7DZVDZ8%2BcpdmQh3wuILh7ut3AEyt%2B51%2BL%2F0cUfwFOX0t0StltmQAAAABJRU5ErkJggg%3D%3D)](http://www.tango-controls.org) [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![](https://img.shields.io/github/release/tango-controls-hdbpp/libhdbpp-timescale.svg)](https://github.com/tango-controls-hdbpp/libhdbpp-timescale/releases)

- [libhdbpp-timescale](#libhdbpp-timescale)
  - [Cloning](#Cloning)
  - [Bug Reports + Feature Requests](#Bug-Reports--Feature-Requests)
  - [Documentation](#Documentation)
  - [Building](#Building)
  - [Installing](#Installing)
  - [License](#License)

HDB++ backend library for the TimescaleDb extenstion to Postgresql. This library is loaded by libhdbpp to archive events from a Tango Controls system. Currently in a pre v1 release phase.

The library requires a correctly configured [TimescaleDb](https://www.timescale.com/) installation. 

The library has been build against a number of other projects, these have been integrated into the repository as sub modules. This may change at a later date if the build system is improved. Current sub modules are:

* libpqxx - Modern C++ Postgresql library
* spdlog - Logging system
* Catch2 - Unit test subsystem

## Cloning

Currently this project is configured to express its dependencies as submodules. This may change in future if there is time to explore, for example, the Meson build system. To successfully clone the project and all its dependencies use the following:

```bash
git clone --recurse-submodules https://github.com/tango-controls-hdbpp/libhdbpp-timescale.git
```

## Bug Reports + Feature Requests

Please file the bug reports and feature requests in the issue tracker

## Documentation

* See the [doc](doc/README.md) directoory for documentation on various topics, including building and installing.
* General documentations on the Tango HDB system can be found [here](http://tango-controls.readthedocs.io/en/latest/administration/services/hdbpp/index.html#hdb-an-archiving-historian-service)

## Building

See [Build.md](doc/build.md) in the doc folder

## Installing

See [Install.md](doc/install.md) in the doc folder

## License

The source code is released under the LGPL3 license and a copy of this license is provided with the code.