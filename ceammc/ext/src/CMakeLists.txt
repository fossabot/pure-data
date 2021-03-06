include(faust)
include(ceammc_pd.cmake)
include(cppcheck.cmake)

if(WITH_COVERAGE)
    if(APPLE)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        endif()
    else()
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        endif()
    endif()
endif()

if(WIN32 OR MSYS OR MINGW)
    list(APPEND CEAMMC_EXT_LINK_LIBRARIES puredata-core)
endif()

set(CEAMMC_EXT_LINK_LIBRARIES ceammc_array ceammc_base ceammc_conv ceammc_data ceammc_dyn ceammc_env
    ceammc_flt ceammc_flow ceammc_fx ceammc_hw ceammc_lfo ceammc_list ceammc_math ceammc_midi
    ceammc_misc ceammc_noise ceammc_osc ceammc_preset ceammc_path ceammc_proto ceammc_random ceammc_spat
    ceammc_symbol ceammc_synth ceammc_system ceammc_string ceammc_tl ceammc_ui ceammc_vector
    ceammc_sound ceammc_window
    ceammc_core)

add_subdirectory(lib)
include_directories(lib)
include_directories(${CMAKE_SOURCE_DIR}/src)

remove_definitions(-DPD_INTERNAL)
add_definitions(-DPD)

add_subdirectory(array)
add_subdirectory(base)
add_subdirectory(conv)
add_subdirectory(data)
add_subdirectory(dyn)
add_subdirectory(env)
add_subdirectory(exp)
add_subdirectory(flow)
add_subdirectory(flt)
add_subdirectory(fx)
add_subdirectory(hw)
add_subdirectory(lfo)
add_subdirectory(list)
add_subdirectory(math)
add_subdirectory(midi)
add_subdirectory(misc)
add_subdirectory(noise)
add_subdirectory(osc)
add_subdirectory(preset)
add_subdirectory(path)
add_subdirectory(proto)
add_subdirectory(random)
add_subdirectory(rtmidi)
add_subdirectory(spat)
add_subdirectory(string)
add_subdirectory(symbol)
add_subdirectory(synth)
add_subdirectory(system)
add_subdirectory(tl)
add_subdirectory(ui)
add_subdirectory(vector)
add_subdirectory(window)

pd_add_extension(NAME ceammc
    FILES ceammc.cpp mod_init.cpp
    INTERNAL True
    LIBRARY ceammc
    LINK ${CEAMMC_EXT_LINK_LIBRARIES})

add_custom_target(ceammc_lib_src SOURCES gen_lib.sh)
