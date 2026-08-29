target_sources(Engine00
        PRIVATE
        win31_platform.cpp
        EntryPoint.cpp
        Heap.cpp
        Heap.hpp
        Intrinsics.cpp
        Win32Types.hpp
        WinG.cpp
        WinG.hpp
        Win31Surface.cpp
        Win31Surface.hpp
        Keyboard.cpp
        Keyboard.hpp
        Mouse.cpp
        Mouse.hpp
        StdFile.cpp
        StdFile.hpp
        CreateSink.cpp
        Menu.cpp
        Menu.hpp
        Audio.cpp
        Audio.hpp
)

target_compile_definitions(Engine00 PUBLIC WIN31=1)

# LibcShim.cpp/PthreadStubs.cpp live in their own tiny static library rather
# than as regular Engine00 sources. The reason: gcc_eh/mingwex (linked further
# below) themselves reference symbols these two files provide (pthread_*,
# atan2/lround/lrint, _assert), but by the time the linker gets to gcc_eh it
# has already made its single left-to-right pass over libEngine00.a and won't
# revisit it for newly-discovered undefined symbols. Putting them in a
# separate archive lets us list that archive a second time *inside* the
# --start-group/--end-group below, so ld freely re-scans it -- listing
# Engine00 itself a second time isn't possible ($<TARGET_FILE:Engine00> used
# on Engine00's own target_link_libraries is a self-referential generator
# expression CMake rejects outright).
add_library(Win31LibC STATIC
        LibcShim.cpp
        PthreadStubs.cpp
)
target_compile_options(Win31LibC PRIVATE -m32 -fno-exceptions -fno-rtti -ffreestanding)

# Ensure Win32 import libs are on every consumer of Engine00 (executables).
# msvcrt-os / real msvcrt.dll is NOT linked here anymore: LibcShim.cpp (plus
# the malloc/free family in Heap.cpp and the memcpy/string helpers in
# Intrinsics.cpp) now implements every libc-style function libstdc++'s
# internals (string/vector resize, matherr-adjacent math helpers) and the
# vendored C libraries (lua, lodepng, inih) call into, all backed by the same
# HeapAlloc-based process heap. This avoids Win32s' well-known trouble
# resolving msvcrt.dll / api-ms-win-crt-*.dll imports entirely.
# The toolchain file still passes "-nodefaultlibs" (so gcc's driver-injected
# plain "-lmsvcrt" never sneaks back in), which means every other library the
# driver would normally add implicitly must be listed here too: mingwex (misc
# CRT helpers, incl. required by mingw's own pow/powi math-error-handling
# internals), mingw32 (provides __mingw_raise_matherr, the actual matherr
# handler mingwex calls into), and stdc++/gcc_eh/gcc (C++ runtime support,
# statically linked per -static-libstdc++/-static-libgcc in the toolchain
# file -- gcc_eh is required even under -fno-exceptions because libstdc++.a
# itself was built with SJLJ-based exception safety for its own string/vector
# resize internals, independent of our own code).
# NOTE: deliberately NOT linking the real "winpthread": its thread.c references
# the PE "_tls_used" TLS directory descriptor, which is normally only defined
# by the CRT startup object we exclude via -nostartfiles. Since Win32s has no
# real threading the engine relies on anyway (platform::CreateThread already
# just logs and returns platform::InvalidThreadId), Win31LibC/PthreadStubs.cpp
# provides minimal single-threaded pthread_key_create/pthread_once/mutex stubs
# instead, satisfying gcc_eh's SJLJ bookkeeping without needing real TLS.
# Wrapped in --start-group/--end-group: these libraries have circular
# references to each other's symbols (mingwex may need symbols now provided by
# Win31LibC, and vice versa), which a single left-to-right linker pass can't
# resolve otherwise. Win31LibC is deliberately listed a second time here (it
# is also linked normally, below, via the plain target_link_libraries call)
# so ld revisits it while resolving mingwex/gcc_eh's own undefined symbols.
target_link_libraries(Engine00 PUBLIC
        Win31LibC
        kernel32 user32 gdi32 winmm advapi32 shell32
        -Wl,--start-group
        $<TARGET_FILE:Win31LibC> mingwex mingw32 stdc++ gcc_eh gcc
        -Wl,--end-group
)

# Only pure CRT/heap stubs are compiled freestanding. Other backend TUs include
# Platform.hpp/Engine.hpp (hosted STL) and must not use -ffreestanding.
set_source_files_properties(
        Heap.cpp
        Intrinsics.cpp
        EntryPoint.cpp
        PROPERTIES COMPILE_OPTIONS "-ffreestanding"
)
