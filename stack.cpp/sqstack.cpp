#include<iostream>
using namespace std;

#define MaxSize 100   //定义顺序栈数组最大容量
typedef char ElemType; //栈内元素类型为char字符

//顺序栈结构体
typedef struct {
    ElemType data[MaxSize]; //数组，存放栈元素
    int top;    //栈顶标记，top=-1代表空栈
} SqStack;

//(1)初始化栈
void InitStack(SqStack& s) {
    s.top = -1;
}
/*解析：
引用&S，直接修改外面的栈变量。
top = -1：约定没有元素。top的值等于栈顶元素在数组的下标。
*/

//(2)判断栈是否为空
bool StackEmpty(SqStack s) {
    return s.top == -1;
}
/*解析：
如果top等于-1，返回true，栈空；否则返回false。
*/

//入栈操作
bool Push(SqStack& s, ElemType e) {
    if (s.top == MaxSize - 1)
        return false;  //top到数组末尾，栈满，入栈失败
    s.data[++s.top] = e;
    return true;
}
/*解析：
++s.top：top先+1，再存元素。
例：top=-1，++top → 0，元素存入data[0]
*/

//出栈操作
bool Pop(SqStack& s, ElemType& e) {
    if (StackEmpty(s))
        return false; //栈空，不能出栈
    e = s.data[s.top--];
    return true;
}
/*解析：
先取出data[top]给e，再top减1。
e加引用&，可以把弹出的元素带回主函数。
*/

//(7)销毁顺序栈
void DestroyStack(SqStack& s) {
    s.top = -1;
}
/*解析：
顺序栈是静态数组，内存自动在栈区，只需要重置top=-1，逻辑上清空栈。
*/

//主函数，按照实验要求一步步执行
int main()
{
    SqStack s;
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
