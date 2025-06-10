# Cup
Cup 是一个 C++ 项目管理的命令行工具，它基于 CMake 运行，参考了 Rust 的包管理器 cargo 的设计，让开发者无需再学习一门新的构建脚本语言即可轻松地管理和构建 C++ 项目；同时 Cup 通过固定的项目结构和配置文件，可以使得项目结构混乱没有统一标准成为 C++ 的历史

## 安装
Cup 只有一个可执行文件，你只需将它放置在任何你喜欢的路径下，并将其添加到环境变量中即可

**Cup 需要依赖 CMake(>=3.10) 运行，因而你还需确保你的设备环境中可以通过环境变量访问到 CMake，你可以通过在命令行运行 `cmake --version` 来检查你的环境中是否有 CMake 以及它的版本号**

## 使用
针对于 Cup 的使用，可以通过 [使用说明](https://github.com/Anglebase/Cup/blob/master/docs/use.md) 学习更多；通过查阅 [配置文件](https://github.com/Anglebase/Cup/blob/master/docs/cup.toml)，可以了解 Cup 的配置文件都支持哪些参数；通过查阅 [君子协定](https://github.com/Anglebase/Cup/blob/master/docs/agree.md)可以了解如何让你编写的库更易使用

对于嵌入式项目的配置文件，可以参考[STM32F103C8T6 Demo项目配置文件](https://github.com/Anglebase/Cup/blob/master/docs/stm32f103.toml)

对于Qt项目的配置文件，可以参考[Qt6 Demo项目配置文件](https://github.com/Anglebase/Cup/blob/master/docs/qt.toml)

## 示例

1. 创建一个名为`hello`的`binary`项目，并将当前路径切换至该项目的根目录
```
> cup new hello && cd hello

[Info] Create new project: hello
[Info] Generator template project...
[Info] Finised.
```
2. 运行默认项目模板(可能的输出)
```
hello> cup run hello.cpp

[Info] Generating cmake...
[Info] Building...
-- Building for: Visual Studio 17 2022
-- Selecting Windows SDK version 10.0.22000.0 to target Windows 10.0.26100.
-- The C compiler identification is MSVC 19.43.34810.0
-- The CXX compiler identification is MSVC 19.43.34810.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x64/cl.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (3.1s)
-- Generating done (0.0s)
-- Build files have been written to: E:/Project/cup/target/hello/build/cmake
适用于 .NET Framework MSBuild 版本 17.13.19+0d9f5a35a

  1>Checking Build System
  Building Custom Rule E:/Project/cup/target/hello/build/CMakeLists.txt
  hello.cpp
  hello_7ce33b1ff0b53bd3d762e4e0cdd093a9.vcxproj -> E:\Project\cup\target\hello\target\Debug\hello.exe
[Info] Run: E:\Project\cup\target\hello\target\Debug\hello.exe

Hello, world!

[Info] Exit Code: 0 (0x0)
```

## 许可证

GPL-v3.0