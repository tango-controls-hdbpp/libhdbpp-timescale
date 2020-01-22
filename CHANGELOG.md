# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.11.1] - 2020-01-22

### Fixed

- Fix potential crash in AttributeName::tangoHostWithDomain() [#8](https://github.com/tango-controls-hdbpp/libhdbpp-timescale/pull/8)

## [0.11.0] - 2020-01-21

### Added

- Added support for time to live feature. 

### Changed

- Excluded large parts of PqxxExtension.hpp from liniting (since it conforms to pqxx coding standards)

## [0.10.0] - 2019-12-06

### Added

- Configuration documentation 
- Added support for syslog logging.
- Added support for storing data via insert strings rather than prepared statements:
  - Tango string spectrum types are always stored via insert strings to remove escape characters
  - This work provides the basis for future batch saving of data events.

### Fixed

- Close logging down in destructor so linked device server can be restarted.
- All unit tests now pass!

### Changed

- Completely redone DbConnectionTest unit tests
  - Speeds up execution of tests
  - Prevents database deadlock due to repeated rapid truncation of tables.
- Build system correctly produces a major version shared object
- Removed Clang path from build (CMake checks PATH)
- Corrected static library build
- Install now places header in include/hdb++/
- Entire library now uses the global default logger from spdlog.
- Updated spdlog submodule to release v1.4.2
- Modern code via clang tidy
- DbConnection::storeParameterEvent() no longer asserts on empty parameters, it now warns in the log.

## [0.9.1] - 2019-07-18

### Added

- Google benchmark sub-module for micro benchmarking. This will enabled future optimisation
  - Benchmark tests for QueryBuilder

### Fixed

- Fix to build system - can build without Clang enabled now.
- Updates/corrections to docs

## [0.9.0] - 2019-07-12

### Added

- Pre v1 release of the library. 
  - This is an almost complete version of the library. 
  - Next couple of versions will complete the library before a v1 release.
