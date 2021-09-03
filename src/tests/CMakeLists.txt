set(TARGET messenger_tests)

set(SOURCES
    messenger_tests.cpp
)

if (UNIX)
    find_package (Threads)
endif()

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

#set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${TARGET})

add_executable(${TARGET} ${SOURCES})

set_target_properties(${TARGET} PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    server
    client
)

target_precompile_headers(${TARGET} PRIVATE precompiled.hpp)

add_test(NAME ${TARGET} COMMAND ${TARGET})
#install(TARGETS ${TARGET} RUNTIME)
