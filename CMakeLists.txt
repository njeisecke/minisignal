cmake_minimum_required(VERSION 3.20)

project(minisignal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(minisignal main.cpp
    connection.h
    signal.h
    main.cpp
)

include(GNUInstallDirs)
install(TARGETS minisignal
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
