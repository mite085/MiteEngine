# 文件系统配置
configure_file(
    ${CMAKE_SOURCE_DIR}/config/filesystem_config.h.in
    ${CMAKE_BINARY_DIR}/src/engine/core/filesystem/filesystem_config.h
    @ONLY
)

# 资源安装规则
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
    USE_SOURCE_PERMISSIONS
)