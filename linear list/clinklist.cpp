//#include <iostream>
//using namespace std;
//typedef char ElemType;
//
////循环单链表结点：和普通单链表结构一样，只是最后一个结点不指向NULL，指向头结点
//typedef struct CNode {
//    ElemType data;
//    struct CNode* next;
//}CNode, * CLinkList;
//
////(1)初始化循环单链表，创建头结点
//void InitCList(CLinkList& h)
//{
//    h = new CNode;
//    h->next = h; //头结点自己指向自己，形成一个环，空表状态
//}
//
////(2)尾插法插入元素，依次放a,b,c,d,e
//bool CList_TailInsert(CLinkList& h, ElemType e)
//{
//    CNode* p = h;
//    //找尾结点：普通单链表判断条件是p->next!=NULL，循环链表是p->next回到头结点h就说明到尾巴了
//    while (p->next != h)
//        p = p->next;
//    CNode* s = new CNode;
//    s->data = e;
//    s->next = h; //新结点接在尾巴，后继依然指向头h，保持环结构
//    p->next = s;
//    return true;
//}
//
////(3)输出循环单链表全部元素
//void PrintCList(CLinkList h)
//{
//    CNode* p = h->next; //从第一个数据结点开始
//    //循环结束条件：走了一圈又回到头结点h，就停止，不会死循环
//    while (p != h)
//    {
//        cout << p->data << " ";
//        p = p->next;
//    }
//    cout << endl;
//}
//
////(4)求循环单链表长度
//int CList_Length(CLinkList h)
//{
//    int cnt = 0;
//    CNode* p = h->next;
//    while (p != h) { cnt++; p = p->next; }
//    return cnt;
//}
//
////(5)判断循环单链表是否为空
//bool CList_Empty(CLinkList h)
//{
//    //头结点的next如果指向自己，说明没有任何数据结点，是空表
//    return h->next == h;
//}
//
////(6)按位查找，取第i个元素
//bool CList_GetElem(CLinkList h, int i, ElemType& e)
//{
//    if (i < 1) return false;
//    CNode* p = h->next;
//    int j = 1;
//    //走到第i个结点，或者绕回头结点就停止
//    while (p != h && j < i) { p = p->next; j++; }
//    if (p == h) return false; //走回头都没找到，说明i超过长度了
//    e = p->data;
//    return true;
//}
//
////(7)按值查找，返回元素e的位序，找不到返回0
//int CList_LocateElem(CLinkList h, ElemType e)
//{
//    CNode* p = h->next;
//    int pos = 1;
//    while (p != h) {
//        if (p->data == e) return pos;
//        p = p->next; pos++;
//    }
//    return 0;
//}
//
////(8)在第i个位序插入元素e
//bool CList_Insert(CLinkList& h, int i, ElemType e)
//{
//    if (i < 1) return false;
//    CNode* p = h;
//    int j = 0;
//    //先找到i位置的前一个结点
//    while (p->next != h && j < i - 1) { p = p->next; j++; }
//    if (p->next == h && j < i - 1) return false; //插入位置越界
//    CNode* s = new CNode;
//    s->data = e;
//    s->next = p->next; //新结点接上原来i位置的结点
//    p->next = s;       //前一个结点接上新结点
//    return true;
//}
//
////(10)删除第i个位序元素，e保存被删的值
//bool CList_Delete(CLinkList& h, int i, ElemType& e)
//{
//    if (i < 1) return false;
//    CNode* p = h;
//    int j = 0;
//    //找到待删结点的前一个结点
//    while (p->next != h && j < i - 1) { p = p->next; j++; }
//    if (p->next == h) return false; //删除位置越界
//    CNode* q = p->next; //q指向要删的结点
//    e = q->data;
//    p->next = q->next;  //前结点跳过q，链接到q的后继
//    delete q;           //释放被删结点内存
//    return true;
//}
//
////(12)释放整个循环单链表
//void DestroyCList(CLinkList& h)
//{
//    CNode* p = h->next;
//    //注意：因为是环，必须先存好下一个结点再删当前，不然删完就找不到环了
//    while (p != h)
//    {
//        CNode* q = p;
//        p = p->next;
//        delete q;
//    }
//    delete h; //最后释放头结点
//    h = NULL;
//}
//
//int main()
//{
//    CLinkList h;
//    ElemType ch;
//    //(1)初始化
//    InitCList(h);
//    //(2)尾插a,b,c,d,e
//    CList_TailInsert(h, 'a');
//    CList_TailInsert(h, 'b');
//    CList_TailInsert(h, 'c');
//    CList_TailInsert(h, 'd');
//    CList_TailInsert(h, 'e');
//    //(3)输出
//    cout << "(3)循环单链表："; PrintCList(h);
//    //(4)长度
//    cout << "(4)链表长度：" << CList_Length(h) << endl;
//    //(5)判空
//    cout << "(5)是否为空：" << (CList_Empty(h) ? "是" : "否") << endl;
//    //(6)第3个元素
//    CList_GetElem(h, 3, ch);
//    cout << "(6)第3个元素：" << ch << endl;
//    //(7)查找a的位置
//    cout << "(7)元素a的位序：" << CList_LocateElem(h, 'a') << endl;
//    //(8)第4位插入f
//    CList_Insert(h, 4, 'f');
//    //(9)输出
//    cout << "(9)插入f后："; PrintCList(h);
//    //(10)删除第3个元素
//    CList_Delete(h, 3, ch);
//    //(11)输出
//    cout << "(11)删除第3个后："; PrintCList(h);
//    //(12)释放
//    DestroyCList(h);
//    cout << "(12)循环单链表释放完成\n";
//    return 0;
//}
