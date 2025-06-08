# Cup
Cup 是一个 C++ 项目管理的命令行工具，它基于 CMake 运行，参考了 Rust 的包管理器 cargo 的设计，让开发者无需再学习一门新的构建脚本语言即可轻松地管理和构建 C++ 项目；同时 Cup 通过固定的项目结构和配置文件，可以使得项目结构混乱没有统一标准成为 C++ 的历史

## 安装
Cup 只有一个可执行文件，你只需将它放置在任何你喜欢的路径下，并将其添加到环境变量中即可

**Cup 需要依赖 CMake(>=3.10) 运行，因而你还需确保你的设备环境中可以通过环境变量访问到 CMake，你可以通过在命令行运行 `cmake --version` 来检查你的环境中是否有 CMake 以及它的版本号**

## 使用

### 基本概念
Cup 所定义的C++项目分为三类：
+ `binary`(可执行文件)
+ `static`(静态链接库)
+ `shared`(动态链接库)

普通的`binary`允许你生成多个可执行文件，而`static`和`shared`则只能生成一个库文件

Cup 的典型项目结构为：
```
├── build
├── bin
├── include
├── src
├── target
└── cup.toml
```
其中，`bin`、`include`和`src`是用于存放源代码的目录；`build`目录是用来存放构建过程中产生的中间文件的目录；`target`目录是用来存放生成的可执行程序的目录，项目所生成的链接库文件位于`build/lib`中；`cup.toml`是Cup的配置文件，它定义了项目的名称、版本、依赖等信息

针对于存放源代码的目录，它们可以有任意子目录，需要注意的是，`include`目录并不会将所有的子目录都添加进编译器的include路径中，而是只会添加`include`目录本身，因此编写代码的`#include`预处理指令时需要其更为完整的相对路径，而非文件名，得益于这一特性，在编写链接库项目时，建议在`include`路径下创建一个与项目同名的文件夹，作为项目的公开API接口；对于`src`和`bin`下的源文件Cup会递归地搜索并将其参与构建，`src`下的所有源文件都不应包含主函数，`bin`下的所有源文件，每一个都必须包含主函数

### 命令行参数

Cup 的基本命令格式为 `cup <command> [args...]`
+ `cup new <project name> [options...]`
创建一个新的 C++ 项目，`project name`指定项目的名称
`options` 可以包含一下选项：
    + `--target <target>` 指定项目的类型，默认为`binary`
    + `--dir <path>` 指定项目的存放路径，默认为当前目录

+ `cup build [build target] [options...]`
构建项目，`build target`指定构建的目标，若未指定，则构建所有的目标，`options`可以包含一下选项：
    + `-r` `-release` 指定项目以 Relase 模式构建
    + `--dir <path>` 指定项目所在的路径
    + `--build <path>` 指定构建目录的生成路径

+ `cup clean [project name]`
清理项目的构建文件，`project name`指定项目的名称，若未指定，则视为当前路径下的项目

+ `cup run <build target> [options...]`
运行项目的可执行文件，`build target`指定要构建目标的名称，`options`可以包含一下选项：
    + `--dir <path>` 指定项目所在的路径
    + `--build <path>` 指定构建目录的生成路径
    + `--args <args...>` 指定运行可执行文件时的参数

### 配置文件

受支持的配置文件项如下：

*带有`*`号标记的是会解析但尚无任何效果的配置项*
```toml
name = "xxx" # 项目名称
*version = "0.1.0" # 项目版本
*description = "xxx project" # 项目描述
*authors = ["xxx"] # 项目作者
*license = "MIT" # 项目许可证

[build]
target = "binary" # 项目类型，可选值：binary, static, shared
generator = "MinGW Makefiles" # 指定CMake生成器
stdc = "11" # 指定C标准
stdcxx = "11" # 指定C++标准
define = ["DEBUG"] # 定义全局宏
include = ["path/to/include"] # 指定外部头文件搜索路径
jobs = 0 # 指定构建时的并行任务数，默认为1，若为0则表示使用CPU的核心数

[link]
name = "path/to/lib" # 外部链接库
...

[dependencies]
name = { path = "path/to/dependency" } # 依赖项
...
```

## 许可证

