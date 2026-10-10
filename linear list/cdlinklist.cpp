#include <iostream>
using namespace std;
typedef char ElemType;

//循环双链表结点：有前后两个指针，而且首尾结点和头结点互相连接形成双向环
typedef struct CDNode {
    ElemType data;
    struct CDNode* prior; //指向前一个结点
    struct CDNode* next;  //指向后一个结点
}CDNode, * CDLinkList;

//(1)初始化循环双链表，创建头结点
void InitCDList(CDLinkList& h)
{
    h = new CDNode;
    h->next = h;  //头结点后继指向自己
    h->prior = h; //头结点前驱也指向自己，空表双向环
}

//(2)尾插法插入元素
bool CDList_TailInsert(CDLinkList& h, ElemType e)
{
    //循环双链表的优势：h->prior永远直接就是最后一个结点，不用遍历找尾巴，O(1)效率
    CDNode* tail = h->prior;
    CDNode* s = new CDNode;
    s->data = e;
    //新结点接到尾巴和头结点之间
    s->next = h;        //新结点后继指向头
    s->prior = tail;    //新结点前驱指向原来的尾巴
    tail->next = s;    //原来尾巴的后继指向新结点
    h->prior = s;       //头结点的前驱更新成新结点
    return true;
}

//(3)输出循环双链表
void PrintCDList(CDLinkList h)
{
    CDNode* p = h->next;
    while (p != h)
    {
        cout << p->data << " ";
        p = p->next;
    }
    cout << endl;
}

//(4)求长度
int CDList_Length(CDLinkList h)
{
    int cnt = 0;
    CDNode* p = h->next;
    while (p != h) { cnt++; p = p->next; }
    return cnt;
}

//(5)判空
bool CDList_Empty(CDLinkList h)
{
    return h->next == h;
}

//(6)按位查找
bool CDList_GetElem(CDLinkList h, int i, ElemType& e)
{
    if (i < 1) return false;
    CDNode* p = h->next;
    int j = 1;
    while (p != h && j < i) { p = p->next; j++; }
    if (p == h) return false;
    e = p->data;
    return true;
}

//(7)按值查找
int CDList_LocateElem(CDLinkList h, ElemType e)
{
    CDNode* p = h->next;
    int pos = 1;
    while (p != h) {
        if (p->data == e) return pos;
        p = p->next; pos++;
    }
    return 0;
}

//(8)第i个位置插入元素
bool CDList_Insert(CDLinkList& h, int i, ElemType e)
{
    if (i < 1) return false;
    CDNode* p = h->next;
    int j = 1;
    while (p != h && j < i) { p = p->next; j++; }
    if (p == h) return false;
    CDNode* s = new CDNode;
    s->data = e;
    //插入时必须修改4个指针，两个方向都要接上，不然会断链
    s->prior = p->prior; //新结点前驱接p的前驱
    s->next = p;         //新结点后继接p
    p->prior->next = s;  //p前面的结点后继接新结点
    p->prior = s;        //p的前驱更新成新结点
    return true;
}

//(10)删除第i个元素
bool CDList_Delete(CDLinkList& h, int i, ElemType& e)
{
    if (i < 1) return false;
    CDNode* p = h->next;
    int j = 1;
    while (p != h && j < i) { p = p->next; j++; }
    if (p == h) return false;
    e = p->data;
    //删除时改两个指针，前后结点直接相连，跳过p
    p->prior->next = p->next;
    p->next->prior = p->prior;
    delete p;
    return true;
}

//(12)释放整个循环双链表
void DestroyCDList(CDLinkList& h)
{
    CDNode* p = h->next;
    while (p != h)
    {
        CDNode* q = p;
        p = p->next;
        delete q;
    }
    delete h;
    h = NULL;
}

int main()
{
    CDLinkList h;
    ElemType ch;
    //(1)初始化
    InitCDList(h);
    //(2)插入a,b,c,d,e
    CDList_TailInsert(h, 'a');
    CDList_TailInsert(h, 'b');
    CDList_TailInsert(h, 'c');
    CDList_TailInsert(h, 'd');
    CDList_TailInsert(h, 'e');
    //(3)输出
    cout << "(3)循环双链表："; PrintCDList(h);
    //(4)长度
    cout << "(4)链表长度：" << CDList_Length(h) << endl;
    //(5)判空
    cout << "(5)是否为空：" << (CDList_Empty(h) ? "是" : "否") << endl;
    //(6)第3个元素
    CDList_GetElem(h, 3, ch);
    cout << "(6)第3个元素：" << ch << endl;
    //(7)查找a
    cout << "(7)元素a的位序：" << CDList_LocateElem(h, 'a') << endl;
    //(8)第4位插入f
    CDList_Insert(h, 4, 'f');
    //(9)输出
    cout << "(9)插入f后："; PrintCDList(h);
    //(10)删除第3个元素
    CDList_Delete(h, 3, ch);
    //(11)输出
    cout << "(11)删除第3个后："; PrintCDList(h);
    //(12)释放
    DestroyCDList(h);
    cout << "(12)循环双链表释放完成\n";
    return 0;
}
