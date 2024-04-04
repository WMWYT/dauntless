cmake_minimum_required(VERSION 3.17)

project(dauntless)

include(GNUInstallDirs)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(plugin)

install(FILES conf/dauntless.ini DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/dauntless")