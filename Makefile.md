# Make Makefile great great again

## 基本规则概述

Makefile脚本指令的基本组成单元式规则，一个规则又分为几个小部分：

```makefile
target: prerequisites
    #command行的开头必须是一个制表符，若换成空格符，Makefile脚本会运行报错
    commands
```

一个**规则**由：**目标（Target）、依赖（Prerequisites）、命令（Commands）**三部分组成。

1. **目标（Target）**：目标通常是文件名，代表了要生成或更新的文件。比如：
   1. 可以是可执行程序的名字
   2. 可以是目标文件的名字
   3. 或者是其它任何想要借助 Makefile 创建和维护的文件。
   4. **注意：一条规则中，只有一个目标，不能有多个目标。**

2. **依赖（Prerequisites）**：代表生成目标时所依赖的文件，可以是一个，也可以是很多个。

3. **命令（Commands）**：代表生成目标时所执行的指令，可以是一条，也可以是多条。
   1. 注意：命令可以是任何shell命令，不局限于编译链接的gcc指令。
   2. **注意：每个命令必须以一个制表符（Tab）开始，这是 Makefile 语法的强制要求！**

## 简单的实例

```c
// compute.h
#ifndef __WD_COMPUTE_H
#define __WD_COMPUTE_H

int add(int a, int b); 
#endif

// add.c
#include <stdio.h>
#include "compute.h"             

int add(int a, int b){
    return a + b;
}

// main.c
#include <stdio.h>
#include "compute.h"

int main(void){
    printf("4 + 2 = %d\n", add(4, 2));      // 该函数在compute.h中声明，add.c中定义
    return 0;
}
```

新建一个Makefile文件

```makefile
main: add.o main.o
	gcc add.o main.o -o main

add.o: add.c compute.h
	gcc -c add.c -o add.o -Wall -g

main.o: main.c compute.h
	gcc -c main.c -o main.o -Wall -g
	
```

当你运行 `make` 命令时，它会自动读取当前目录下的 Makefile/makefile 文件。

## Makefile具体使用方法

### 伪目标

在 Makefile 中，**伪目标（Phony Target）** 是一种特殊目标，它**不代表实际的文件名**，而是用于执行特定操作（如清理、安装等）的命名指令。

> 如何声明伪目标

使用 `.PHONY` 显式声明：

```makefile
.PHONY: clean install uninstall

clean:
    rm -f *.o

install:
    cp program /usr/local/bin

uninstall:
    rm -f /usr/local/bin/program
```

> 具体情景

伪目标在实际工作中非常有用，常用来实现**clean**和**rebuild**两个功能，脚本代码如下：

```makefile
main: main.o add.o
    gcc main.o add.o -o main
main.o: main.c compute.h
    gcc -c main.c -Wall -g
add.o: add.c compute.h
    gcc -c add.c -Wall -g
clean:
    rm -f main main.o add.o     #删除所有生成的目标文件以及可执行程序，并且不给提示。-f是必须的
rebuild: clean main             #clean和main要写在依赖的位置，而不是命令的位置！
```

这里定义了两个伪目标，clean和rebuild，我们可以利用指令`make clean`以及`make rebuild`来生成这两个目标。最终的效果是：

1. **make clean**表示删除所有生成的目标文件以及可执行程序，这类似于VS当中的"清理解决方案/项目"的功能。
2. **make rebuild**表示要构建目标rebuild，于是去寻找依赖clean，发现clean不存在，于是先生成目标clean。然后再寻找依赖main，此时main可执行程序已经被清理了，于是就重新生成一次main可执行程序。这个过程实际上是：
   1. 先执行"清理解决方案/项目"的功能。
   2. 然后再将main可执行程序作为目标，重新编译链接生成一个新的main可执行程序。
   3. **这类似于VS当中的"重新生成解决方案/项目"的功能。**

建议在定义一个Makefile时，添加上这两个伪目标，以实现清理和重新生成的功能。

除此之外，实际工作中，我们还**建议用".PHONY"标记所有的伪目标**，脚本代码如下：

```makefile
#...其它脚本代码
.PHONY: clean rebuild           #PHONY有假的意思, 用它标记伪目标是一个好的编程实践，注意.PHONY后面有一个冒号
clean:
    rm -f main main.o add.o     #删除所有生成的目标文件以及可执行程序，并且不给提示。-f是必须的
rebuild: clean main             #clean和main要写在依赖的未知，而不是命令的位置！
```

好处：

1. 增强代码可读取，立刻就知道某个目标是一个伪目标。
2. **即便当前目录下存在名为clean或rebuild的文件，"make clean"指令也能正常执行，而不会因文件clean/rebuild作为目标存在而不执行对应指令！**

### 变量

> 自定义变量

在上面我们已经实现过的脚本中：

```makefile
main: main.o add.o
    gcc main.o add.o -o main
main.o: main.c compute.h
    gcc -c main.c -o main.o -Wall -g
add.o: add.c compute.h
    gcc -c add.c -o add.o -Wall -g

clean:
    rm -f main main.o add.o
rebuild: clean main
```

利用自定义变量，修改成：

```makefile
OUT := main     #目标文件
OBJS := main.o add.o       #生成目标文件所需要的依赖   
COM_OP := -Wall -g        #编译选项

$(OUT):$(OBJS)
    gcc main.o add.o -o main
main.o: main.c compute.h
    gcc -c main.c -o main.o $(COM_OP)
add.o: add.c compute.h
    gcc -c add.c -o add.o $(COM_OP)

clean:
    rm -f $(OUT) $(OBJS) 
rebuild: clean $(OUT)
```

> 自动变量

下面是一些常用的自动变量，**变量名**及其描述如下：

1. `@`：表示此规则的目标文件名。
2. `<`：表示此规则的第一个依赖文件名。
3. `^`：表示此所有的依赖文件列表，这些依赖项之间以空格分隔。

**注意：这些都只是变量名，要使用这些变量的话，需要在变量名前加字符$**

修改Makefile脚本代码：

```makefile
OUT := main     #目标文件
OBJS := main.o add.o       #生成目标文件所需要的依赖
COM_OP := -Wall -g        #编译选项

$(OUT):$(OBJS)
    gcc $^ -o $@
main.o: main.c compute.h
    gcc -c $< -o $@ $(COM_OP)
add.o: add.c compute.h
    gcc -c $< -o $@ $(COM_OP)                            

.PHONY: clean rebuild
clean:
    rm -f $(OUT) $(OBJS)         
rebuild: clean $(OUT)
```

>  预定义变量

预定义变量，即由Makefile自身预先定义好的变量，我们可以直接拿来，也可以先重新赋值再用。

常用的预定义变量如下：

|  变量名  |      功能      | 默认含义 |
| :------: | :------------: | :------: |
|    AR    | 打包静态库文件 |    ar    |
|    AS    |    汇编程序    |    as    |
|    CC    |    C编译器     |    cc    |
|   CPP    |   C预编译器    | $(CC) -E |
|   CXX    |   C++编译器    |   g++    |
|    RM    |      删除      |  rm –f   |
| ARFLAGS  |     库选项     |    无    |
| ASFLAGS  |    汇编选项    |    无    |
|  CFLAGS  |  C编译器选项   |    无    |
| CPPFLAGS | C预编译器选项  |    无    |
| CXXFLAGS | C++编译器选项  |    无    |

于是引入预定义变量后，我们可以将上面的 Makefile脚本改写成：

```makefile
OUT := main     #目标文件
OBJS := main.o add.o       #生成目标文件所需要的依赖
COM_OP := -Wall -g        #编译选项
CC := gcc        #修改CC的默认值                            

$(OUT):$(OBJS)
    $(CC) $^ -o $@
main.o: main.c compute.h
    $(CC) -c $< -o $@ $(COM_OP)
add.o: add.c compute.h
    $(CC) -c $< -o $@ $(COM_OP)   

.PHONY: clean rebuild
clean:
    $(RM) $(OUT) $(OBJS)         
rebuild: clean $(OUT)
```

此时，我们新增一个文件`sub.c`，包含头文件，并在main.c中调用其中的函数，于是我们就可以快速修改这个Makefile为：

```makefile
OUT := main     #目标文件
OBJS := main.o add.o sub.o      #生成目标文件所需要的依赖
COM_OP := -Wall -g        #编译选项
CC := gcc        #修改CC的默认值

$(OUT):$(OBJS)
    $(CC) $^ -o $@
main.o: main.c compute.h
    $(CC) -c $< -o $@ $(COM_OP)
add.o: add.c compute.h
    $(CC) -c $< -o $@ $(COM_OP) 
sub.o: sub.c compute.h	# 新增一条！
    $(CC) -c $< -o $@ $(COM_OP)

.PHONY: clean rebuild
clean:
    $(RM) $(OUT) $(OBJS)
rebuild: clean $(OUT)
```

### 模式规则

Makefile 的 **模式规则（Pattern Rules）** 是一种高级规则，用于基于文件名模式（通配符）定义通用编译规则，从而避免为每个文件重复编写相似规则。它是自动化构建的核心机制之一。

语法结构如下：

```makefile
%.target : %.source
    commands
```

1. `%.target`表示可以匹配任意目标文件。比如`%.o`表示任意.o文件作为目标
2. `%.source`表示可以匹配任意依赖文件。比如`%.c`表示任意.c文件作为依赖

运用模式规则，修改Makefile文件：

```makefile
OUT := main     #目标文件
OBJS := main.o add.o sub.o      #生成目标文件所需要的依赖
COM_OP := -Wall -g        #编译选项
CC := gcc        #修改CC的默认值

$(OUT):$(OBJS)
    $(CC) $^ -o $@

%.o : %.c compute.h
    $(CC) -c $< -o $@ $(COM_OP)
#以下内容可以被上面一个模式规则替代
#main.o: main.c compute.h
#    $(CC) -c $< -o $@ $(COM_OP)
#add.o: add.c compute.h
#    $(CC) -c $< -o $@ $(COM_OP) 
#sub.o: sub.c compute.h
#    $(CC) -c $< -o $@ $(COM_OP)                       

.PHONY: clean rebuild
clean:
    $(RM) $(OUT) $(OBJS)         
rebuild: clean $(OUT)
```

使用模式规则，可以极大的简化代码，而且还可以提高扩展性。

比如这时，我们再增加一个运算`mul`（乘法），创建一个`mul.c`文件包含头文件compute，并实现函数。此时我们该如何改这个Makefile呢？

很简单只需要修改一个`OBJS`自定义变量的值就可以了：

```makefile
OBJS := main.o add.o sub.o mul.o      #生成目标文件所需要的依赖
```

**值得注意的是：**模式规则不能作为Makefile的第一个规则**，因为Makefile会选择第一个明确的目标作为默认目标，而模式规则中的目标，不指定具体的目标文件名，而仅仅定义了从一种文件类型转换到另一种文件类型的通用规则。**模式规则中的目标不是一个明确的目标，Makefile脚本执行时若把模式规则写在最上面，脚本执行会跳过这个模式规则目标。

归根结底：**Makefile 中应该先写「明确的最终目标」，再写「生成中间文件的规则（包括编译和模式规则）」**。

```makefile
# 1. 声明默认目标（必须是明确目标）
.DEFAULT_GOAL := all
all: program

# 2. 最终目标的链接规则（明确目标）
program: main.o utils.o
    $(CXX) $^ -o $@

# 3. 中间文件的编译规则（可用模式规则）
%.o : %.cpp
    $(CXX) -c $< -o $@

# 4. 清理等伪目标
.PHONY: clean
clean:
    rm -f *.o program
```

### 内置函数

> 通配符函数

```makefile
$(wildcard <pattern>)   
```

函数的名字是：wildcard

**它的作用是：查找符合<pattern>的所有文件列表**

**函数的返回值是：返回所有符合<pattern>的文件名，文件名之间以空格分隔。**

具体情景：

```makefile
SRCS := $(wildcard *.c)
```

其效果是：查找当前目录下，所有以.c结尾的文件名，并将文件名以空格分隔，再赋值给变量SRCS。

比如当前目录下有.c文件：main.c、add.c、sub.c

那么这个函数调用的作用等价于：

```makefile
SRCS := main.c add.c sub.c
```

> 模式替换函数

```makefile
$(patsubst <pattern>,<replacement>,<text>)
```

**函数的名字是：patsubst，它是词组"pattern substitution"的简写，意为模式替换。**

**它的作用是：查找<text>中符合模式<pattern>的单词(单词以空白字符分隔)，将其替换为<replacement>。**

注意事项：

1. <pattern>可以包括通配符%，表示任意长度的字符串。
2. 如果<replacement>中也含有%，那么<replacement>中的%所代表的字符串和<pattern>中%所代表的字符串相同。
3. <text>作为替换的数据源，它是最后一个参数。

**函数的返回值是：返回替换后的字符串**

具体情景：

```
OBJS := $(patsubst %.c, %.o, foo.c bar.c)
```

其效果是：将字符串 foo.c、bar.c 中符合模式 %.c 的单词替换成 %.o，返回结果为 foo.o 和 bar.o。

**注意：该函数的参数部分是以空格为间隔的，只需要一个空格作为间隔符，不要添加过多的空格！**

最后，修改Makefile脚本文件：

```makefile
OUT := main
SRCS := $(wildcard *.c) #将当前目录下的所有.c文件的文件名以空格分割，然后赋值给SRCS变量
OBJS := $(patsubst %.c,%.o,$(SRCS)) #获取当前目录下所有.c文件对应的.o文件，以空格分割
COM_OP := -Wall -g 
CC := gcc        #修改CC的默认值

$(OUT):$(OBJS)
    $(CC) $^ -o $@
%.o : %.c compute.h
    $(CC) -c $< -o $@ $(COM_OP)

.PHONY: clean rebuild
clean:
    $(RM) $(OUT) $(OBJS)
rebuild: clean $(OUT)
```

如果我再添加一个`div`运算，创建.c文件，修改main函数，就不需要对该Makefile脚本。



## 多个可执行程序的通用Makefile

```makefile
# 获取当前目录下所有 .c 源文件
SRCS := $(wildcard *.c)

# 使用 patsubst 模式替换，将所有的 .c 文件名去掉扩展名
OUTS := $(patsubst %.c, %, $(SRCS))

# 定义编译器为 gcc
CC := gcc

# 定义编译选项
COM_OP := -Wall -g

# 声明伪目标
.PHONY: clean rebuild all

# 执行 make 或 make all 时会构建所有程序
all: $(OUTS)

# 将 .c 源文件生成无扩展名的可执行文件
# $@ = "hello"（目标名）
# $^ = "hello.c"（依赖文件）
% : %.c
	$(CC) $^ -o $@ $(COM_OP)

# 删除所有生成的可执行文件
clean:
	$(RM) $(OUTS)

# rebuild
rebuild: clean all
```

