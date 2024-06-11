
target_sources(Engine00 PRIVATE
        main.cpp
        StdFile.cpp
        StdFile.hpp
        string_2_wstring.hpp
        Logger_Sink.cpp
        SdlBitmap.cpp
        SdlBitmap.hpp
)

target_link_libraries(Engine00
        PUBLIC SDL2::SDL2-static
)

