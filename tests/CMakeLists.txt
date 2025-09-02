# 设置测试包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/fixtures
    ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googlemock/include
)

# 添加单元测试子目录
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(system)
