#include "keyboard.h"

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
    init_cq(&kbd_circular_buf_queue);
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
        if((shift_status==1 && capslock_status==1)){
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
            put_char(make_ascii_mapping[c][0]);
        }
        else if((shift_status==1&&capslock_status==0) || (shift_status==0&&capslock_status==1)){
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][1]);
            put_char(make_ascii_mapping[c][1]);
        }
        else{
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
            put_char(make_ascii_mapping[c][0]);
        }
    }
    else if(c==0x29 || c==0x1a || c==0x1b || c==0x2b || c==0x27 || c==0x28 || c==0x33 || c==0x34 || c==0x35 || (c>=0x02&&c<=0x0d)){
        if(shift_status==1){
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][1]);
            put_char(make_ascii_mapping[c][1]);
        }
        else {
            cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
            put_char(make_ascii_mapping[c][0]);
        }
    }
    else if(c==0x1c || c==0x0e || c==0x39){
        cq_put_one_elem(&kbd_circular_buf_queue,make_ascii_mapping[c][0]);
        put_char(make_ascii_mapping[c][0]);
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


/*
struct circular_queue{ 
    char buf[CIRCULAR_QUEUE_MAXSIZE];
    int head;
    int tail;
};
*/
//we simply turn off the intr to guarantee multi-thread safe
void init_cq(struct circular_queue* q){
    enum INTR_STATUS old_status=disable_intr();
    q->head=0;
    q->tail=0;
    set_intr_status(old_status);
}

char cq_take_one_elem(struct circular_queue* q){
    enum INTR_STATUS old_status=disable_intr();
    if(cq_is_empty(q)){
        return -1;
    }
    else{
        int old=q->head;
        q->head=(q->head+1)%CIRCULAR_QUEUE_MAXSIZE;
        ASSERT((q->buf)[old]!=-1);
        return (q->buf)[old];
    }
    set_intr_status(old_status);
}

void cq_put_one_elem(struct circular_queue* q,char c){
    enum INTR_STATUS old_status=disable_intr();
    if(cq_is_full(q)){
        return;
    }
    else{
        q->buf[q->tail]=c;
        q->tail=(q->tail+1)%CIRCULAR_QUEUE_MAXSIZE;
    }
    set_intr_status(old_status);
}

int cq_is_empty(const struct circular_queue* q){
    enum INTR_STATUS old_status=disable_intr();
    return (q->head)==(q->tail);
    set_intr_status(old_status);
}

int cq_is_full(const struct circular_queue* q){
    enum INTR_STATUS old_status=disable_intr();
    return ((q->tail+1)%CIRCULAR_QUEUE_MAXSIZE)==(q->head);
    set_intr_status(old_status);
}

