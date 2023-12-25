include_directories(${dauntless_SOURCE_DIR} 
                    ${dauntless_SOURCE_DIR}/src
                    ${dauntless_SOURCE_DIR}/include
                    ${dauntless_SOURCE_DIR}/lib)

# file(GLOB LIBDAUNTLESS *.c)

# add_library(libdauntless SHARED ${LIBDAUNTLESS})

# set_target_properties(libdauntless PROPERTIES
#     OUTPUT_NAME dauntless)

# install(TARGETS libdauntless
#         RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
#         ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#         LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")