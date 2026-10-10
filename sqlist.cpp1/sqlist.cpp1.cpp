#include <iostream>
using namespace std;

#define MaxSize 50
typedef char ElemType;

//顺序表结
typedef struct {
    ElemType data[MaxSize];
    int length;
} SqList;

//1.初始化
void InitList(SqList& L) {
    L.length = 0;
}

//插入元素，i是位序从1开始
int ListInsert(SqList& L, int i, ElemType e) {
    if (i<1 || i>L.length + 1) return 0;
    for (int j = L.length; j >= i; j--)
        L.data[j] = L.data[j - 1];
    L.data[i - 1] = e;
    L.length++;
    return 1;
}

//打印顺序表
void PrintList(SqList L) {
    for (int i = 0; i < L.length; i++)
        cout << L.data[i] << " ";
    cout << endl;
}

//取第i个元素
int GetElem(SqList L, int i, ElemType& e) {
    if (i<1 || i>L.length) return 0;
    e = L.data[i - 1];
    return 1;
}

//查找元素位置
int LocateElem(SqList L, ElemType e) {
    for (int i = 0; i < L.length; i++)
        if (L.data[i] == e) return i + 1;
    return 0;
}

//删除第i个元素
int ListDelete(SqList& L, int i, ElemType& e) {
    if (i<1 || i>L.length) return 0;
    e = L.data[i - 1];
    for (int j = i; j < L.length; j++)
        L.data[j - 1] = L.data[j];
    L.length--;
    return 1;
}

int main() {
    SqList L;
    char e;
    //(1)初始化
    InitList(L);
    //(2)插入a,b,c,d,e
    ListInsert(L, 1, 'a');
    ListInsert(L, 2, 'b');
    ListInsert(L, 3, 'c');
    ListInsert(L, 4, 'd');
    ListInsert(L, 5, 'e');

    //(3)输出表
    cout << "顺序表："; PrintList(L);
    //(4)长度
    cout << "长度：" << L.length << endl;
    //(5)判空
    if (L.length == 0) cout << "表为空" << endl;
    else cout << "表不为空" << endl;
    //(6)第3个元素
    GetElem(L, 3, e);
    cout << "第3个元素：" << e << endl;
    //(7)a的位置
    cout << "a的位置：" << LocateElem(L, 'a') << endl;
    //(8)第4位插入f
    ListInsert(L, 4, 'f');
    //(9)输出
    cout << "插入f后："; PrintList(L);
    //(10)删除第3个
    ListDelete(L, 3, e);
    //(11)输出
    cout << "删除第3个后："; PrintList(L);
    //(12)释放
    L.length = 0;
    cout << "释放顺序表" << endl;
    return 0;
}