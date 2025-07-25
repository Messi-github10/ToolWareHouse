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