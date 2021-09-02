set(TARGET "messenger_server")
set(LIB_TARGET "server")



if (UNIX)
    find_package (Threads)
endif()

find_package(Boost COMPONENTS system  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(SOURCES
    main.cpp
    listener.cpp
    shared_state.cpp
    websocket_session.cpp
    commands_processor.cpp
)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (${TARGET} ${SOURCES})
add_library(${LIB_TARGET} STATIC ${SOURCES})

target_precompile_headers(${TARGET} PRIVATE precompiled.hpp)

target_link_libraries(${TARGET}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ) 

if (UNIX)
    target_link_libraries (${TARGET} Threads::Threads)
    target_compile_options(${TARGET} PUBLIC
        -Wall -Wextra -pedantic -Werror
    )
endif()

set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
