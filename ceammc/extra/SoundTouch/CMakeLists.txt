set(SOUNDTOUCH_SRC
    soundtouch/source/SoundTouch/AAFilter.cpp
    soundtouch/source/SoundTouch/BPMDetect.cpp
    soundtouch/source/SoundTouch/FIFOSampleBuffer.cpp
    soundtouch/source/SoundTouch/FIRFilter.cpp
    soundtouch/source/SoundTouch/InterpolateCubic.cpp
    soundtouch/source/SoundTouch/InterpolateLinear.cpp
    soundtouch/source/SoundTouch/InterpolateShannon.cpp
    soundtouch/source/SoundTouch/PeakFinder.cpp
    soundtouch/source/SoundTouch/RateTransposer.cpp
    soundtouch/source/SoundTouch/SoundTouch.cpp
    soundtouch/source/SoundTouch/TDStretch.cpp
    soundtouch/source/SoundTouch/cpu_detect_x86.cpp
    #soundtouch/source/SoundTouch/mmx_optimized.cpp
    soundtouch/source/SoundTouch/sse_optimized.cpp
)

if(WIN32 OR MSYS)
    add_definitions(-DSOUNDTOUCH_DISABLE_X86_OPTIMIZATIONS)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/soundtouch/include)
add_library(soundtouch STATIC ${SOUNDTOUCH_SRC})

if(UNIX)
    set_target_properties(soundtouch PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

add_subdirectory(pd)
