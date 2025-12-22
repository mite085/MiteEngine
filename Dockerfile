# 1. Docker环境+容器创建说明
#
# 首先安装DockerDesktop，会自动将Docker + WSL2等环境配置好（无需额外操作）
# 然后在项目根目录展开命令行，输入：docker pull ubuntu:latest
# 然后命令行继续输入：docker build -t mite-ci-env -f Dockerfile .
# 最后输入命令：docker run -it --name mite-ci-container -v %cd%:/workspace mite-ci-env

# 2. Docker容器使用与项目构建说明
#
# 进入容器后（DockerDesktop中点击容器的exec），工作目录已经切换到/workspace
# 这里的/workspace已经和宿主机的项目根目录绑定好了（可以直接输入ls -la查询）
# 首先创建build目录：mkdir build && cd build
# 然后运行CMake构建项目：cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
# 最后运行make编译项目：ninja

# 使用和 GitHub Actions 相同的基础镜像
FROM ubuntu:latest

# 安装最小必要依赖
RUN apt-get update && apt-get install -y \
    # 基础编译工具
    cmake \
    build-essential \
    pkg-config \
    python3 \
    ninja-build \
    # OpenGL 核心依赖
    libgl1-mesa-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    # GLFW 必要依赖
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    # Wayland 支持（GLFW 需要）
    libwayland-dev \
    wayland-protocols \
    # Zlib（Assimp 需要）
    zlib1g-dev \
    # 清理缓存
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /workspace


