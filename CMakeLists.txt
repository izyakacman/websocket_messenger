cmake_minimum_required (VERSION 3.8)

set(TARGET "websocket_messenger")

if(DEFINED ENV{GITHUB_RUN_NUMBER})
    project (${TARGET} VERSION 0.0.$ENV{GITHUB_RUN_NUMBER})
else ()
    project (${TARGET})
endif()

enable_testing()

add_subdirectory(src)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
