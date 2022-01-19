#ifndef SHELL_H
#define SHELL_H
#define MAX_CMD_LEN 32
#define MAX_ARG_NUM 10 //命令也是arg, 包括命令在内最多10个单词
void shell();
void print_prompt();
/*
divider: 如空格，则遇到一个空格就认为是一个arg
cmd: ls -l -h0  => ls0-l0-h0 
                   |  |  |
              argv[0][1][2] argv[3]=0
 */
int parse_cmd(char* cmd,char* argv[],char divider);

#define u_assert(condition) \
if(condition){} else{print_panic(__FILE__,__LINE__,#condition);}

void print_panic(const char* filename,int line,const char* msg);

#endif