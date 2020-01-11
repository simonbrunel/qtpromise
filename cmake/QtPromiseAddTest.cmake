function(qtpromise_add_test NAME)
    cmake_parse_arguments(_ARG "" "" "SOURCES;LIBRARIES" ${ARGN})

    set(_TARGET qtpromise.tests.auto.${NAME})

    add_executable(${_TARGET} ${_ARG_SOURCES})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${_TARGET} gcov)
        target_compile_options(${_TARGET}
            PRIVATE
                -fprofile-arcs
                -ftest-coverage
                -O0
                -g
        )
    endif()

    target_link_libraries(${_TARGET}
        Qt5::Test
        qtpromise
        qtpromise.tests.utils
        ${_ARG_LIBRARIES}
    )

    add_test(NAME ${_TARGET}
        COMMAND ${_TARGET}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()
