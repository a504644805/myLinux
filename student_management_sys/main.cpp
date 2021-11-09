#include <iostream>
#include<iostream>
#include<stdlib.h>
#include<unordered_map>
#include<vector>
#include<algorithm>
#include "list.h"
#include "student.h"
using namespace std;
int MAX_name_Length;
int MAX_class_no_length;
int MAX_sno_length;
int MAX_age_length;
/*
使用Linux中的双向链表的头文件和相关代码，构建学生管
理程序。
– 定义学生节点，包含学号、姓名、年龄和班级等信息
– 录入学生信息
– 可以根据学号和姓名查询学生信息
– 修改学生信息（学号不可修改）
– 删除一个学生节点
– 按照学号顺序打印班级名单
*/
int main() {
    MAX_name_Length=10;
    MAX_class_no_length=10;
    MAX_sno_length=10;
    MAX_age_length=3;
    cout<<unitbuf;
    cout<<left;
    student_sys ss;

    string cmd;
    cout<<"Welcome to student management sys v.1.0.0"<<endl;
    cout<<"input commond plz"<<endl;
    cout<<"add: add a new student"<<endl;
    cout<<"del: del a student with sno"<<endl;
    cout<<"find: find a stu with sno"<<endl;
    cout<<"findN: find all stu with the name"<<endl;
    cout<<"modify: modify student by giving sno"<<endl;
    cout<<"printA: print all student"<<endl;
    cout<<"exit: exit the sys"<<endl;

    cout<<"/***For beauty when print, the default sno,name,class_no's max length is 10***/"<<endl;
    unsigned int sno,age,class_no;
    string name;
    while(cmd!="exit"){
        cout<<"# ";
        cin>>cmd;
        if(cmd=="add"){
            cout<<"input sno name age class_no"<<endl;
            cin>>sno;
            cin>>name;
            cin>>age>>class_no;
            if(!cin.good()){
                cout<<"bad input, this commond is omitted, input new commond again"<<endl;
                cin.clear();
                cin.ignore();
                continue;
            }
            if(!ss.add_stu(sno,name,age,class_no)){
                cout<<"sno already exist, add fail"<<endl;
            }
        }
        else if(cmd=="del"){
            cout<<"input sno"<<endl;
            cin>>sno;
            if(!cin.good()){
                cout<<"bad input, this commond is omitted, input new commond again"<<endl;
                cin.setstate((_Ios_Iostate )0);
                cin.clear();
                string t;
                cin>>t;
                continue;
            }
            if(ss.del_stu(sno)==false){
                cout<<"no such student, del fail"<<endl;
            }
        }
        else if(cmd=="find"){
            cout<<"input sno"<<endl;
            cin>>sno;
            if(!cin.good()){
                cout<<"bad input, this commond is omitted, input new commond again"<<endl;
                cin.clear();
                string t;
                cin>>t;
                continue;
            }
            auto i=ss.search_stu(sno);
            if(i == nullptr){
                cout<<"no such student"<<endl;
            }
            else{
                i->print_stu();
            }
        }
        else if(cmd=="findN"){
            cout<<"input name"<<endl;
            cin>>name;
            if(!cin.good()){
                cout<<"bad input, this commond is omitted, input new commond again"<<endl;
                cin.setstate((_Ios_Iostate )0);
                cin.clear();
                string t;
                cin>>t;
                continue;
            }
            auto i=ss.search_stu(name);
            if(i.size() == 0){
                cout<<"no such student"<<endl;
            }
            else{
                for(int k=0;k<i.size();k++){
                    i[k]->print_stu();
                }
            }
        }
        else if(cmd=="modify"){
            cout<<"input sno name age class_no"<<endl;
            cin>>sno;
            cin>>name;
            cin>>age>>class_no;
            if(!cin.good()){
                cout<<"bad input, this commond is omitted, input new commond again"<<endl;
                cin.setstate((_Ios_Iostate )0);
                cin.clear();
                string t;
                cin>>t;
                continue;
            }
            if(ss.modify_stu(sno,name,age,class_no)==-2){
                cout<<"sno already exist, modify fail"<<endl;
            }
            else if(ss.modify_stu(sno,name,age,class_no)==-1){
                cout<<"no such student, modify fail"<<endl;
            }
            else{
            }
        }
        else if(cmd=="printA"){
            ss.print_all_stu();
        }
        else if(cmd=="exit"){
            cout<<"Thank you for using, Bye~"<<endl;
            return 0;
        }
        else{
            cout<<"unidentified commond, input again plz"<<endl;
        }
    }

    /*
    ss.add_stu(17051330, "xy", 16, 17052313);
    ss.add_stu(17051334, "szw", 16, 17052313);
    ss.add_stu(17051328, "yjz", 16, 17052313);
    ss.add_stu(17051239, "ws", 16, 17052319);

    ss.del_stu(17051334);

    auto v=ss.search_stu("ws");
    auto s=ss.search_stu(17051330);
    ss.modify_stu(17051330,"xy2",16,17052313);
    s=ss.search_stu(17051330);

    ss.print_all_stu();
    */
    return 0;
}
