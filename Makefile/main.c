#include <stdio.h>
#include "compute.h"

int main(void){
    printf("4 + 2 = %d\n", add(4, 2));      // 该函数在compute.h中声明，add.c中定义
    return 0;
}