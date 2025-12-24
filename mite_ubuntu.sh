# === MiteEngine Ubuntu启动器 ===
# 自动检测 OpenGL 版本，低于 4.5 则启用软件渲染 (LLVMpipe)
# 自动在当前目录查找 MiteEngine 可执行文件
# 使用方法：
# 1. chmod +x ./mite_ubuntu.sh
# 2. sudo ./mite_ubuntu.sh ./MiteEngine

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== MiteEngine Ubuntu启动器 ===${NC}"

# 检测 glxinfo 是否安装
if ! command -v glxinfo &> /dev/null; then
    echo -e "${RED}错误: glxinfo 未安装${NC}"
    echo "正在安装 mesa-utils..."
    sudo apt update && sudo apt install -y mesa-utils
fi

# 获取 OpenGL 版本
get_opengl_version() {
    # 尝试获取核心版本，如果失败则获取兼容版本
    local version=$(glxinfo 2>/dev/null | grep -E "OpenGL core profile version|OpenGL version string" | head -1)
    
    if [[ -z "$version" ]]; then
        echo "0.0"
        return
    fi
    
    # 提取版本号 (如 4.6, 3.3, 2.1)
    echo "$version" | grep -oE '[0-9]+\.[0-9]+' | head -1
}

# 获取版本号
VERSION=$(get_opengl_version)
MAJOR=$(echo $VERSION | cut -d. -f1)
MINOR=$(echo $VERSION | cut -d. -f2)

echo -e "检测到 OpenGL 版本: ${GREEN}$VERSION${NC}"

# 检查是否满足要求
if [[ $MAJOR -ge 4 ]] && [[ $MINOR -ge 5 ]]; then
    echo -e "${GREEN}✓ 硬件支持 OpenGL 4.5+，使用硬件加速${NC}"
    # 清除软件渲染标志（如果之前设置过）
    unset LIBGL_ALWAYS_SOFTWARE
    unset GALLIUM_DRIVER
else
    echo -e "${YELLOW}⚠ OpenGL 版本低于 4.5, 启用软件渲染 (LLVMpipe)${NC}"
    
    # 检查 LLVMpipe 是否可用
    if ! glxinfo -i 2>/dev/null | grep -q "llvmpipe"; then
        echo -e "${YELLOW}正在安装软件渲染组件...${NC}"
        sudo apt update && sudo apt install -y mesa-utils llvmpipe
    fi
    
    # 设置软件渲染环境变量
    export LIBGL_ALWAYS_SOFTWARE=1
    export GALLIUM_DRIVER=llvmpipe
    
    echo -e "${GREEN}✓ 已启用 LLVMpipe 软件渲染${NC}"
    
    # 验证软件渲染版本
    SW_VERSION=$(LIBGL_ALWAYS_SOFTWARE=1 glxinfo 2>/dev/null | grep -E "OpenGL core profile version|OpenGL version string" | head -1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
    echo -e "软件渲染 OpenGL 版本: ${GREEN}$SW_VERSION${NC}"
fi

echo -e "\n${YELLOW}=== 启动应用程序 ===${NC}"

# 检查是否有传入应用程序参数
if [ $# -eq 0 ]; then
    echo -e "${RED}错误: 请指定要启动的应用程序${NC}"
    echo "用法: $0 <应用程序命令>"
    echo "示例: $0 sudo ./mite_ubuntu.sh ./MiteEngine"
    exit 1
fi

# 显示当前环境
echo -e "当前渲染模式: ${GREEN}$([ -n "$LIBGL_ALWAYS_SOFTWARE" ] && echo "软件渲染" || echo "硬件加速")${NC}"
echo -e "启动命令: ${GREEN}$@${NC}"

# 执行应用程序
exec "$@"

