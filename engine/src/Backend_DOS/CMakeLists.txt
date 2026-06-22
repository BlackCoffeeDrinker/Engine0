

add_subdirectory(Video)

target_sources(Engine00
        PRIVATE
        dos_platform.cpp
        interrupt.cpp
        interrupt.hpp
        timer.cpp
        timer.hpp
        OSType.cpp
        OSType.hpp
        PlatformThread.cpp
        PlatformThread.hpp
        
        StdFile.hpp
        StdFile.cpp

        CreateSink.cpp

        memory.cpp
        memory.hpp

        keyboard.cpp
        keyboard.hpp
        mouse.cpp
        mouse.h
)
