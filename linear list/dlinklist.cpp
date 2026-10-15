//#include <iostream>
//using namespace std;
//typedef char ElemType;
//
////双链表结点：prior前驱指针，next后继指针，可以双向走
//typedef struct DNode {
//    ElemType data;
//    struct DNode* prior; //指向前一个结点
//    struct DNode* next;  //指向后一个结点
//}DNode, * DLinkList;
//
////(1)初始化双链表，创建头结点
//void InitDList(DLinkList& h)
//{
//    h = new DNode;
//    h->prior = NULL;
//    h->next = NULL;
//}
//
////(2)尾部插入元素
//bool DList_TailInsert(DLinkList& h, ElemType e)
//{
//    DNode* s = new DNode; //新建结点
//    s->data = e;
//    s->next = NULL;
//    DNode* p = h;
//    while (p->next != NULL) p = p->next; //找到尾结点
//    p->next = s;
//    s->prior = p; //新结点前驱指向原来尾结点
//    return true;
//}
//
////(3)输出双链表
//void PrintDList(DLinkList h)
//{
//    DNode* p = h->next;
//    while (p != NULL)
//    {
//        cout << p->data << " ";
//        p = p->next;
//    }
//    cout << endl;
//}
//
////(4)求长度
//int DList_Length(DLinkList h)
//{
//    int cnt = 0;
//    DNode* p = h->next;
//    while (p) { cnt++; p = p->next; }
//    return cnt;
//}
//
////(5)判断是否为空
//bool DList_Empty(DLinkList h)
//{
//    return h->next == NULL;
//}
//
////(6)按位查找
//bool DList_GetElem(DLinkList h, int i, ElemType& e)
//{
//    if (i < 1) return false;
//    DNode* p = h->next;
//    int j = 1;
//    while (p && j < i) { p = p->next; j++; }
//    if (!p) return false;
//    e = p->data;
//    return true;
//}
//
////(7)按值查找
//int DList_LocateElem(DLinkList h, ElemType e)
//{
//    DNode* p = h->next;
//    int pos = 1;
//    while (p) {
//        if (p->data == e) return pos;
//        p = p->next; pos++;
//    }
//    return 0;
//}
//
////(8)第i位置插入元素
//bool DList_Insert(DLinkList& h, int i, ElemType e)
//{
//    if (i < 1) return false;
//    DNode* p = h->next;
//    int j = 1;
//    while (p && j < i) { p = p->next; j++; }
//    if (!p) return false;
//    DNode* s = new DNode;
//    s->data = e;
//    s->prior = p->prior; //新结点前驱 = p的前驱
//    s->next = p;         //新结点后继 = p
//    p->prior->next = s;  //p前面结点指向s
//    p->prior = s;        //p的前驱改成s
//    return true;
//}
//
////(10)删除第i个结点
//bool DList_Delete(DLinkList& h, int i, ElemType& e)
//{
//    if (i < 1) return false;
//    DNode* p = h->next;
//    int j = 1;
//    while (p && j < i) { p = p->next; j++; }
//    if (!p) return false;
//    e = p->data;
//    p->prior->next = p->next; //前结点跳过p
//    if (p->next != NULL)
//        p->next->prior = p->prior; //后结点前驱指向p的前驱
//    delete p;
//    return true;
//}
//
////(12)释放双链表
//void DestroyDList(DLinkList& h)
//{
//    DNode* p = h->next, * q;
//    while (p)
//    {
//        q = p;
//        p = p->next;
//        delete q;
//    }
//    delete h;
//    h = NULL;
//}
//
//int main()
//{
//    DLinkList h;
//    ElemType ch;
//    //(1)初始化
//    InitDList(h);
//    //(2)插入a,b,c,d,e
//    DList_TailInsert(h, 'a');
//    DList_TailInsert(h, 'b');
//    DList_TailInsert(h, 'c');
//    DList_TailInsert(h, 'd');
//    DList_TailInsert(h, 'e');
//    //(3)输出
//    cout << "(3)双链表："; PrintDList(h);
//    //(4)长度
//    cout << "(4)链表长度：" << DList_Length(h) << endl;
//    //(5)判空
//    cout << "(5)是否为空：" << (DList_Empty(h) ? "是" : "否") << endl;
//    //(6)第3个元素
//    DList_GetElem(h, 3, ch);
//    cout << "(6)第3个元素：" << ch << endl;
//    //(7)查找a
//    cout << "(7)元素a的位序：" << DList_LocateElem(h, 'a') << endl;
//    //(8)第4位插入f
//    DList_Insert(h, 4, 'f');
//    //(9)输出
//    cout << "(9)插入f后："; PrintDList(h);
//    //(10)删除第3个
//    DList_Delete(h, 3, ch);
//    //(11)输出
//    cout << "(11)删除第3个后："; PrintDList(h);
//    //(12)释放
//    DestroyDList(h);
//    cout << "(12)双链表释放完成\n";
//    return 0;
//}
