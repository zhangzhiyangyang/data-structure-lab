// ============================================================
// 二叉树的线索化（中序线索二叉树）—— 课程第 11 期配套代码
// 功能：
//   1. 建一棵示例二叉树（先序 A B D G E H I C F，中序 G D B A H E I C F）
//   2. 中序线索化（把空指针改写为前驱/后继线索）
//   3. 非递归、O(1) 空间的中序遍历（沿后继线索走）
//   4. 逆中序遍历（沿前驱线索走）
//   5. 找任意结点的中序后继（练习题 3）
//   6. 对比普通递归中序遍历，验证结果一致
// 编译：g++ -std=c++17 -Wall -Wextra -o threaded.exe threaded_binary_tree.cpp
// ============================================================
#include <iostream>
#include <string>

typedef char ElemType;

// ---------- 结点定义 ----------
// 在普通二叉链表上加了两个标志位：
//   ltag = 0：lchild 存的是左孩子（Link）
//   ltag = 1：lchild 存的是前驱线索（Thread），指向中序序列的前一个结点
//   rtag / rchild 同理，rtag = 1 时指向中序序列的后一个结点
struct ThreadNode {
    ElemType data;       // 数据域
    ThreadNode *lchild;  // 左孩子 或 前驱线索（由 ltag 决定含义）
    ThreadNode *rchild;  // 右孩子 或 后继线索（由 rtag 决定含义）
    int ltag;            // 0 = Link 孩子指针；1 = Thread 前驱线索
    int rtag;            // 0 = Link 孩子指针；1 = Thread 后继线索
};

const int LINK = 0;    // 指针域存放孩子指针
const int THREAD = 1;  // 指针域存放线索

// ---------- 根据先序带空标记 '#' 建树 ----------
// 输入 "#BD..." 这样的字符串，'#' 表示空树。
// 例如示例树：A B D G # # # # C E H # # I # # F # #
ThreadNode* build(const std::string& s, int& pos) {
    if (pos >= (int)s.size() || s[pos] == '#') {  // 空结点
        pos++;
        return nullptr;
    }
    ThreadNode* node = new ThreadNode{s[pos], nullptr, nullptr, LINK, LINK};
    pos++;
    node->lchild = build(s, pos);  // 先递归建左子树
    node->rchild = build(s, pos);  // 再递归建右子树
    return node;
}

// ---------- 中序线索化 ----------
// pre 是"上一个被访问（中序意义下）的结点"，用指针引用传进来，
// 这样递归返回后上一层能拿到最新值。
void inThreading(ThreadNode* node, ThreadNode*& pre) {
    if (node == nullptr) return;

    inThreading(node->lchild, pre);  // (1) 先线索化左子树

    // (2) 处理当前结点的左指针：若左孩子为空，放前驱线索
    if (node->lchild == nullptr) {
        node->lchild = pre;   // 前驱线索指向中序前一个结点（第一个结点为 null）
        node->ltag = THREAD;
    }
    // (3) 处理"上一个结点"的右指针：若它的右孩子为空，放后继线索指向当前结点
    if (pre != nullptr && pre->rchild == nullptr) {
        pre->rchild = node;   // 后继线索：pre 的中序后继就是当前结点
        pre->rtag = THREAD;
    }
    pre = node;               // (4) 当前结点成为下一轮的"前驱"

    inThreading(node->rchild, pre);  // (5) 再线索化右子树
    // 注意：这里递归右子树前不需要判 rtag，因为中序线索化时
    // rtag 在进入递归时一定还是 LINK（改写发生在"上一层处理"阶段）。
    // 但先序线索化就必须判 rtag == 0，见笔记易错点 6。
}

// 封装：线索化入口。结束后 pre 指向中序最后一个结点，它的 rchild 保持 null。
void createInThread(ThreadNode* root) {
    ThreadNode* pre = nullptr;        // 初始前驱为空
    if (root != nullptr) {
        inThreading(root, pre);
        pre->rchild = nullptr;        // 最后一个结点无后继
        pre->rtag = THREAD;
    }
}

// ---------- 找中序序列的第一个结点 ----------
// 从根一路往左走，直到遇到前驱线索（ltag == THREAD）为止
ThreadNode* firstNode(ThreadNode* p) {
    while (p != nullptr && p->ltag == LINK) p = p->lchild;
    return p;
}

// ---------- 找结点 p 的中序后继（练习题 3 的核心） ----------
ThreadNode* nextNode(ThreadNode* p) {
    if (p->rtag == THREAD)      // 情况 1：右指针本身就是后继线索，O(1) 直达
        return p->rchild;
    // 情况 2：有右孩子，后继 = 右子树中最左的结点（最多走树高 h 步）
    return firstNode(p->rchild);
}

// ---------- 找结点 p 的中序前驱（对称操作） ----------
ThreadNode* prevNode(ThreadNode* p) {
    if (p->ltag == THREAD)      // 情况 1：左指针就是前驱线索
        return p->lchild;
    // 情况 2：有左孩子，前驱 = 左子树中最右的结点
    p = p->lchild;
    while (p != nullptr && p->rtag == LINK) p = p->rchild;
    return p;
}

// ---------- 非递归中序遍历（沿后继线索走，空间 O(1)） ----------
void inorderThreaded(ThreadNode* root) {
    std::cout << "中序线索遍历: ";
    for (ThreadNode* p = firstNode(root); p != nullptr; p = nextNode(p)) {
        std::cout << p->data << ' ';
    }
    std::cout << '\n';
}

// ---------- 逆中序遍历（沿前驱线索走，练习题 4） ----------
void inorderReversed(ThreadNode* root) {
    std::cout << "逆中序遍历  : ";
    // 先找中序序列最后一个结点：从根一路往右走，直到遇到后继线索
    ThreadNode* p = root;
    while (p != nullptr && p->rtag == LINK) p = p->rchild;
    // 反复找前驱
    for (; p != nullptr; p = prevNode(p)) {
        std::cout << p->data << ' ';
    }
    std::cout << '\n';
}

// ---------- 普通递归中序遍历（只能在线索化之前用，用于对比） ----------
void inorderPlain(ThreadNode* node, std::string& out) {
    if (node == nullptr) return;
    inorderPlain(node->lchild, out);
    out += node->data;
    inorderPlain(node->rchild, out);
}

// ---------- 找树中值为 x 的结点（演示 nextNode 用法，练习题 3） ----------
ThreadNode* findNode(ThreadNode* root, ElemType x) {
    // 线索化后用 O(1) 空间的线索遍历查找
    for (ThreadNode* p = firstNode(root); p != nullptr; p = nextNode(p)) {
        if (p->data == x) return p;
    }
    return nullptr;
}

int main() {
    // (1) 建树：先序 A B D G E H I C F，中序 G D B A H E I C F
    //     带空标记：A B D G # # # # C E H # # I # # F # #
    int pos = 0;
    ThreadNode* root = build("ABDG####CEH##I##F##", pos);
    std::cout << "(1) 树已建立（先序 A B D G E H I C F）\n";

    // (2) 线索化之前的普通递归中序遍历，留作对比基准
    std::string plain;
    inorderPlain(root, plain);
    std::cout << "(2) 普通递归中序: " << plain << '\n';

    // (3) 执行中序线索化
    createInThread(root);
    std::cout << "(3) 中序线索化完成\n";

    // (4) 线索化之后的非递归中序遍历，应与 (2) 完全一致
    inorderThreaded(root);

    // (5) 逆中序遍历，应为 (2) 的倒序
    inorderReversed(root);

    // (6) 演示找结点 B 的中序后继：B 的右孩子 G 为空？B 有右孩子吗？
    //     本树中 B 的右孩子为空，所以 rtag=THREAD，后继 O(1) 直达 = A
    ThreadNode* b = findNode(root, 'B');
    ThreadNode* succ = nextNode(b);
    std::cout << "(6) B 的中序后继 = " << (succ ? std::string(1, succ->data) : "无") << '\n';

    // (7) 演示找结点 A 的中序后继：A 有右孩子 C，需找 C 子树最左结点 = H
    ThreadNode* a = findNode(root, 'A');
    ThreadNode* succ2 = nextNode(a);
    std::cout << "(7) A 的中序后继 = " << (succ2 ? std::string(1, succ2->data) : "无") << '\n';

    // (8) 练习题 5 简答（运行现象印证）：
    //     (a) 错。rtag=0 的结点找后继要下探右子树最左结点，最坏 O(h)。
    //     (b) 错。两种结构都是每结点 2 个指针 + 线索树多 2 个 tag，O(n) 相同。
    //     (c) 对。后序线索树中找后继需借助双亲，纯二叉链表必须加双亲指针。
    std::cout << "(8) 判断题结论：a=错 b=错 c=对（理由见笔记第 7 节）\n";

    return 0;
}

/* ======================= 预期输出 =======================
(1) 树已建立（先序 A B D G E H I C F）
(2) 普通递归中序: GDBAHEICF
(3) 中序线索化完成
中序线索遍历: G D B A H E I C F
逆中序遍历  : F C I E H A B D G
(6) B 的中序后继 = A
(7) A 的中序后继 = H
(8) 判断题结论：a=错 b=错 c=对（理由见笔记第 7 节）
======================================================= */
