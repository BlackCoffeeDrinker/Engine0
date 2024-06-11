set(PLATFORM_SRC
        DosMode13.hpp DosMode13.cpp
        main.cpp
        StdFile.cpp
        StdFile.hpp
        DosDetect.cpp
        DosDetect.hpp

        KeyboardDecoding/Keys.hpp
        KeyboardDecoding/KeyboardEventHandler.hpp
        KeyboardDecoding/KeyboardEventHandler.cpp
        KeyboardDecoding/KeyboardEventPauseHandler.hpp
        KeyboardDecoding/KeyboardEventHandlerResult.hpp
        KeyboardDecoding/KeyboardEventExtendedHandler.hpp
        KeyboardDecoding/KeyboardEventNormalHandler.hpp

        MouseInt33/MouseComDrv.cpp
        MouseInt33/MouseComDrv.hpp
        Signals.cpp
        Signals.hpp
        Timer.cpp
        Timer.hpp

        DosModeVesa.cpp
        DosModeVesa.hpp)

target_sources(Game PRIVATE ${PLATFORM_SRC})
