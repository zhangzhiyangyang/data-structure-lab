//#include <iostream>
//using namespace std;
//#define MaxSize 100   //定义顺序表最大容量
//typedef char ElemType;  //元素类型为字符，存放a,b,c,d,e,f
//
////顺序表结构体：连续数组存储，length记录当前有效元素个数
//typedef struct {
//    ElemType data[MaxSize];
//    int length;
//} SqList;
//
////(1)初始化顺序表
//void InitList(SqList& L)
//{
//    L.length = 0;  //有效元素置0，代表空表
//}
//
////尾部插入元素，用来依次添加a,b,c,d,e
//bool ListInsert_Tail(SqList& L, ElemType e)
//{
//    if (L.length >= MaxSize) return false; //判断是否满了，满了不能插入
//    L.data[L.length] = e;  //放到当前最后位置
//    L.length++;            //有效元素数量+1
//    return true;
//}
//
////(3)遍历输出顺序表所有元素
//void PrintList(SqList L)
//{
//    for (int i = 0; i < L.length; i++)
//        cout << L.data[i] << " ";
//    cout << endl;
//}
//
////(4)获取顺序表当前长度
//int ListLength(SqList L)
//{
//    return L.length;
//}
//
////(5)判断顺序表是否为空
//bool ListEmpty(SqList L)
//{
//    return L.length == 0; //长度为0就是空表
//}
//
////(6)按位查找，取出第i个位序的元素
//bool GetElem(SqList L, int i, ElemType& e)
//{
//    if (i<1 || i>L.length) return false; //位序非法，直接返回失败
//    e = L.data[i - 1]; //题目位序从1开始，数组下标从0，所以i-1
//    return true;
//}
//
////(7)按值查找，查找元素e，返回它的位序，找不到返回0
//int LocateElem(SqList L, ElemType e)
//{
//    for (int i = 0; i < L.length; i++)
//        if (L.data[i] == e) return i + 1; //找到，返回位序(下标+1)
//    return 0; //遍历完没找到，返回0
//}
//
////(8)在第i个位序插入元素e
//bool ListInsert(SqList& L, int i, ElemType e)
//{
//    if (i<1 || i>L.length + 1 || L.length >= MaxSize) return false;
//    //从后往前，把i位置后面所有元素全部后移一位，空出位置
//    for (int j = L.length; j >= i; j--)
//        L.data[j] = L.data[j - 1];
//    L.data[i - 1] = e;  //放入新元素
//    L.length++;       //长度+1
//    return true;
//}
//
////(10)删除第i个位序元素，e保存被删掉的值
//bool ListDelete(SqList& L, int i, ElemType& e)
//{
//    if (i<1 || i>L.length) return false; //检查位序合法性
//    e = L.data[i - 1]; //取出要删除的元素
//    //i后面的元素全部向前移动，覆盖被删除位置
//    for (int j = i; j < L.length; j++)
//        L.data[j - 1] = L.data[j];
//    L.length--; //长度-1
//    return true;
//}
//
//int main()
//{
//    SqList L;
//    ElemType ch;
//    //(1)初始化顺序表
//    InitList(L);
//    //(2)依次插入a,b,c,d,e
//    ListInsert_Tail(L, 'a');
//    ListInsert_Tail(L, 'b');
//    ListInsert_Tail(L, 'c');
//    ListInsert_Tail(L, 'd');
//    ListInsert_Tail(L, 'e');
//    //(3)输出顺序表
//    cout << "(3)顺序表："; PrintList(L);
//    //(4)输出长度
//    cout << "(4)表长度：" << ListLength(L) << endl;
//    //(5)判断是否为空
//    cout << "(5)是否为空：" << (ListEmpty(L) ? "是" : "否") << endl;
//    //(6)取第3个元素
//    GetElem(L, 3, ch);
//    cout << "(6)第3个元素：" << ch << endl;
//    //(7)查找a的位置
//    cout << "(7)元素a的位序：" << LocateElem(L, 'a') << endl;
//    //(8)第4位插入f
//    ListInsert(L, 4, 'f');
//    //(9)输出顺序表
//    cout << "(9)插入f后："; PrintList(L);
//    //(10)删除第3个元素
//    ListDelete(L, 3, ch);
//    //(11)输出顺序表
//    cout << "(11)删除第3个后："; PrintList(L);
//    //(12)释放顺序表：静态数组，重置长度即释放
//    InitList(L);
//    cout << "(12)顺序表释放完成\n";
//    return 0;
//}
