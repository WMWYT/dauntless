include_directories(${dauntless_SOURCE_DIR} 
                    ${dauntless_SOURCE_DIR}/src
                    ${dauntless_SOURCE_DIR}/include
                    ${dauntless_SOURCE_DIR}/lib)

set(LIB ../../lib/dauntless.c)

add_library(test SHARED test.c ${LIB})
