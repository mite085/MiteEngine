# 设定VCXProject名为MiteEngine_Core
set(ENGINE_CORE_TEST_NAME MiteEngine_Engine_Core_Test)

# 创建源代码组(筛选器)
set(CORE_TEST_SRCS
    event_test.cpp
    filesystem_test.cpp
    logger_test.cpp
    time_test.cpp
    timer_test.cpp
    uuid_test.cpp
)
source_group("core tests" FILES ${CORE_TEST_SRCS})

add_executable(${ENGINE_CORE_TEST_NAME} ${CORE_TEST_SRCS})

target_link_libraries(${ENGINE_CORE_TEST_NAME}
    PRIVATE
    gtest_main
    gmock
    MiteEngine_Engine_Core
)
target_include_directories(${ENGINE_CORE_TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
add_test(NAME ${ENGINE_CORE_TEST_NAME} COMMAND ${ENGINE_CORE_TEST_NAME})