
macro(e0_supports_sanitizers)
    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
        set(SUPPORTS_UBSAN ON)
    else ()
        set(SUPPORTS_UBSAN OFF)
    endif ()

    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(SUPPORTS_ASAN OFF)
    else ()
        set(SUPPORTS_ASAN ON)
    endif ()
endmacro()


macro(e0_setup_options)
    option(E0_ENABLE_HARDENING "Enable hardening" ON)
    option(E0_ENABLE_COVERAGE "Enable coverage reporting" OFF)
    cmake_dependent_option(
            E0_ENABLE_GLOBAL_HARDENING
            "Attempt to push hardening options to built dependencies"
            ON
            E0_ENABLE_HARDENING
            OFF)

    e0_supports_sanitizers()

    if (NOT PROJECT_IS_TOP_LEVEL OR E0_PACKAGING_MAINTAINER_MODE)
        option(E0_ENABLE_IPO "Enable IPO/LTO" OFF)
        option(E0_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
        option(E0_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(E0_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
        option(E0_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(E0_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
        option(E0_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(E0_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
        option(E0_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(E0_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
        option(E0_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
        option(E0_ENABLE_PCH "Enable precompiled headers" OFF)
        option(E0_ENABLE_CACHE "Enable ccache" OFF)
    else ()
        option(E0_ENABLE_IPO "Enable IPO/LTO" ON)
        option(E0_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
        option(E0_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(E0_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
        option(E0_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(E0_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
        option(E0_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" ON)
        option(E0_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" ON)
        option(E0_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(E0_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
        option(E0_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
        option(E0_ENABLE_PCH "Enable precompiled headers" OFF)
        option(E0_ENABLE_CACHE "Enable ccache" ON)
    endif ()

    if (NOT PROJECT_IS_TOP_LEVEL)
        mark_as_advanced(
                E0_ENABLE_IPO
                E0_WARNINGS_AS_ERRORS
                E0_ENABLE_USER_LINKER
                E0_ENABLE_SANITIZER_ADDRESS
                E0_ENABLE_SANITIZER_LEAK
                E0_ENABLE_SANITIZER_UNDEFINED
                E0_ENABLE_SANITIZER_THREAD
                E0_ENABLE_SANITIZER_MEMORY
                E0_ENABLE_UNITY_BUILD
                E0_ENABLE_CLANG_TIDY
                E0_ENABLE_CPPCHECK
                E0_ENABLE_COVERAGE
                E0_ENABLE_PCH
                E0_ENABLE_CACHE)
    endif ()

    e0_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
    if (LIBFUZZER_SUPPORTED AND (E0_ENABLE_SANITIZER_ADDRESS OR E0_ENABLE_SANITIZER_THREAD OR E0_ENABLE_SANITIZER_UNDEFINED))
        set(DEFAULT_FUZZER ON)
    else ()
        set(DEFAULT_FUZZER OFF)
    endif ()

    option(E0_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(e0_global_options)
    if (E0_ENABLE_IPO)
        include(cmake/InterproceduralOptimization.cmake)
        e0_enable_ipo()
    endif ()

    e0_supports_sanitizers()

    if (E0_ENABLE_HARDENING AND E0_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if (NOT SUPPORTS_UBSAN
                OR E0_ENABLE_SANITIZER_UNDEFINED
                OR E0_ENABLE_SANITIZER_ADDRESS
                OR E0_ENABLE_SANITIZER_THREAD
                OR E0_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else ()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif ()
        message("${E0_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${E0_ENABLE_SANITIZER_UNDEFINED}")
        e0_enable_hardening(e0_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif ()
endmacro()

macro(e0_local_options)
    if (PROJECT_IS_TOP_LEVEL)
        include(cmake/StandardProjectSettings.cmake)
    endif ()

    add_library(e0_warnings INTERFACE)
    add_library(e0_options INTERFACE)

    include(cmake/CompilerWarnings.cmake)
    e0_set_project_warnings(
            e0_warnings
            ${E0_WARNINGS_AS_ERRORS}
            ""
            ""
            ""
            "")

    if (E0_ENABLE_USER_LINKER)
        include(cmake/Linker.cmake)
        e0_configure_linker(e0_options)
    endif ()

    include(cmake/Sanitizers.cmake)
    e0_enable_sanitizers(
            e0_options
            ${E0_ENABLE_SANITIZER_ADDRESS}
            ${E0_ENABLE_SANITIZER_LEAK}
            ${E0_ENABLE_SANITIZER_UNDEFINED}
            ${E0_ENABLE_SANITIZER_THREAD}
            ${E0_ENABLE_SANITIZER_MEMORY})

    set_target_properties(e0_options PROPERTIES UNITY_BUILD ${E0_ENABLE_UNITY_BUILD})

    if (E0_ENABLE_PCH)
        target_precompile_headers(
                e0_options
                INTERFACE
                <vector>
                <string>
                <utility>)
    endif ()

    if (E0_ENABLE_CACHE)
        include(cmake/Cache.cmake)
        e0_enable_cache()
    endif ()

    include(cmake/StaticAnalyzers.cmake)
    if (E0_ENABLE_CLANG_TIDY)
        e0_enable_clang_tidy(e0_options ${E0_WARNINGS_AS_ERRORS})
    endif ()

    if (E0_ENABLE_CPPCHECK)
        e0_enable_cppcheck(${E0_WARNINGS_AS_ERRORS} "" # override cppcheck options
        )
    endif ()

    if (E0_ENABLE_COVERAGE)
        include(cmake/Tests.cmake)
        e0_enable_coverage(e0_options)
    endif ()

    if (E0_WARNINGS_AS_ERRORS)
        check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
        if (LINKER_FATAL_WARNINGS)
            # This is not working consistently, so disabling for now
            # target_link_options(e0_options INTERFACE -Wl,--fatal-warnings)
        endif ()
    endif ()

    if (E0_ENABLE_HARDENING AND NOT E0_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if (NOT SUPPORTS_UBSAN
                OR E0_ENABLE_SANITIZER_UNDEFINED
                OR E0_ENABLE_SANITIZER_ADDRESS
                OR E0_ENABLE_SANITIZER_THREAD
                OR E0_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else ()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif ()
        e0_enable_hardening(e0_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif ()

endmacro()