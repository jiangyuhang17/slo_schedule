cmake_minimum_required(VERSION 3.0.0)
project(slo_schedule VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(slo_schedule main.cpp log.cpp Manager.cpp Connection.cpp)

TARGET_LINK_LIBRARIES(slo_schedule event)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)