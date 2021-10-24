//
// Created by xy on 10/16/21.
//
#include <iostream>
#include<iostream>
#include<stdlib.h>
#include<unordered_map>
#include<iomanip>
#include<vector>
#include<assert.h>
#include "list.h"

#ifndef STUDENT_MANAGEMENT_SYS_STUDENT_H
#define STUDENT_MANAGEMENT_SYS_STUDENT_H
typedef struct student_s {
    unsigned int no;
    std::string name;
    unsigned int age;
    unsigned int class_no;

    struct list_head node;

    student_s()=default;
    student_s(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no_) :\
		no(no_), name(name_), age(age_), class_no(class_no_) {
        INIT_LIST_HEAD(&node);
    }
}student_s;

struct student_sys{
    std::unordered_map<unsigned int,student_s*> classes;

    student_sys() = default;
    student_s* search_stu(unsigned int no_);
    std::vector<student_s*> search_stu(const std::string& name_);
    void modify_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no);
    void add_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no);
    void del_stu(unsigned int no_);
    //分班级按学号从小到大打印
    void print_all_stu() ;
};
#endif //STUDENT_MANAGEMENT_SYS_STUDENT_H
