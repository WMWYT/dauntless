include_directories(${dauntless_SOURCE_DIR} 
                    ${dauntless_SOURCE_DIR}/src
                    ${dauntless_SOURCE_DIR}/include
                    ${dauntless_SOURCE_DIR}/3rd/uthash
                    ${dauntless_SOURCE_DIR}/3rd/iniparser)

file(GLOB D_SOURCES *.c)

set(LIB_SRC 
    ../3rd/iniparser/iniparser.c
    ../3rd/iniparser/dictionary.c
)

add_executable(dauntless ${D_SOURCES} ${LIB_SRC})

install(TARGETS dauntless RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
