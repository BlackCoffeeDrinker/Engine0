
target_link_libraries(Engine00 PUBLIC SDL3::SDL3-static)
target_sources(Engine00 PRIVATE
        main.cpp
        SDL_KeyboardSystem.cpp
        SDL_KeyboardSystem.h
        Logger.cpp
        Logger.h
        Bitmap.cpp
        Bitmap.hpp
        StdFile.cpp
        StdFile.hpp
        string_2_wstring.hpp
        CreateSink.cpp
)
