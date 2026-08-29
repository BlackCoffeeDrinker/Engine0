set(CMAKE_SYSTEM_NAME Windows)

set(WIN31 TRUE)

set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
set(CMAKE_IMPORT_LIBRARY_PREFIX "lib")
set(CMAKE_IMPORT_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_LINK_LIBRARY_SUFFIX "")
set(CMAKE_DL_LIBS "")

set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")


# IMPORTANT: find_program(<VAR> ...) refuses to overwrite <VAR> if it is
# already present in the CMake cache -- it just leaves the stale value alone.
# This bit us in practice: an IDE-managed build directory (e.g. a CLion CMake
# profile) can end up with CMAKE_C_COMPILER/CMAKE_CXX_COMPILER cached to the
# wrong architecture's compiler (observed: x86_64-w64-mingw32-g++ instead of
# i686-w64-mingw32-g++, both installed side by side by Homebrew's mingw-w64
# under toolchain-x86_64/ and toolchain-i686/), and every subsequent
# reconfigure silently keeps using the wrong 64-bit compiler -- it still
# "succeeds" (since -m32 is a valid flag for gcc's driver either way), but the
# 64-bit toolchain's import libraries (libkernel32.a, libgcc_eh.a, etc.) are
# not multilib and get rejected by the linker as "incompatible" for a 32-bit
# target ("cannot find -lkernel32: Invalid argument"). To make this toolchain
# file self-healing against that kind of stale/mismatched cache, forcibly
# discard any previously cached compiler value that isn't the i686 one before
# searching again.
foreach(_win31_compiler_var CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_RC_COMPILER
        CMAKE_AR CMAKE_RANLIB CMAKE_NM CMAKE_OBJCOPY CMAKE_STRIP)
    if(DEFINED CACHE{${_win31_compiler_var}} AND NOT "$CACHE{${_win31_compiler_var}}" MATCHES "i686-w64-mingw32")
        unset(${_win31_compiler_var} CACHE)
    endif()
endforeach()

find_program(CMAKE_C_COMPILER NAMES "i686-w64-mingw32-gcc" REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES "i686-w64-mingw32-g++" REQUIRED)
find_program(CMAKE_RC_COMPILER NAMES "i686-w64-mingw32-windres" "windres")
find_program(CMAKE_AR NAMES "i686-w64-mingw32-ar" "i686-w64-mingw32-gcc-ar")
find_program(CMAKE_RANLIB NAMES "i686-w64-mingw32-ranlib" "i686-w64-mingw32-gcc-ranlib")
find_program(CMAKE_NM NAMES "i686-w64-mingw32-nm")
find_program(CMAKE_OBJCOPY NAMES "i686-w64-mingw32-objcopy")
find_program(CMAKE_STRIP NAMES "i686-w64-mingw32-strip")

# Belt-and-suspenders: fail loudly and clearly instead of silently linking a
# broken 64-bit executable if, for any reason, the resolved compiler still
# isn't the i686 one (e.g. only x86_64-w64-mingw32-gcc is on PATH at all).
if(NOT CMAKE_C_COMPILER MATCHES "i686-w64-mingw32" OR NOT CMAKE_CXX_COMPILER MATCHES "i686-w64-mingw32")
    message(FATAL_ERROR
            "Win31/Win32s toolchain requires the i686-w64-mingw32 (32-bit) "
            "cross-compiler, but resolved CMAKE_C_COMPILER='${CMAKE_C_COMPILER}' / "
            "CMAKE_CXX_COMPILER='${CMAKE_CXX_COMPILER}'. If you're seeing this "
            "after previously configuring this build directory with a different "
            "compiler, delete the build directory and reconfigure from scratch "
            "-- CMake caches the compiler path and won't self-correct otherwise.")
endif()

execute_process(COMMAND "${CMAKE_C_COMPILER}" -print-search-dirs
        RESULT_VARIABLE CC_SEARCH_DIRS_RESULT
        OUTPUT_VARIABLE CC_SEARCH_DIRS_OUTPUT)

if(CC_SEARCH_DIRS_RESULT)
    message(FATAL_ERROR "Could not determine search dirs")
endif()

string(REGEX MATCH ".*libraries: (.*).*" CC_SD_LIBS "${CC_SEARCH_DIRS_OUTPUT}")
string(STRIP "${CMAKE_MATCH_1}" CC_SEARCH_DIRS)
string(REPLACE ":" ";" CC_SEARCH_DIRS "${CC_SEARCH_DIRS}")

foreach(CC_SEARCH_DIR ${CC_SEARCH_DIRS})
    if(CC_SEARCH_DIR MATCHES "=.*")
        string(REGEX MATCH "=(.*)" CC_LIB "${CC_SEARCH_DIR}")
        set(CC_SEARCH_DIR "${CMAKE_MATCH_1}")
    endif()
    if(IS_DIRECTORY "${CC_SEARCH_DIR}")
        if(IS_DIRECTORY "${CC_SEARCH_DIR}/../include" OR IS_DIRECTORY "${CC_SEARCH_DIR}/../lib" OR IS_DIRECTORY "${CC_SEARCH_DIR}/../bin")
            list(APPEND CC_ROOTS "${CC_SEARCH_DIR}/..")
        else()
            list(APPEND CC_ROOTS "${CC_SEARCH_DIR}")
        endif()
    endif()
endforeach()

list(APPEND CMAKE_FIND_ROOT_PATH ${CC_ROOTS})

# search for programs in the host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries, headers and packages in the target directories
if(NOT DEFINED CACHE{CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
endif()
if(NOT DEFINED CACHE{CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()
if(NOT DEFINED CACHE{CMAKE_FIND_ROOT_PATH_MODE_PACKAGE})
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

# Win32s target flags.
# NOTE: Do NOT put -ffreestanding in the global flags. GCC 16's libstdc++ headers
# refuse to compile in freestanding mode (#error in <bits/requires_hosted.h>), and
# the engine + vendors (lodepng, lua, zlib wrappers) require a hosted STL.
# We use -nostartfiles (not -nostdlib) so default CRT *startup objects* are omitted
# (our Backend_Win31 EntryPoint supplies WinMainCRTStartup) while still linking
# libstdc++/mingw CRT/import libs that the rest of the engine needs.
set(WIN31_COMMON_FLAGS "-march=i386 -mtune=i386 -m32 -fno-exceptions -fno-rtti -fno-threadsafe-statics")
set(CMAKE_C_FLAGS_INIT "-march=i386 -mtune=i386 -m32")
set(CMAKE_CXX_FLAGS_INIT "${WIN31_COMMON_FLAGS}")

# Custom entry point + Windows 3.10 subsystem. -nostartfiles drops crt0; libs stay.
# Win32s (Windows 3.1's Win32 loader) predates PE features like ASLR/NX and its
# selector-based memory manager chokes on oversized images, so:
#  - --disable-dynamicbase / --disable-nxcompat keep the PE DllCharacteristics flags
#    limited to what a 1993-era loader understands (a set DYNAMIC_BASE/NX_COMPAT bit
#    is a known cause of the Win32s program loader's "Unexpected error: 21").
#    --disable-high-entropy-va is a 64-bit-only ld option, not applicable to i686.
#  - --strip-all drops DWARF/debug sections from the linked image; leaving them in
#    (e.g. a RelWithDebInfo/Debug build) can balloon SizeOfImage to 10+ MB, which
#    Win32s' limited selector pool cannot map on 8MB-class target machines.
#  - --major-os-version,3 --minor-os-version,10 stamp the PE optional header's
#    "OS Version" as 3.10, matching what the real Win32s loader expects; a linker
#    default of 6.x (Vista+) here is another documented cause of the Win32s
#    program loader's "Unexpected error: 21".
#  - -static -static-libgcc -static-libstdc++ statically embed libgcc/libstdc++/
#    libwinpthread into the .exe instead of importing libgcc_s_sjlj-1.dll,
#    libstdc++-6.dll and libwinpthread-1.dll at load time -- none of those DLLs
#    ship with Windows 3.1/Win32s, so a dynamic dependency on them is a load-time
#    failure on real hardware even when the exe otherwise looks fine under Wine
#    or a modern OS.
# -nodefaultlibs: gcc's own driver spec silently appends the plain "-lmsvcrt"
# (UCRT-forwarding) at the end of every link regardless of what libraries we
# already listed explicitly, which is how api-ms-win-crt-*.dll imports kept
# reappearing even after switching to "msvcrt-os". This suppresses that
# driver-injected default library list entirely; engine/src/Backend_Win31/
# CMakeLists.txt's target_link_libraries(...) call is responsible for listing
# every library actually needed (msvcrt-os, mingwex, stdc++, gcc/gcc_eh) since
# none of them are implicitly added anymore.
set(WIN31_LINK_FLAGS "-Wl,--subsystem,windows:3.10 -Wl,-e,_WinMainCRTStartup@0 -Wl,--major-os-version,3 -Wl,--minor-os-version,10 -nostartfiles -nodefaultlibs -static -static-libgcc -static-libstdc++ -Wl,--disable-dynamicbase -Wl,--disable-nxcompat -Wl,--strip-all")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${WIN31_LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${WIN31_LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${WIN31_LINK_FLAGS}")

# NOTE: do NOT try to override CMAKE_C_STANDARD_LIBRARIES(_INIT)/
# CMAKE_CXX_STANDARD_LIBRARIES(_INIT) here to swap in "msvcrt-os" (the genuine
# legacy msvcrt.dll import lib) in place of the default "msvcrt" (which is itself
# a forwarder onto the modern api-ms-win-crt-*.dll UCRT "API Set" DLLs that Win32s
# cannot resolve): CMake's own Modules/Platform/Windows-GNU.cmake unconditionally
# re-sets both _INIT variables (with no "if not already set" guard) *after* this
# toolchain file runs, discarding anything set here either way. The actual fix
# lives in engine/src/Backend_Win31/CMakeLists.txt's target_link_libraries(...)
# call, which adds "msvcrt-os" after Engine00's own object archive in the real
# link command -- see the comment there for the full rationale.
set(CMAKE_C_STANDARD_LIBRARIES "-lkernel32 -luser32 -lgdi32 -lwinmm -ladvapi32 -lshell32")
set(CMAKE_CXX_STANDARD_LIBRARIES "-lkernel32 -luser32 -lgdi32 -lwinmm -ladvapi32 -lshell32 -lstdc++ -lgcc")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

add_compile_definitions(WIN31=1 _WIN32=1 WIN32=1)
