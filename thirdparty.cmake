# 查找 OpenGL 库（跨平台）
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found! Please install OpenGL development libraries.")
endif()

# 删除第三方库的测试构建
set(ASSIMP_BUILD_TESTS OFF)
set(GLFW_BUILD_TESTS OFF)
set(SPDLOG_BUILD_TESTS OFF)
set(SHADERC_SKIP_TESTS ON)
set(SPIRV_SKIP_TESTS ON)

# 启用文件夹功能（Visual Studio专用）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 添加第三方目录(使用现代 CMake 方式包含头文件目录)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/meshoptimizer)

# 条件添加 googletest
if(BUILD_TESTS)
    add_subdirectory(thirdparty/googletest)
endif()

# 创建接口库来管理第三方头文件
add_library(mite_engine_thirdparty_headers INTERFACE)

# 添加头文件目录到接口库
target_include_directories(mite_engine_thirdparty_headers INTERFACE
    thirdparty/glm
    thirdparty/glad
    thirdparty/stduuid
    thirdparty/stduuid/include
    thirdparty/cereal/include
    thirdparty/stbimg
    thirdparty/json/single_include
    thirdparty/threadpool/include
)

# imgui和imguizmo无cmakelist，为避免污染依赖库，此处手动添加
# imguifuledialog通过find_package寻找imgui，故手动实现，不依赖其自带的cmake文件
include(imgui.cmake)
include(imguizmo.cmake)
include(imguifiledialog.cmake)

# ============================================
# Shaderc 依赖自动同步
# ============================================
set(SHADERC_DEPS_SYNCED FALSE)
# 检查是否需要同步依赖
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/shaderc/third_party/glslang/CMakeLists.txt")
    message(STATUS "Shaderc dependencies already exist, skipping sync")
    set(SHADERC_DEPS_SYNCED TRUE)
else()
    message(STATUS "Shaderc dependencies not found, will sync automatically")
endif()
# 如果依赖不存在，自动同步
if(NOT SHADERC_DEPS_SYNCED)
    # 查找 Python3
    find_package(Python3 COMPONENTS Interpreter)
    
    if(Python3_Interpreter_FOUND)
        message(STATUS "Found Python3: ${Python3_EXECUTABLE}")
        
        # 获取 shaderc 目录
        set(SHADERC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/shaderc")
        
        # 检查 git-sync-deps 脚本是否存在
        if(EXISTS "${SHADERC_DIR}/utils/git-sync-deps")
            message(STATUS "Running git-sync-deps for Shaderc...")
            
            # 执行同步命令
            execute_process(
                COMMAND ${Python3_EXECUTABLE} utils/git-sync-deps
                WORKING_DIRECTORY ${SHADERC_DIR}
                RESULT_VARIABLE SYNC_RESULT
                OUTPUT_VARIABLE SYNC_OUTPUT
                ERROR_VARIABLE SYNC_ERROR
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
            )
            
            if(SYNC_RESULT EQUAL 0)
                message(STATUS "Shaderc dependencies synced successfully")
                message(STATUS "Output: ${SYNC_OUTPUT}")
                set(SHADERC_DEPS_SYNCED TRUE)
            else()
                message(WARNING "Failed to sync Shaderc dependencies")
                message(WARNING "Error: ${SYNC_ERROR}")
                message(WARNING "You may need to manually run: cd thirdparty/shaderc && python3 utils/git-sync-deps")
            endif()
        else()
            message(WARNING "git-sync-deps script not found at ${SHADERC_DIR}/utils/git-sync-deps")
            message(WARNING "Please ensure you have the full Shaderc repository")
        endif()
    else()
        message(WARNING "Python3 not found! Cannot auto-sync Shaderc dependencies")
        message(WARNING "Please install Python3 and ensure it's in PATH, or manually run:")
        message(WARNING "  cd thirdparty/shaderc && python3 utils/git-sync-deps")
    endif()
endif()
# 检查依赖是否就绪，然后添加 shaderc
if(SHADERC_DEPS_SYNCED OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/shaderc/third_party/glslang/CMakeLists.txt")
    message(STATUS "Adding Shaderc to build...")
    add_subdirectory(thirdparty/shaderc)
else()
    message(WARNING "Shaderc dependencies not available, skipping Shaderc build")
    message(WARNING "You can manually sync dependencies with:")
    message(WARNING "  cd thirdparty/shaderc && python3 utils/git-sync-deps")
    message(WARNING "Then reconfigure CMake")
endif()



# 简化的文件夹设置函数
function(set_thirdparty_folder target folder)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER "ThirdParty/${folder}")
    endif()
endfunction()

# ============================================
# 为基础第三方库设置文件夹属性
# ============================================
set_thirdparty_folder(spdlog "Logging")
set_thirdparty_folder(assimp "Assets")
set_thirdparty_folder(unit "Assets")
set_thirdparty_folder(UpdateAssimpLibsDebugSymbolsAndDLLs "Assets")
set_thirdparty_folder(zlibstatic "Assets")
set_thirdparty_folder(meshoptimizer "MeshOptimizer")
# ImGui 相关
set_thirdparty_folder(Imgui "UI")
set_thirdparty_folder(ImGuizmo "UI")
set_thirdparty_folder(ImGuiFileDialog "UI")
# Google Test
set_thirdparty_folder(gmock "Testing")
set_thirdparty_folder(gmock_main "Testing")
set_thirdparty_folder(gtest "Testing")
set_thirdparty_folder(gtest_main "Testing")
# ============================================
# Shaderc 相关目标
# ============================================
set_thirdparty_folder(add-copyright "Shaderc")
set_thirdparty_folder(build-version "Shaderc")
set_thirdparty_folder(check-copyright "Shaderc")
set_thirdparty_folder(core_tables "Shaderc")
set_thirdparty_folder(extinst_tables "Shaderc")
set_thirdparty_folder(glslc "Shaderc")
set_thirdparty_folder(glslc_exe "Shaderc")
set_thirdparty_folder(shaderc "Shaderc")
set_thirdparty_folder(shaderc_combined "Shaderc")
set_thirdparty_folder(shaderc_combined-pkg-config "Shaderc")
set_thirdparty_folder(shaderc_shared "Shaderc")
set_thirdparty_folder(shaderc_static-pkg-config "Shaderc")
set_thirdparty_folder(shaderc_util "Shaderc")
set_thirdparty_folder(shaderc-online-compile "Shaderc")
set_thirdparty_folder(shaderc-pkg-config "Shaderc")
set_thirdparty_folder(spirv-tools-check-python-tests "Shaderc")
set_thirdparty_folder(spirv-tools-check-python-types "Shaderc")
set_thirdparty_folder(spirv-tools-pkg-config "Shaderc")
set_thirdparty_folder(spirv-tools-tables "Shaderc")
set_thirdparty_folder(testdata "Shaderc")
# ============================================
# glslang 相关目标（Shaderc 子模块）
# ============================================
set_thirdparty_folder(GenericCodeGen "glslang")
set_thirdparty_folder(glslang "glslang")
set_thirdparty_folder(glslang-default-resource-limits "glslang")
set_thirdparty_folder(MachineIndependent "glslang")
set_thirdparty_folder(OSDependent "glslang")
set_thirdparty_folder(SPIRV "glslang")
set_thirdparty_folder(glslang-standalone "glslang")
set_thirdparty_folder(SPVRemapper "glslang")
# ============================================
# SPIRV-Tools 相关目标（Shaderc 子模块）
# ============================================
set_thirdparty_folder(spirv-tools-build-version "SPIRV-Tools")
set_thirdparty_folder(spirv-tools-header-DebugInfo "SPIRV-Tools")
set_thirdparty_folder(spirv-tools-header-NonSemanticShaderDebugInfo100 "SPIRV-Tools")
set_thirdparty_folder(spirv-tools-header-OpenCLDebugInfo100 "SPIRV-Tools")
set_thirdparty_folder(spirv-tools-cpp-example "SPIRV-Tools")
set_thirdparty_folder(spirv-as "SPIRV-Tools")
set_thirdparty_folder(spirv-cfg "SPIRV-Tools")
set_thirdparty_folder(spirv-diff "SPIRV-Tools")
set_thirdparty_folder(spirv-dis "SPIRV-Tools")
set_thirdparty_folder(spirv-link "SPIRV-Tools")
set_thirdparty_folder(spirv-lint "SPIRV-Tools")
set_thirdparty_folder(spirv-objdump "SPIRV-Tools")
set_thirdparty_folder(spirv-opt "SPIRV-Tools")
set_thirdparty_folder(spirv-reduce "SPIRV-Tools")
set_thirdparty_folder(spirv-val "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-diff "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-link "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-lint "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-opt "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-reduce "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-shared "SPIRV-Tools")
set_thirdparty_folder(SPIRV-Tools-static "SPIRV-Tools")
set_thirdparty_folder(spirv-tools-vimsyntax "SPIRV-Tools")