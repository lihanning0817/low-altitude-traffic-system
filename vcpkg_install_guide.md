# 使用vcpkg安装C++依赖库指南

## 1. 安装vcpkg

1. 打开命令提示符（CMD）或PowerShell
2. 克隆vcpkg仓库：
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   ```
   
   如果您没有安装git，可以从 https://git-scm.com/downloads 下载并安装。

3. 进入vcpkg目录：
   ```bash
   cd vcpkg
   ```

4. 运行bootstrap脚本：
   ```bash
   .\bootstrap-vcpkg.bat
   ```

## 2. 安装所需的依赖库

在vcpkg目录中运行以下命令来安装所需的依赖库：

```bash
.\vcpkg install boost websocketpp openssl unofficial-libmysql
```

这个命令将安装：
- Boost库：用于网络通信和系统功能
- websocketpp：用于WebSocket服务器实现
- OpenSSL：用于安全通信
- unofficial-libmysql：用于MySQL数据库连接（Windows平台）

## 3. 集成vcpkg与Visual Studio

为了让Visual Studio能够自动找到这些库，需要集成vcpkg：

```bash
.\vcpkg integrate install
```

## 4. 验证安装

您可以通过以下命令验证安装的库：

```bash
.\vcpkg list
```

这将显示所有已安装的库及其版本。

## 5. 故障排除

如果在安装过程中遇到问题，请尝试以下解决方案：

1. **权限问题**：
   如果遇到权限错误，请以管理员身份运行命令提示符。

2. **网络问题**：
   如果下载失败，可能是网络问题。可以尝试使用代理或稍后重试。

3. **编译问题**：
   如果编译失败，确保您已安装Visual Studio 2022及其C++开发工具。

4. **清理重新安装**：
   如果需要清理并重新安装，可以删除vcpkg目录并重新开始。

## 6. 使用已安装的库

安装完成后，您在构建C++项目时，CMake将能够自动找到这些库。您不需要额外配置路径。

如果您在构建过程中遇到任何问题，请确保：
1. vcpkg已正确集成到Visual Studio
2. 您在正确的目录中运行构建命令
3. 所有依赖库都已成功安装

按照这些步骤，您应该能够成功安装所有必需的C++依赖库，并准备好构建智能低空交通管理系统的后端。
