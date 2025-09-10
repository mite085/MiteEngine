# 文件系统配置 - 修改为使用相对路径
configure_file(
    ${CMAKE_SOURCE_DIR}/config/filesystem_config.h.in
    ${CMAKE_BINARY_DIR}/src/engine/core/filesystem/filesystem_config.h
    @ONLY
)