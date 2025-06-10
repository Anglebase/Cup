# 使用 Cup

## 基本概念
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

构建时 Cup 允许以`-r`或`-release`命令参数选项指定项目是否以 Release 模式编译，处于Debug模式下，Cup会隐式地添加宏`DEBUG`和`_DEBUG`；Release模式下，Cup会隐式地添加宏`NDEBUG`和`_NDEBUG`,用于条件编译。Debug模式下，将会令程序保留调试信息，以便于调试；Release模式下，将启用编译器的优化。

## 命令行参数

Cup 的基本命令格式为 `cup <command> [args...]`

+ `cup new <project name> [options...]`
创建一个新的 C++ 项目，`project name`指定项目的名称
`options` 可以包含以下选项：
    + `--target <target>` 指定项目的类型，默认为`binary`
    + `--dir <path>` 指定项目的存放路径，默认为当前目录

+ `cup build [build target] [options...]`
构建项目，`build target`指定构建的目标，若未指定，则构建所有的目标，该项应指定包含主函数的源文件相对于`bin`目录的路径，`options`可以包含以下选项：
    + `-r` `-release` 指定项目以 Relase 模式构建
    + `--dir <path>` 指定项目所在的路径
    + `--build <path>` 指定构建目录的生成路径

+ `cup clean [options...]`
清理项目的构建文件，`options`可以包含以下选项：
    + `--dir <path>` 指定项目所在的路径
    + `--build <path>` 指定构建目录的生成路径

+ `cup run <build target> [options...]`
运行项目的可执行文件，`build target`指定要构建目标的名称，该项应指定包含主函数的源文件相对于`bin`目录的路径，`options`可以包含以下选项：
    + `-r` `-release` 指定项目以 Relase 模式构建
    + `--dir <path>` 指定项目所在的路径
    + `--build <path>` 指定构建目录的生成路径
    + `--args <args...>` 指定运行可执行文件时的参数

+ `cup list <option> [options...]`
列出指定Cup项目下的下列属性之一：
    + `include`: 项目所引用到的所有头文件目录(均以绝对路径列出)
    + `deps`: 项目所依赖的其它项目列表
`options`可以包含以下选项：
    + `--dir <path>` 指定项目所在的路径

+ `cup help`
显示Cup的帮助信息