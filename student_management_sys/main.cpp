#include <iostream>
#include<iostream>
#include<stdlib.h>
#include<unordered_map>
#include<vector>
#include "list.h"
#include "student.h"
using namespace std;

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
    cout<<unitbuf;
    cout<<left;
    student_sys ss;
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
    return 0;
}
