if(NOT TARGET TangoInterfaceLibrary)

    # Ensure pkg-config is installed 
    find_package(PkgConfig REQUIRED)

    # Now search for the tango.pc file, this is a required dependency
    message(STATUS "Search for TANGO package config...")
    pkg_search_module(TANGO REQUIRED tango>=9.2.5)
    message(STATUS "Found tango version ${TANGO_VERSION} at ${TANGO_PREFIX}")

    include(FindLibraries)
    find_libraries(LIBRARIES ${TANGO_LIBRARIES} SEARCH_PATHS ${TANGO_LIBRARY_DIRS})

    # Create an interface library to represent the tango linkage
    add_library(TangoInterfaceLibrary INTERFACE)
    #target_include_directories(TangoInterfaceLibrary INTERFACE $<BUILD_INTERFACE:${TANGO_INCLUDE_DIRS}>)
    #target_link_libraries(TangoInterfaceLibrary INTERFACE $<BUILD_INTERFACE:${FOUND_LIBRARIES}>)
    #target_compile_options(TangoInterfaceLibrary INTERFACE $<BUILD_INTERFACE:${TANGO_CFLAGS}>)
    set_target_properties(TangoInterfaceLibrary 
        PROPERTIES 
            INTERFACE_INCLUDE_DIRECTORIES "${TANGO_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${FOUND_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${TANGO_CFLAGS}")

    message(STATUS "Configured Tango Interface for TANGO version ${TANGO_VERSION}")
endif(NOT TARGET TangoInterfaceLibrary)