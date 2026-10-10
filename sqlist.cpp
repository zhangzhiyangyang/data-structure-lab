#include <iostream>
using namespace std;

#define MaxSize 50
typedef char ElemType;

// 顺序表结构定义
typedef struct {
    ElemType data[MaxSize];
    int length;
} SqList;

// (1)初始化顺序表L
void InitList(SqList& L) {
    L.length = 0;
}

// (2)依次插入a,b,c,d,e元素
bool CreateList(SqList& L) {
    ElemType arr[] = { 'a','b','c','d','e' };
    int n = sizeof(arr) / sizeof(arr[0]);
    if (n > MaxSize) return false;
    for (int i = 0; i < n; i++) {
        L.data[i] = arr[i];
    }
    L.length = n;
    return true;
}

// (3)输出顺序表L
void PrintList(SqList L) {
    for (int i = 0; i < L.length; i++) {
        cout << L.data[i] << " ";
    }
    cout << endl;
}

// (4)输出顺序表L的长度
int GetLength(SqList L) {
    return L.length;
}

// (5)判断顺序表L是否为空
bool IsEmpty(SqList L) {
    return L.length == 0;
}

// (6)输出顺序表L的第3个元素
bool GetElem(SqList L, int i, ElemType& e) {
    if (i<1 || i>L.length) return false;
    e = L.data[i - 1];
    return true;
}

// (7)输出元素a的位置
int LocateElem(SqList L, ElemType e) {
    for (int i = 0; i < L.length; i++) {
        if (L.data[i] == e) return i + 1;
    }
    return 0; //找不到返回0
}

// (8)在第4个元素的位置上插入f元素
bool ListInsert(SqList& L, int i, ElemType e) {
    if (i<1 || i>L.length + 1 || L.length >= MaxSize) return false;
    for (int j = L.length; j >= i; j--) {
        L.data[j] = L.data[j - 1];
    }
    L.data[i - 1] = e;
    L.length++;
    return true;
}

// (9)输出顺序表L（复用PrintList）

// (10)删除顺序表L的第3个元素
bool ListDelete(SqList& L, int i, ElemType& e) {
    if (i<1 || i>L.length) return false;
    e = L.data[i - 1];
    for (int j = i; j < L.length; j++) {
        L.data[j - 1] = L.data[j];
    }
    L.length--;
    return true;
}

// (11)输出顺序表L（复用PrintList）

// (12)释放顺序表L，顺序表静态数组只需置length=0
void DestroyList(SqList& L) {
    L.length = 0;
}

int main() {
    SqList L;
    ElemType e;
    int pos;

    cout << "(1)初始化顺序表" << endl;
    InitList(L);

    cout << "(2)依次插入a,b,c,d,e" << endl;
    CreateList(L);

    cout << "(3)顺序表L：";
    PrintList(L);

    cout << "(4)顺序表长度：" << GetLength(L) << endl;

    cout << "(5)是否为空：" << (IsEmpty(L) ? "是" : "否") << endl;

    GetElem(L, 3, e);
    cout << "(6)第3个元素：" << e << endl;

    pos = LocateElem(L, 'a');
    cout << "(7)元素a的位置：" << pos << endl;

    ListInsert(L, 4, 'f');
    cout << "(8)第4位插入f" << endl;

    cout << "(9)插入后顺序表：";
    PrintList(L);

    ListDelete(L, 3, e);
    cout << "(10)删除第3个元素，被删元素：" << e << endl;

    cout << "(11)删除后顺序表：";
    PrintList(L);

    DestroyList(L);
    cout << "(12)释放顺序表完成" << endl;

    return 0;
}
