set(TARGET "messenger_client")
set(LIB_TARGET "client")

if (UNIX)
    find_package (Threads)
endif()

find_package(Boost COMPONENTS system  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(SOURCES
    main.cpp
    client.cpp
)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (${TARGET} ${SOURCES})
add_library(${LIB_TARGET} STATIC ${SOURCES})

target_precompile_headers(${TARGET} PRIVATE precompiled.hpp)

target_link_libraries(${TARGET}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ) 

if (MSVC)
    target_compile_options(${TARGET} PUBLIC
        /bigobj
    )
    target_compile_options(${LIB_TARGET} PUBLIC
        /bigobj
    )
else ()
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
