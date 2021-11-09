//
// Created by xy on 10/24/21.
//
#include <iostream>
#include<iostream>
#include<stdlib.h>
#include<unordered_map>
#include<iomanip>
#include<vector>
#include<assert.h>
#include "list.h"
#include "student.h"

student_s* student_sys::search_stu(unsigned int no_){//学号唯一
    student_s* cur= nullptr;
    for(auto i=classes.begin();i!=classes.end();i++){
        list_for_each_entry(cur,&((*i).second->node),node){
            if(cur->no==no_){
                return cur;
            }
        }
    }
    return nullptr;
}
std::vector<student_s*> student_sys::search_stu(const std::string& name_) {//姓名不唯一
    student_s* cur;
    std::vector<student_s*> result;
    for(auto i=classes.begin();i!=classes.end();i++){
        list_for_each_entry(cur,&((*i).second->node),node){
            if(cur->name==name_){
                result.push_back(cur);
            }
        }
    }
    return result;
}
int student_sys::modify_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no){
    student_s* s=search_stu(no_);
    if(!s){
        return -1;
    }
    if(s->no!=no_){//学号不可修改
        return -2;
    }

    if(s->class_no!=class_no) {//更换班级
        del_stu(no_);
        add_stu(no_,name_,age_,class_no);
    }
    else {
        s->name=name_;
        s->age=age_;
    }
    return true;
}
int student_sys::add_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no){
    if(search_stu(no_))//学号唯一
        return false;

    auto i=classes.find(class_no);
    if(i==classes.end()){
        auto head=new student_s();
        INIT_LIST_HEAD(&(head->node));
        classes.insert({class_no,head});
        i=classes.find(class_no);
    }
    //按学号从小到大放入对应班级的某位置
    student_s* cur, *head=(*i).second,*pre=head;
    auto s=new student_s(no_,name_,age_,class_no);
    list_for_each_entry(cur,&(head->node),node){
        if(cur->no>s->no){
            __list_add(&(s->node),&(pre->node),&(cur->node));
            break;
        }
        pre=cur;
    }
    if(cur==(*i).second) {//list_for_each_entry走尽，未找到比新学生的no还大的学生
        list_add_tail(&(s->node), &(pre->node));
    }

    return true;
}
int student_sys::del_stu(unsigned int no_){//
    student_s* s=search_stu(no_);
    if(!s){
        return false;
    };
    __list_del(s->node.prev,s->node.next);
    free(s);
    return true;
}
//分班级按学号从小到大打印
void student_sys::print_all_stu() {
    using std::cout;
    using std::endl;
    using std::setw;
    int w=10;
    student_s *cur;
    for (auto i = classes.begin(); i != classes.end(); i++) {
        list_for_each_entry(cur, &((*i).second->node), node) {
            cur->print_stu();
        }
        cout << endl;
    }
}