# 文件系统配置 - 修改为使用相对路径
configure_file(
    ${CMAKE_SOURCE_DIR}/config/filesystem_config.h.in
    ${CMAKE_BINARY_DIR}/src/engine/core/filesystem/filesystem_config.h
    @ONLY
)

# 翻译系统配置
set(LOCALIZATION_DIR assets/localization)
set(LOCALIZATION_FILES
    ${LOCALIZATION_DIR}/en-US.json
    ${LOCALIZATION_DIR}/zh-CN.json
    ${LOCALIZATION_DIR}/localization.schema.json
)

# 生成配置文件路径头文件 - 修改为使用相对路径
configure_file(
    ${CMAKE_SOURCE_DIR}/config/localization_config.h.in
    ${CMAKE_BINARY_DIR}/src/engine/ui/ui_core/localization_config.h
    @ONLY
)

# 添加自定义目标来复制assets文件夹（用于开发时方便）
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)
