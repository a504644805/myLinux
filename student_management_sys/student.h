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
extern int MAX_name_Length;
extern int MAX_class_no_length;
extern int MAX_sno_length;
extern int MAX_age_length;
#ifndef STUDENT_MANAGEMENT_SYS_STUDENT_H
#define STUDENT_MANAGEMENT_SYS_STUDENT_H
using std::setw;
using std::cin;
using std::cout;
using std::endl;

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
    void print_stu() {
        cout << "Name:" << setw(MAX_name_Length) << name << "  ";
        cout << "No:" << setw(MAX_sno_length) << no << "  ";
        cout << "Age:" << setw(MAX_age_length) << age << "  ";
        cout << "Class:" << setw(MAX_class_no_length) << class_no << endl;
    }
}student_s;

struct student_sys{
    std::unordered_map<unsigned int,student_s*> classes;

    student_sys() = default;
    student_s* search_stu(unsigned int no_);
    std::vector<student_s*> search_stu(const std::string& name_);
    int modify_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no);
    int add_stu(unsigned int no_, const std::string& name_, unsigned int age_, unsigned int class_no);
    int del_stu(unsigned int no_);
    //分班级按学号从小到大打印
    void print_all_stu() ;
};
#endif //STUDENT_MANAGEMENT_SYS_STUDENT_H
