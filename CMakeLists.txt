cmake_minimum_required(VERSION 3.12)
include(cmake/utils.cmake)
project_guarded(ccore)
project(ccore
    VERSION 1.12.0
    DESCRIPTION "Data structures and utilities library for C programmes"
    HOMEPAGE_URL "https://github.com/amyinorbit/ccore"
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(CCORE_BUILD_DEMO "Build ccore executable demo" ON)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME}
STATIC
    src/format.c
    src/list.c
    src/log.c
    src/math.c
    src/memory.c
    src/table.c
    src/value.c
    src/filesystem.c
    src/debug.c
    src/except.c
    src/tpool.c
    src/cfg.c
    src/time.c
    src/run_loop.c
)

# add alias so the project can be uses with add_subdirectory
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CPack)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC m Threads::Threads dbghelp psapi)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC m Threads::Threads)
endif()

# Adding the install interface generator expression makes sure that the include
# files are installed to the proper location (provided by GNUInstallDirs)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
set_target_properties(${PROJECT_NAME}
PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

# locations are provided by GNUInstallDirs
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

write_basic_package_version_file("${PROJECT_NAME}-config-version.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION include)

if(CCORE_BUILD_DEMO)
    add_subdirectory(demo)
endif()
