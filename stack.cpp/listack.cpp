#include<iostream>
using namespace std;

typedef char ElemType;
//链栈结点结构
typedef struct LNode {
    ElemType data;       //存放结点数据
    struct LNode* next;  //指向下一个结点
} LNode, * LiStack;
/*解析：
LiStack 是 LNode* 的别名，代表栈顶指针。
链栈用单链表实现，链表头部作为栈顶。
*/

//(1)初始化链栈
void InitStack(LiStack& s) {
    s = nullptr;
}
/*解析：
s是栈顶指针，初始赋值nullptr，代表链表为空，栈里没有节点。
*/

//(2)判断栈是否为空
bool StackEmpty(LiStack s) {
    return s == nullptr;
}
/*解析：
栈顶指针是空，栈就为空。
*/

//入栈（头插法）
bool Push(LiStack& s, ElemType e) {
    LNode* p = new LNode; //新建节点，动态申请内存
    p->data = e;
    p->next = s;  //新节点指向原来的栈顶
    s = p;        //更新栈顶为新节点
    return true;
}
/*解析：
链栈全部头插，新元素直接放在链表最前面，作为新栈顶。
不需要移动后面元素，效率很高。
*/

//出栈
bool Pop(LiStack& s, ElemType& e) {
    if (StackEmpty(s))
        return false;
    LNode* p = s;   //p保存当前栈顶结点
    e = p->data;    //取出栈顶数据
    s = s->next;    //栈顶指针往后移动一位
    delete p;       //释放弹出节点的内存，防止内存泄漏
    return true;
}
/*解析：
弹出栈顶节点，一定要delete释放动态分配的内存。
e带引用，把弹出元素传回main。
*/

//(7)销毁链栈，释放全部结点
void DestroyStack(LiStack& s) {
    LNode* p, * q;
    p = s;
    while (p != nullptr) {
        q = p;
        p = p->next;
        delete q;
    }
    s = nullptr;
}
/*解析：
循环遍历链表，逐个delete所有节点。
链栈是new出来的堆内存，不delete会内存泄漏。
最后栈顶指针置空。
*/

//主程序，按实验要求执行
int main()
{
    LiStack s;
    ElemType e;
    //(1)初始化栈s
    InitStack(s);
    //(2)判断栈s是否为空
    if (StackEmpty(s)) cout << "栈为空" << endl;
    //(3)依次进栈a,b,c,d,e
    Push(s, 'a');
    Push(s, 'b');
    Push(s, 'c');
    Push(s, 'd');
    Push(s, 'e');
    //(4)判断栈s是否非空
    if (!StackEmpty(s)) cout << "栈非空" << endl;
    //(5)输出出栈序列
    cout << "出栈序列：";
    while (!StackEmpty(s)) {
        Pop(s, e);
        cout << e << " ";
    }
    cout << endl;
    //(6)判断栈s是否为空
    if (StackEmpty(s)) cout << "栈为空" << endl;
    //(7)释放栈
    DestroyStack(s);
    return 0;
}
