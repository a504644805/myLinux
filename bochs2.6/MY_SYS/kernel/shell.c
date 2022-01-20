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
            putchar(c);
            cmd[cmd_offset]=0;
            char* argv[MAX_ARG_NUM+1];
            parse_cmd(cmd,argv,' ');
            if(strcmp(argv[0],"")==0){
            }
            else if(strcmp(argv[0],"ls")==0){
                print_dir("/");
            }
            else if(strcmp(argv[0],"clear")==0){
                clear();
            }
            else{//外部命令,目前只支持绝对路径
                if(cmd[0]!='/'){
                    printf("%s: command not found\n",argv[0]);
                }
                else{
                    int fd=open(argv[0],O_RDONLY);
                    if(fd>=3){
                        close(fd);
                        int i=fork();
                        if(i==-1){
                            u_assert(1==2);
                        }
                        else if(i==0){
                            execv(argv[0],argv);
                            u_assert(1==2);//shouldn't be here, 已经一去不回头了
                        }
                        else{
                            int child_status;
                            while(1){
                                int child_pid=wait(&child_status);
                                if(child_pid==-1){
                                    continue;
                                }
                                else{
                                    printf("SHELL: my exit child process's pid: %d, exit_status: %d\n",child_pid,child_status);
                                    break;
                                }
                            }
                        }
                    }
                    else if(fd==-1){
                        printf("%s: exec file not found\n",argv[0]);
                    }
                    else{
                        u_assert(1==2);
                    }
                }
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

/*
divider: 如空格，则遇到一个空格就认为是一个arg
cmd: ls -l -h0  => ls0-l0-h0 
                   |  |  |
              argv[0][1][2] argv[3]=0
 */
int parse_cmd(char* cmd,char* argv[],char divider){
    if (strcmp(cmd,"")==0){
        argv[0] = cmd;
        return 0;
    }
    int count = 1;
    argv[0] = cmd;
    for (int i = 0; cmd[i]!=0 ; i++) {
        if (cmd[i] == divider) {
            cmd[i] = 0;
            argv[count] = cmd+i+1;
            count++;
            ASSERT(count<=MAX_ARG_NUM);
        }
        else {
        }
    }
    argv[count] = 0;
    return count;
}


void print_panic(const char* filename,int line,const char* msg){
    //disable_intr();
    printf("ASSERT fail in: %s at line %d\n",filename,line);
    put_str("ASSERT fail in:");put_str(filename);put_str(" at line ");put_int(line);put_str("\n");
    put_str(msg);put_str("\n");
    while(1);
}