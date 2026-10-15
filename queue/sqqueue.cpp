#include<iostream>
using namespace std;

#define MaxSize 6        //数组最大容量，设置为6
typedef char ElemType;   //队列存储元素类型为字符char

//环形队列结构体
typedef struct {
    ElemType data[MaxSize]; //存放队列元素的数组
    int front;  //队头指针：指向队列队头元素
    int rear;   //队尾指针：指向队尾元素的下一个空位
} SqQueue;

//(1)初始化环形队列
void InitQueue(SqQueue& q) {
    q.front = q.rear = 0;
}
/*解析：
引用 &q，直接修改外部队列变量。
初始化front、rear都等于0，代表队列为空。
环形队列约定：牺牲一个数组位置，用来区分【队满】和【队空】。
队空条件：front == rear
队满条件：(rear + 1) % MaxSize == front
*/

//(2)判断队列是否为空
bool QueueEmpty(SqQueue q) {
    return q.front == q.rear;
}
/*解析：
如果队头下标等于队尾下标，队列里没有元素，返回true（空）；否则返回false。
bool返回值：反馈判断结果。
*/

//入队操作：在队尾添加元素
bool EnQueue(SqQueue& q, ElemType e) {
    //判断队列是否已满
    if ((q.rear + 1) % MaxSize == q.front)
        return false; //队列满，入队失败，返回false

    q.data[q.rear] = e;       //把元素放入rear指向的空位
    q.rear = (q.rear + 1) % MaxSize; //rear向后移动，取模实现环形
    return true; //入队成功
}
/*解析：
% MaxSize 取模是环形核心！当rear走到数组末尾（MaxSize-1），+1后取模直接回到下标0，实现环形循环。
顺序表是一次性填满数组；环形队列头尾可以循环复用空间。
*/

//出队操作：队头取出元素
bool DeQueue(SqQueue& q, ElemType& e) {
    if (QueueEmpty(q))
        return false; //队列为空，不能出队，返回false

    e = q.data[q.front];              //取出front位置的队头元素
    q.front = (q.front + 1) % MaxSize;//队头指针向后移动
    return true;
}
/*解析：
参数 ElemType &e 引用传递，把弹出的元素带回主函数。
队列规则：先进先出FIFO，只能从队头出队。
*/

//(7)释放环形队列
void DestroyQueue(SqQueue& q) {
    q.front = q.rear = 0;
}
/*解析：
环形队列底层是静态数组，内存在栈上，不需要delete。
只需要重置front、rear为0，逻辑上清空队列。
*/

//主函数，严格按照实验题目7个步骤
int main()
{
    SqQueue q;
    ElemType e;
    //(1)初始化队列q
    InitQueue(q);
    //(2)判断队列q是否为空
    if (QueueEmpty(q)) cout << "队列为空" << endl;
    //(3)依次进队 a,b,c
    EnQueue(q, 'a');
    EnQueue(q, 'b');
    EnQueue(q, 'c');
    //(4)出队一个元素，输出该元素
    DeQueue(q, e);
    cout << "出队元素：" << e << endl;
    //(5)依次进队 d,e,f
    EnQueue(q, 'd');
    EnQueue(q, 'e');
    EnQueue(q, 'f');
    //(6)输出全部出队序列
    cout << "完整出队序列：";
    while (!QueueEmpty(q)) {
        DeQueue(q, e);
        cout << e << " ";
    }
    cout << endl;
    //(7)释放队列
    DestroyQueue(q);
    return 0;
}
