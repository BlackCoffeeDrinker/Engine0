
target_sources(Platform PRIVATE
        main.cpp
        StdFile.cpp
        StdFile.hpp
        string_2_wstring.hpp
        Logger_Sink.cpp
        SdlBitmap.cpp
        SdlBitmap.hpp
)

target_link_libraries(Platform
        PUBLIC SDL2::SDL2-static
)

