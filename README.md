# slr1_parser_cpp

## 项目简介

这是一个用C++实现的简单编译器项目，包含以下模块：
- **词法分析器（Lexer）**：将源代码分解为词法单元（Token）。
- **语法分析器（Parser）**：基于SLR(1)算法进行语法分析，生成语法树。
- **SLR分析表生成器（SLRParser）**：构造SLR(1)分析表。

## 项目结构

```
compiler_cpp/
├── CMakeLists.txt       # CMake构建脚本
├── include/             # 头文件目录
│   └── grammer.h        # 文法相关的结构定义
├── src/                 # 源代码目录
│   ├── main.cpp         # 主程序入口
│   ├── lexer/           # 词法分析器模块
│   │   └── lexer.cpp    # 词法分析器实现
│   ├── parser/          # 语法分析器模块
│   │   └── parser.cpp   # 语法分析器实现
│   └── slr/             # SLR分析表生成器模块
│       └── slr.cpp      # SLR分析表生成器实现
└── README.md            # 项目说明文件
```

## 构建与运行

### 1. 安装依赖

确保已安装以下工具：
- CMake（版本 >= 3.10）
- C++编译器（支持C++17标准）

### 2. 构建项目

在项目根目录下运行以下命令：

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release --clean-first
```

### 3. 运行程序

构建完成后，运行生成的可执行文件：

```bash
cd Release
./compiler
```

### 4. 示例输入

程序会解析硬编码的C++代码片段（位于`main.cpp`中），并输出语法树。

### 5. 输出示例

```
Parsing completed successfully!

Syntax Tree:
Program
  Statements
    Statement
      DeclStmt
        Type
          int (int)
        x (x)
        ; (;)
    Statements
      Statement
        AssignStmt
          x (x)
          = (=)
          10 (10)
          ; (;)
      Statements
        Statement
          DeclStmt
            Type
              float (float)
            y (y)
            ; (;)
        Statements
          Statement
            AssignStmt
              y (y)
              = (=)
              3.14 (3.14)
              ; (;)
          Statements
            StmtList
              Statement
                IfStmt
                  if (if)
                  ( (()
                  Expr
                    x (x)
                    OPERATOR
                      > (>)
                    5 (5)
                  ) ())
                  { ({)
                  Statements
                    StmtList
                      Statement
                        Compute
                          y (y)
                          = (=)
                          Expr
                            y (y)
                            OPERATOR
                              + (+)
                            1.0 (1.0)
                          ; (;)
                  } (})
                  ElsePart
                    else (else)
                    { ({)
                    Statements
                      StmtList
                        Statement
                          WhileStmt
                            while (while)
                            ( (()
                            Expr
                              y (y)
                              OPERATOR
                                < (<)
                              10.0 (10.0)
                            ) ())
                            { ({)
                            Statements
                              StmtList
                                Statement
                                  Compute
                                    y (y)
                                    = (=)
                                    Expr
                                      y (y)
                                      OPERATOR
                                        * (*)
                                      2.0 (2.0)
                                    ; (;)
                            } (})
                    } (})
```


### 6. 其他注意事项

- 如果需要解析其他代码，请修改`main.cpp`中的`code`变量内容。
- 如果运行时出现编码问题，请确保终端支持UTF-8编码。