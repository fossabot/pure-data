set(SYNTH_SOURCES)

macro(ceammc_synth_extension name)
    list(APPEND SYNTH_SOURCES "synth_${name}.cpp")
    ceammc_faust_gen(synth ${name})
endmacro()

ceammc_synth_extension(risset_tone)

add_library(ceammc_synth STATIC mod_synth.cpp ${SYNTH_SOURCES})
set_target_properties(ceammc_synth PROPERTIES COMPILE_DEFINITIONS "FAUST_MACRO")
if(UNIX)
    set_target_properties(ceammc_synth PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
