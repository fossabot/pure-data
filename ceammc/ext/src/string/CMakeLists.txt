set(STRING_SOURCES datatype_string.cpp)
macro(ceammc_string_extension name)
    list(APPEND STRING_SOURCES "string_${name}.cpp")
endmacro()

ceammc_string_extension(contains)
ceammc_string_extension(ends_with)
ceammc_string_extension(equal)
ceammc_string_extension(format)
ceammc_string_extension(join)
ceammc_string_extension(length)
ceammc_string_extension(remove)
ceammc_string_extension(replace)
ceammc_string_extension(split)
ceammc_string_extension(starts_with)
ceammc_string_extension(str)
ceammc_string_extension(to_symbol)
ceammc_string_extension(substr)

add_library(ceammc_string STATIC mod_string.cpp ${STRING_SOURCES})
if(UNIX)
    set_target_properties(ceammc_string PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
