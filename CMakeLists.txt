cmake_minimum_required(VERSION 3.8)

if(DEFINED PROJECT_NAME)
    set(SUBPROJECT ON)
endif()

project(qtpromise VERSION 0.6.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Qt5 5.6.0 REQUIRED COMPONENTS Core)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qtpromise INTERFACE)
add_library(qtpromise::qtpromise ALIAS qtpromise)

target_link_libraries(qtpromise INTERFACE Qt5::Core)
target_include_directories(qtpromise
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qtpromise/include>"
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_NO_KEYWORDS
)

# https://github.com/simonbrunel/qtpromise/issues/10
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
    add_compile_options(
        -Werror
        -Wpedantic
        -Wall
        -Wextra

        -Wconversion
        -Wdouble-promotion
        -Wformat=2
        -Wlogical-op
        -Wmissing-noreturn
        -Wold-style-cast
#        -Wshadow                   # disabled due to many findings in the current code
        -Wsign-conversion
        -Wswitch-default
        -Wunused-local-typedefs
#        -Wuseless-cast             # disabled due to Qt's moc warnings

        -pedantic-errors
    )

    # https://github.com/Barro/compiler-warnings/blob/master/gcc/warnings-gcc-6.txt
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6)
        add_compile_options(
            -Wduplicated-cond
        )
    endif()

    # https://github.com/Barro/compiler-warnings/blob/master/gcc/warnings-gcc-7.txt
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7)
        add_compile_options(
            -Wduplicated-branches
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # https://clang.llvm.org/docs/DiagnosticsReference.html
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic

        -Wsuggest-destructor-override
        -Wsuggest-override
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # https://docs.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level
    add_compile_options(
        /WX
    )
endif()

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "qtpromiseConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/qtpromiseConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/qtpromiseConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qtpromise/cmake
)

install(
    EXPORT ${PROJECT_NAME}_Targets
    FILE qtpromiseTargets.cmake
    NAMESPACE qtpromise::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qtpromise/cmake
)

install(
    FILES "${PROJECT_BINARY_DIR}/qtpromiseConfig.cmake"
          "${PROJECT_BINARY_DIR}/qtpromiseConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qtpromise/cmake
    )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/qtpromise
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtpromise/src)
install(FILES ${PROJECT_SOURCE_DIR}/include/QtPromise
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtpromise/include)


if(NOT SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()
