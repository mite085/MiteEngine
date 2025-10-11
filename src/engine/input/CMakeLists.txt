cmake_minimum_required(VERSION 3.10)

# 设定VCXProject名为MiteEngine_Input
set(ENGINE_INPUT_LIB_NAME MiteEngine_Engine_Input)

# 创建源代码组(筛选器)
set(INPUT_SRCS
    input/input.h
    input/input.cpp
    input/input_context.h
    input/input_context.cpp
    input/input_event.h
    input/input_event.cpp
    input/input_manager.h
    input/input_manager.cpp
    input/input_define.h
    input/input_define.cpp
    input/input_context_stack.h
    input/input_context_stack.cpp
    input/input_processor.h
    input/input_processor.cpp
    input/modular_input_context.h
    input/modular_input_context.cpp
)
source_group("input" FILES ${INPUT_SRCS})

set(INPUT_PROCESSOR_SRCS
    # input_processors/camera_input_processor.h
    # input_processors/camera_input_processor.cpp
)
source_group("input_processors" FILES ${INPUT_PROCESSOR_SRCS})

# 创建静态库
add_library(${ENGINE_INPUT_LIB_NAME}  
    STATIC  
    ${INPUT_SRCS}
    ${INPUT_PROCESSOR_SRCS}
)

# 设置头文件路径（供其他项目包含）
target_include_directories(${ENGINE_INPUT_LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  # 公开头文件目录
)

target_link_libraries(${ENGINE_INPUT_LIB_NAME}
    PRIVATE 
    glfw
    shaderc
    glslang
    spdlog::spdlog
    MiteEngine_Engine_Core
    MiteEngine_Engine_Data
    MiteEngine_Engine_Event
    MiteEngine_Engine_Material
    MiteEngine_Engine_Scene_Core
)