// 二叉树的基本操作与遍历
// 配套笔记：notes/2026-10-15-树与二叉树.md
// 编译：g++ -std=c++17 -o binary_tree binary_tree.cpp
//
// 覆盖内容：
//   1. 二叉链表的结构定义与建树
//   2. 先序 / 中序 / 后序遍历（递归版 + 非递归版）
//   3. 层次遍历（队列实现）
//   4. 求结点总数、深度、叶子结点数
//   5. 交换所有结点的左右子树
//   6. 判断是否为完全二叉树

#include <iostream>
#include <stack>
#include <queue>

using namespace std;

typedef char ElemType;

// ---------- 1. 二叉链表结构 ----------
struct BiTNode {
    ElemType data;
    BiTNode *lchild, *rchild;
    explicit BiTNode(ElemType e) : data(e), lchild(nullptr), rchild(nullptr) {}
};
typedef BiTNode *BiTree;

// 新建结点
BiTree NewNode(ElemType e) {
    return new BiTNode(e);
}

// ---------- 2. 递归遍历 ----------
void PreOrder(const BiTree &T) {
    if (!T) return;                 // 空树直接返回（易错点：别写成 if (T) return）
    cout << T->data << ' ';
    PreOrder(T->lchild);
    PreOrder(T->rchild);
}

void InOrder(const BiTree &T) {
    if (!T) return;
    InOrder(T->lchild);
    cout << T->data << ' ';
    InOrder(T->rchild);
}

void PostOrder(const BiTree &T) {
    if (!T) return;
    PostOrder(T->lchild);
    PostOrder(T->rchild);
    cout << T->data << ' ';
}

// ---------- 3. 非递归遍历（显式用栈模拟递归）----------
// 先序：访问结点后先压右孩子再压左孩子，保证左孩子先出栈
void PreOrderNoRec(BiTree T) {
    if (!T) return;
    stack<BiTree> st;
    st.push(T);
    while (!st.empty()) {
        BiTree p = st.top();
        st.pop();
        cout << p->data << ' ';
        if (p->rchild) st.push(p->rchild);
        if (p->lchild) st.push(p->lchild);
    }
}

// 中序：一路向左入栈，无法向左时出栈访问，再转向右子树
void InOrderNoRec(BiTree T) {
    stack<BiTree> st;
    BiTree p = T;
    while (p || !st.empty()) {
        while (p) {          // 走到最左
            st.push(p);
            p = p->lchild;
        }
        p = st.top();
        st.pop();
        cout << p->data << ' ';   // 访问
        p = p->rchild;            // 转向右子树
    }
}

// 后序：用「根-右-左」的先序变体遍历，再整体逆序，即得「左-右-根」
void PostOrderNoRec(BiTree T) {
    if (!T) return;
    stack<BiTree> st;
    stack<ElemType> out;      // 暂存访问序列，最后逆序输出
    st.push(T);
    while (!st.empty()) {
        BiTree p = st.top();
        st.pop();
        out.push(p->data);
        if (p->lchild) st.push(p->lchild);
        if (p->rchild) st.push(p->rchild);
    }
    while (!out.empty()) {
        cout << out.top() << ' ';
        out.pop();
    }
}

// ---------- 4. 层次遍历（队列）----------
void LevelOrder(BiTree T) {
    if (!T) return;
    queue<BiTree> q;
    q.push(T);
    while (!q.empty()) {
        BiTree p = q.front();
        q.pop();
        cout << p->data << ' ';
        if (p->lchild) q.push(p->lchild);
        if (p->rchild) q.push(p->rchild);
    }
}

// ---------- 5. 统计类操作 ----------
// 结点总数
int CountNodes(BiTree T) {
    if (!T) return 0;
    return 1 + CountNodes(T->lchild) + CountNodes(T->rchild);
}

// 树的深度
int Depth(BiTree T) {
    if (!T) return 0;
    int dl = Depth(T->lchild);
    int dr = Depth(T->rchild);
    return (dl > dr ? dl : dr) + 1;
}

// 叶子结点数
int CountLeaves(BiTree T) {
    if (!T) return 0;
    if (!T->lchild && !T->rchild) return 1;
    return CountLeaves(T->lchild) + CountLeaves(T->rchild);
}

// ---------- 6. 交换所有结点的左右子树 ----------
void SwapChildren(BiTree T) {
    if (!T) return;
    BiTree tmp = T->lchild;
    T->lchild = T->rchild;
    T->rchild = tmp;
    SwapChildren(T->lchild);
    SwapChildren(T->rchild);
}

// ---------- 7. 判断是否为完全二叉树 ----------
// 思路：层次遍历，一旦遇到空结点，其后就不应再出现非空结点
bool IsComplete(BiTree T) {
    if (!T) return true;
    queue<BiTree> q;
    q.push(T);
    bool seenEmpty = false;           // 是否已经遇到过空孩子
    while (!q.empty()) {
        BiTree p = q.front();
        q.pop();
        if (!p) {
            seenEmpty = true;         // 遇到空结点，之后的结点必须全为空
            continue;
        }
        if (seenEmpty) return false;  // 空结点之后又出现非空结点
        q.push(p->lchild);
        q.push(p->rchild);
    }
    return true;
}

// 释放整棵树，避免内存泄漏
void DestroyTree(BiTree &T) {
    if (!T) return;
    DestroyTree(T->lchild);
    DestroyTree(T->rchild);
    delete T;
    T = nullptr;
}

// ---------- main：构造测试树并验证 ----------
//         A
//       ╱   ╲
//      B     C
//     ╱ ╲     ╲
//    D   E     F
BiTree BuildTestTree() {
    BiTree A = NewNode('A');
    BiTree B = NewNode('B');
    BiTree C = NewNode('C');
    BiTree D = NewNode('D');
    BiTree E = NewNode('E');
    BiTree F = NewNode('F');

    A->lchild = B;  A->rchild = C;
    B->lchild = D;  B->rchild = E;
    C->rchild = F;
    return A;
}

int main() {
    BiTree T = BuildTestTree();

    cout << "递归先序：  "; PreOrder(T);        cout << endl;
    cout << "递归中序：  "; InOrder(T);         cout << endl;
    cout << "递归后序：  "; PostOrder(T);       cout << endl;
    cout << "非递归先序："; PreOrderNoRec(T);   cout << endl;
    cout << "非递归中序："; InOrderNoRec(T);    cout << endl;
    cout << "非递归后序："; PostOrderNoRec(T);  cout << endl;
    cout << "层次遍历：  "; LevelOrder(T);      cout << endl;

    cout << "结点总数：" << CountNodes(T) << endl;
    cout << "树的深度：" << Depth(T) << endl;
    cout << "叶子结点：" << CountLeaves(T) << endl;
    cout << "是否完全二叉树：" << (IsComplete(T) ? "是" : "否") << endl;

    SwapChildren(T);
    cout << "交换左右子树后先序："; PreOrder(T); cout << endl;

    DestroyTree(T);
    return 0;
}

// 预期输出：
// 递归先序：  A B D E C F
// 递归中序：  D B E A C F
// 递归后序：  D E B F C A
// 非递归先序：A B D E C F
// 非递归中序：D B E A C F
// 非递归后序：D E B F C A
// 层次遍历：  A B C D E F
// 结点总数：6
// 树的深度：3
// 叶子结点：3
// 是否完全二叉树：否
// 交换左右子树后先序：A C F B E D
