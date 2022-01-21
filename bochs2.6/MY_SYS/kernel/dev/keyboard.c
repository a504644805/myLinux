#include "keyboard.h"
#include "list.h"
#include "stdio.h"
#include "io.h"
#include "print.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "thread.h"
#include "circular_queue.h"
//[][2]
//make_code  ascii  ascii when shift
char make_ascii_mapping[][2]={
    {0,0},
    {KBD_ESC,KBD_ESC},
    {'1','!'},
    {'2','@'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','^'},
    {'7','&'},
    {'8','*'},
    {'9','('},
    {'0',')'},
    {'-','_'},
    {'=','+'},
    {'\b','\b'},
    {KBD_INVI,KBD_INVI},//put_char do not support tab currently
    {'q','Q'},
    {'w','W'},
    {'e','E'},
    {'r','R'},
    {'t','T'},
    {'y','Y'},
    {'u','U'},
    {'i','I'},
    {'o','O'},
    {'p','P'},
    {'[','{'},
    {']','}'},
    {'\n','\n'},
    {KBD_INVI,KBD_INVI},
    {'a','A'},
    {'s','S'},
    {'d','D'},
    {'f','F'},
    {'g','G'},
    {'h','H'},
    {'j','J'},
    {'k','K'},
    {'l','L'},
    {';',':'},
    {'\'','\"'},
    {'`','~'},
    {KBD_INVI,KBD_INVI},
    {'\\','|'},
    {'z','Z'},
    {'x','X'},
    {'c','C'},
    {'v','V'},
    {'b','B'},
    {'n','N'},
    {'m','M'},
    {',','<'},
    {'.','>'},
    {'/','?'},
    {KBD_INVI,KBD_INVI},
    {KBD_INVI,KBD_INVI},
    {KBD_INVI,KBD_INVI},
    {' ',' '}
};

//producer is kbd_intr_handler, consumer is shell who read the commond
struct circular_queue kbd_circular_buf_queue;
void init_kbd(){
    init_cq(&kbd_circular_buf_queue,CQ_DO_NOT_USE_EMPTY);
}
int ctrl_status=0,alt_status=0,capslock_status=0,shift_status=0;

void kbd_intr_handler(){
    uint8_t c=inb(KBD_READ_REG);//c is abbrevation of code
    if(c==KBD_EXTEND){
        
    }
    else if(c==KBD_CTRL){
        ctrl_status=1;
    }
    else if(c==KBD_ALT){
        alt_status=1;
    }
    else if(c==KBD_SHIFT){
        shift_status=1;
    }
    else if(c==KBD_CAPSLOCK){
        //put_int(c);
        capslock_status=!capslock_status;
    }
    else if((c>=0x10&&c<=0x19)||(c>=0x1e&&c<=0x26)||(c>=0x2c&&c<=0x32)){//group1
        char ch=make_ascii_mapping[c][0];
        if((ctrl_status==1 && ch=='u') || (ctrl_status==1 && ch=='l')){//快捷键
            cq_put_one_elem(&kbd_circular_buf_queue,ch -'a');
        }
        else{
            if((shift_status==1 && capslock_status==1)){
                cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
            }
            else if((shift_status==1&&capslock_status==0) || (shift_status==0&&capslock_status==1)){
                cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][1]);
            }
            else{
                cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
            }
        }
    }
    else if(c==0x29 || c==0x1a || c==0x1b || c==0x2b || c==0x27 || c==0x28 || c==0x33 || c==0x34 || c==0x35 || (c>=0x02&&c<=0x0d)){
        if(shift_status==1){
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][1]);
        }
        else {
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
        }
    }
    else if(c==0x1c || c==0x0e || c==0x39){
        cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
    }
    else if((c&0x80)){//break code
        c-=0x80;
        if(c==KBD_CTRL){
            ctrl_status=0;
        }
        else if(c==KBD_ALT){
            alt_status=0;
        }
        else if(c==KBD_SHIFT){
            shift_status=0;
        }
        else if(c==KBD_CAPSLOCK){
            capslock_status=capslock_status;//don't change
            //put_int(c+0x80);
        }
        else{

        }
    }
    else{

    }
}

