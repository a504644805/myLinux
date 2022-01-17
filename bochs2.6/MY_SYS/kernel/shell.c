#include "syscall.h"
#include "stdio.h"
#include "shell.h"
#include "fs.h"
void shell(){
    char cmd[MAX_CMD_LEN+1]={0};
    int cmd_offset=0;//offset的两层含义
    char c;
    print_prompt();
    while(1){
        read(STD_IN,&c,1);
        switch (c)
        {
        case 'u'-'a':
            //清理掉当前的cmd_buf
            for (size_t i = 0; i < cmd_offset; i++){
                putchar('\b');
            }
            cmd_offset=0;            
            break;
        case 'l'-'a':
            //清屏但保留cmd_buf
            clear();
            print_prompt();
            cmd[cmd_offset]=0;
            printf("%s",cmd);
            break;
        case '\b':
            if(cmd_offset==0){
                //nop
            }
            else{
                ASSERT(cmd_offset>0);
                cmd_offset--;
                putchar(c);
            }
            break;
        case '\n':
            cmd[cmd_offset]=0;
            putchar(c);
            if(strcmp(cmd,"")==0){
            }
            else if(strcmp(cmd,"ls")==0){
                print_dir("/");
            }
            else if(strcmp(cmd,"clear")==0){
                clear();
            }
            else{
                printf("%s: command not found\n",cmd);
            }
            print_prompt();
            cmd_offset=0;
            break;
        default:
            if(cmd_offset==MAX_CMD_LEN){
                //nop
            }
            else{
                ASSERT(cmd_offset>=0 && cmd_offset<MAX_CMD_LEN);
                cmd[cmd_offset++]=c;
            }
            putchar(c);
            break;
        }
    }
}

void print_prompt(){
    printf("xy@localhost:/$ ");
}