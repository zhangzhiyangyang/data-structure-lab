#include<iostream>
using namespace std;

typedef char ElemType;
//队列结点结构体
typedef struct QNode {
    ElemType data;        //结点存放的数据
    struct QNode* next;   //指向下一个结点的指针
} QNode;

//链队列结构：保存队头、队尾两个指针
typedef struct {
    QNode* front; //队头指针，出队用
    QNode* rear;  //队尾指针，入队用
} LinkQueue;

//(1)初始化链队列
void InitQueue(LinkQueue& q) {
    q.front = q.rear = nullptr;
}
/*解析：
front、rear都置空指针，链表没有任何结点，队列为空。
链队列用单链表实现：结点动态new创建，不需要预先分配固定大小数组。
*/

//(2)判断链队列是否为空
bool QueueEmpty(LinkQueue q) {
    return q.front == nullptr;
}
/*解析：
front等于空指针，代表链表没有结点，队列为空。
*/

//入队：在链表尾部新增结点
bool EnQueue(LinkQueue& q, ElemType e) {
    QNode* p = new QNode; //动态申请新结点内存
    p->data = e;
    p->next = nullptr;    //新结点后面暂时没有节点

    if (QueueEmpty(q)) {
        //队列为空：新节点既是队头，也是队尾
        q.front = q.rear = p;
    }
    else {
        //队列不为空：接到队尾后面，更新rear
        q.rear->next = p;
        q.rear = p;
    }
    return true;
}
/*解析：
链队列入队永远在尾部。
new 是在堆内存开辟结点；链队列没有最大容量限制，内存足够就可以一直新增元素。
*/

//出队：删除链表最前面的结点
bool DeQueue(LinkQueue& q, ElemType& e) {
    if (QueueEmpty(q))
        return false; //空队列，无法出队

    QNode* p = q.front;   //p保存要删除的队头结点
    e = p->data;          //取出结点内元素，引用带回main函数
    q.front = q.front->next; //队头后移

    //特殊情况：弹出最后一个结点，队列变空，rear也要置空
    if (q.rear == p)
        q.rear = nullptr;

    delete p; //释放结点堆内存，防止内存泄漏
    return true;
}
/*解析：
队列FIFO，只能删除队头。
如果删掉这个结点后队列空了，必须把rear置nullptr，不然rear变成野指针。
delete释放new出来的内存，这是链结构必须做的！
*/

//(7)销毁链队列，释放全部结点
void DestroyQueue(LinkQueue& q) {
    QNode* p, * s;
    p = q.front;
    while (p != nullptr) {
        s = p;
        p = p->next;
        delete s; //逐个释放每一个结点
    }
    q.front = q.rear = nullptr;
}
/*解析：
循环遍历链表，从队头到队尾逐个delete所有节点。
链队列所有结点都是new出来的，如果不delete，会内存泄漏。
*/

//main主函数，严格按照实验7条要求
int main()
{
    LinkQueue q;
    ElemType e;
    //(1)初始化链队列q
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
    //(6)输出出队序列
    cout << "完整出队序列：";
    while (!QueueEmpty(q)) {
        DeQueue(q, e);
        cout << e << " ";
    }
    cout << endl;
    //(7)释放链队列
    DestroyQueue(q);
    return 0;
}
