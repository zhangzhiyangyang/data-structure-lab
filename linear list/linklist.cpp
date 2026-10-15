//#include <iostream>
//using namespace std;
//typedef char ElemType;
//
////单链表结点：data存数据，next指针指向下一个结点
//typedef struct LinkNode {
//    ElemType data;
//    struct LinkNode* next;
//}LinkNode, * LinkList;
//
////(1)初始化单链表，创建头结点（头结点不存数据）
//void InitList(LinkList& h)
//{
//    h = new LinkNode; //开辟头结点空间
//    h->next = NULL;   //头结点初始指向空，代表空链表
//}
//
////(2)尾插法：在链表尾部插入新元素，用来添加a,b,c,d,e
//bool ListTailInsert(LinkList& h, ElemType e)
//{
//    LinkNode* p = new LinkNode; //新建结点
//    p->data = e;                //存入字符
//    p->next = NULL;             //新结点作为尾结点，next置空
//    LinkNode* tail = h;         //tail指针用来寻找链表尾部
//    while (tail->next != NULL)
//        tail = tail->next;      //循环走到最后一个结点
//    tail->next = p;             //尾结点指向新结点
//    return true;
//}
//
////(3)遍历输出链表所有元素
//void PrintList(LinkList h)
//{
//    LinkNode* p = h->next; //p从第一个数据结点开始
//    while (p != NULL)
//    {
//        cout << p->data << " ";
//        p = p->next; //指针往后走
//    }
//    cout << endl;
//}
//
////(4)统计链表长度
//int ListLength(LinkList h)
//{
//    int cnt = 0;
//    LinkNode* p = h->next;
//    while (p != NULL) { cnt++; p = p->next; } //每遇到结点计数+1
//    return cnt;
//}
//
////(5)判断链表是否为空
//bool ListEmpty(LinkList h)
//{
//    return h->next == NULL; //头结点后面没有数据结点，为空
//}
//
////(6)按位查找，取出第i个位序元素
//bool GetElem(LinkList h, int i, ElemType& e)
//{
//    if (i < 1) return false; //位序不能小于1
//    LinkNode* p = h->next;
//    int j = 1;
//    while (p && j < i) { p = p->next; j++; } //循环走到第i个结点
//    if (!p) return false;
//    e = p->data;
//    return true;
//}
//
////(7)按值查找，返回元素e的位序，找不到返回0
//int LocateElem(LinkList h, ElemType e)
//{
//    LinkNode* p = h->next;
//    int pos = 1;
//    while (p) {
//        if (p->data == e) return pos; //找到，返回当前位序
//        p = p->next; pos++;
//    }
//    return 0;
//}
//
////(8)在第i个位序插入元素e
//bool ListInsert(LinkList& h, int i, ElemType e)
//{
//    if (i < 1) return false;
//    LinkNode* p = h;
//    int j = 0;
//    while (p && j < i - 1) { p = p->next; j++; } //走到i-1位置结点
//    if (!p) return false;
//    LinkNode* s = new LinkNode; //新建结点
//    s->data = e;
//    s->next = p->next; //新结点指向原来i位置的结点
//    p->next = s;       //前一个结点指向新结点
//    return true;
//}
//
////(10)删除第i个位序元素，e接收被删的值
//bool ListDelete(LinkList& h, int i, ElemType& e)
//{
//    if (i < 1) return false;
//    LinkNode* p = h;
//    int j = 0;
//    while (p->next && j < i - 1) { p = p->next; j++; } //找到i-1结点
//    if (!p->next) return false;
//    LinkNode* q = p->next; //q指向待删除结点
//    e = q->data;
//    p->next = q->next;     //跳过q结点，断开链接
//    delete q;              //释放被删结点内存，防止内存泄漏
//    return true;
//}
//
////(12)释放整个链表，释放所有结点内存
//void DestroyList(LinkList& h)
//{
//    LinkNode* p = h->next, * q;
//    while (p)
//    {
//        q = p;
//        p = p->next;
//        delete q;
//    }
//    delete h; //释放头结点
//    h = NULL;
//}
//
//int main()
//{
//    LinkList h;
//    ElemType ch;
//    //(1)初始化单链表
//    InitList(h);
//    //(2)尾插依次放入a,b,c,d,e
//    ListTailInsert(h, 'a');
//    ListTailInsert(h, 'b');
//    ListTailInsert(h, 'c');
//    ListTailInsert(h, 'd');
//    ListTailInsert(h, 'e');
//    //(3)输出链表
//    cout << "(3)单链表："; PrintList(h);
//    //(4)输出长度
//    cout << "(4)链表长度：" << ListLength(h) << endl;
//    //(5)判空
//    cout << "(5)是否为空：" << (ListEmpty(h) ? "是" : "否") << endl;
//    //(6)取第3个元素
//    GetElem(h, 3, ch);
//    cout << "(6)第3个元素：" << ch << endl;
//    //(7)查找a的位置
//    cout << "(7)元素a的位序：" << LocateElem(h, 'a') << endl;
//    //(8)第4位插入f
//    ListInsert(h, 4, 'f');
//    //(9)输出链表
//    cout << "(9)插入f后："; PrintList(h);
//    //(10)删除第3个元素
//    ListDelete(h, 3, ch);
//    //(11)输出链表
//    cout << "(11)删除第3个后："; PrintList(h);
//    //(12)释放链表
//    DestroyList(h);
//    cout << "(12)单链表释放完成\n";
//    return 0;
//}
