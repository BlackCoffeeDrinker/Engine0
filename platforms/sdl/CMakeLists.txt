target_sources(Engine00
        PRIVATE main.cpp StdFile.cpp StdFile.hpp string_2_wstring.hpp Engine_OpenStream.cpp Logger_Sink.cpp
)

target_link_libraries(Engine00 PUBLIC SDL2::SDL2-static)
