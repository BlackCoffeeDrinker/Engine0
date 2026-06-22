target_sources(Engine00
        PRIVATE
        MetalSurface.mm
        CreateSink.cpp
        PlatformName.cpp
        ApplePrivateInclude.h
        MetalSurface.h
        Apple_KeyboardSystem.h
        main.mm
        Apple_KeyboardSystem.mm
        OpenStream.cpp
)

target_link_libraries(Engine00 PUBLIC
        "$<LINK_LIBRARY:FRAMEWORK,CoreFoundation>"
        "$<LINK_LIBRARY:FRAMEWORK,Cocoa>"
        "$<LINK_LIBRARY:FRAMEWORK,QuartzCore>"
        "$<LINK_LIBRARY:FRAMEWORK,Metal>"
)
