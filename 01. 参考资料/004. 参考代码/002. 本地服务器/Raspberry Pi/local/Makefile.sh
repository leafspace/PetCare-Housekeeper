#makefile
CC:=gcc                                                                     # 注释：定义一个变量，表示当前编辑器为gcc
exe:=VoicePrinter
obj:=SourceMain.o ComCommon.o
all:$(obj)
    $(CC) -o $(exe) $(obj)
    %.o:%.c                                                                 # 注释：模式通配，自动将.c文件编译成.o文件
    $(CC) -c $^ -o $@                                                       # 注释：通配符

.PHONY:clean                                                                # 注释：声明clean是伪目标
clean:
 rm -rf $(obj) $(exe)
