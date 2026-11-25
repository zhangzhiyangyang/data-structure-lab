/* ============================================================
 * 2026-11-25 红黑树（概述、插入、删除）
 * 配套笔记：notes/2026-11-25-红黑树.md
 * 编译：g++ -std=c++17 -Wall -Wextra -o rb code/2026-11-25-red_black_tree.cpp
 *
 * 采用 CLRS（《算法导论》第 13 章）经典写法：
 *   - 黑色哨兵 nil 代替所有空指针，插入/删除修复无需特判空树；
 *   - 三叉链表（left/right/parent）方便向上回溯修复。
 * main 中 6 组测试覆盖：插入修复三情形、与普通 BST 的对比、
 * 查找、删除修复、大规模性质校验，以及练习题 1 的参考答案。
 * ============================================================ */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

using namespace std;

/* ---------- 结点定义 ---------- */
enum Color { RED, BLACK };   // 用 int 也可以，枚举更清晰

struct Node {
    int   key;
    Color color;                        // 红 / 黑
    Node *left, *right, *parent;        // 三叉链表
    Node(int k, Color c) : key(k), color(c),
                           left(nullptr), right(nullptr), parent(nullptr) {}
};

/* ============================================================
 * 红黑树类
 * ============================================================ */
class RBTree {
public:
    Node* nil;    // 黑色哨兵：代替所有 NULL（性质 3），恒为黑
    Node* root;   // 根结点，root->parent 也指向 nil

    RBTree() {
        nil = new Node(0, BLACK);                 // 哨兵本身是黑结点
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }

    /* ---------- 旋转：只改指针不改颜色，中序序列不变 ---------- */
    // 左旋 x：x 的右孩子 y 顶上来，x 降为 y 的左孩子
    //     x              y
    //    / \            / ╲
    //   a   y   ==>    x   c
    //      / \        / ╲
    //     b   c      a   b
    void leftRotate(Node* x) {
        Node* y = x->right;          // y 是要"顶上去"的右孩子
        x->right = y->left;          // y 的左子树 b 转给 x 作右子树
        if (y->left != nil) y->left->parent = x;
        y->parent = x->parent;       // 让 x 的父亲认 y 当孩子
        if (x->parent == nil)      root = y;
        else if (x == x->parent->left)  x->parent->left  = y;
        else                            x->parent->right = y;
        y->left = x;                 // x 降为 y 的左孩子
        x->parent = y;
    }

    // 右旋 x：左旋的镜像
    //       y                x
    //      / \              / ╲
    //     x   c    ==>     a   y
    //    / \                  / ╲
    //   a   b                b   c
    void rightRotate(Node* x) {
        Node* y = x->left;           // y 是要"顶上去"的左孩子
        x->left = y->right;          // y 的右子树 b 转给 x 作左子树
        if (y->right != nil) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nil)      root = y;
        else if (x == x->parent->left)  x->parent->left  = y;
        else                            x->parent->right = y;
        y->right = x;                // x 降为 y 的右孩子
        x->parent = y;
    }

    /* ---------- 插入：先按 BST 插入并染红，再向上修复 ---------- */
    void insert(int key) {
        Node* z = new Node(key, RED);            // 新结点染红！
        z->left = z->right = z->parent = nil;

        Node* y = nil;                           // y 记录插入位置的父结点
        Node* x = root;
        while (x != nil) {                       // 标准 BST 下落
            y = x;
            x = (z->key < x->key) ? x->left : x->right;
        }
        z->parent = y;
        if (y == nil)             root = z;      // 空树：新结点即根
        else if (z->key < y->key) y->left = z;
        else                      y->right = z;

        insertFixup(z);                          // 修复可能出现的连续红
    }

    // 插入修复：z 为"刚出问题"的红结点，循环把它向上移
    void insertFixup(Node* z) {
        // 只要父结点也是红（违反性质 4），就继续修复
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                // ----- 父结点是祖父的左孩子（镜像情形见 else 分支）-----
                Node* uncle = z->parent->parent->right;   // 叔结点
                if (uncle->color == RED) {
                    /* 情形 1：叔为红 —— 父叔染黑、祖父染红，z 上移到祖父
                     *        只变色不旋转 */
                    z->parent->color = BLACK;
                    uncle->color     = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        /* 情形 2：叔为黑且 z 是内侧孩子 —— 对父左旋，
                         * 转化为情形 3 的"外侧"结构 */
                        z = z->parent;
                        leftRotate(z);
                    }
                    /* 情形 3：叔为黑且 z 是外侧孩子 —— 父染黑、祖父染红，
                     * 对祖父右旋，一次旋转移除矛盾 */
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                // ----- 镜像：父结点是祖父的右孩子，左右旋互换 -----
                Node* uncle = z->parent->parent->left;
                if (uncle->color == RED) {
                    z->parent->color = BLACK;             // 情形 1（镜像）
                    uncle->color     = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;                    // 情形 2（镜像）
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;             // 情形 3（镜像）
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;     // 情形 1 可能一路把根染红，强制回黑（性质 2）
    }

    /* ---------- 删除 ---------- */
    // 用子树 v 顶替结点 u 的位置
    // 注意：即使 v 是哨兵，也要设置 v->parent（删除修复要用！）
    void transplant(Node* u, Node* v) {
        if (u->parent == nil)          root = v;
        else if (u == u->parent->left) u->parent->left  = v;
        else                           u->parent->right = v;
        v->parent = u->parent;         // 哨兵也要设置，这是最隐蔽的坑
    }

    Node* minimum(Node* x) {           // 子树最小结点（最左）
        while (x->left != nil) x = x->left;
        return x;
    }

    void remove(int key) {
        Node* z = search(key);
        if (z == nil) {                // 不存在则不做任何事
            cout << "删除失败：键 " << key << " 不存在\n";
            return;
        }
        Node* y = z;                   // y：真正被摘除的结点
        Node* x = nullptr;             // x：接替 y 位置的结点
        Color yOrig = y->color;        // 记录被摘除结点的原颜色！

        if (z->left == nil) {          // 左空：直接用右孩子顶替
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {  // 右空：直接用左孩子顶替
            x = z->left;
            transplant(z, z->left);
        } else {                       // 双孩：找后继 y 顶替 z
            y = minimum(z->right);     // 实际摘除的是后继！
            yOrig = y->color;          // 所以记录后继的颜色
            x = y->right;
            if (y->parent == z) {
                x->parent = y;         // x 可能是哨兵，也要挂好父指针
            } else {
                transplant(y, y->right);   // 先把后继从原位摘出
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);          // 后继顶替 z 的位置
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;       // 继承 z 的颜色（结构换壳）
        }
        delete z;
        if (yOrig == BLACK)            // 摘掉的是黑结点 ⇒ 该路径少一个黑
            deleteFixup(x);            // 用"双黑"修复补回来
        // 摘掉的是红结点则黑高不变，无需修复
    }

    // 删除修复：x 位置"欠一个黑色"（双黑），w 为 x 的兄弟
    void deleteFixup(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                // ----- x 是左孩子（镜像见 else 分支）-----
                Node* w = x->parent->right;          // 兄弟
                if (w->color == RED) {
                    /* 情形 1：兄弟为红 —— 兄染黑、父染红，对父左旋
                     * 目的：换出一个"黑兄弟"，转入情形 2/3/4 */
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;            // 新兄弟
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    /* 情形 2：兄弟黑、两个侄子也黑 —— 兄弟染红，
                     * 双黑上移到父（父若原本是红，下一步循环即退出并染黑）*/
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        /* 情形 3：近侄红、远侄黑 —— 对兄弟右旋，
                         * 转化为情形 4 的"远侄红"结构 */
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    /* 情形 4：远侄红 —— 兄染父色、父染黑、远侄染黑，
                     * 对父左旋，一次补上缺失的黑，结束 */
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;                        // 直接结束
                }
            } else {
                // ----- 镜像：x 是右孩子，左右旋与近/远侄互换 -----
                Node* w = x->parent->left;
                if (w->color == RED) {               // 情形 1（镜像）
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;                  // 情形 2（镜像）
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {   // 情形 3（镜像）
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;     // 情形 4（镜像）
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;    // 把"双黑"里多余的那个黑记在 x 头上：直接染黑
    }

    /* ---------- 查找 / 遍历 / 高度 ---------- */
    Node* search(int key) {            // 迭代版 BST 查找
        Node* x = root;
        while (x != nil && key != x->key)
            x = (key < x->key) ? x->left : x->right;
        return x;
    }

    void inorder(vector<int>& out) { inorderRec(root, out); }
    void inorderRec(Node* x, vector<int>& out) {
        if (x == nil) return;
        inorderRec(x->left, out);
        out.push_back(x->key);
        inorderRec(x->right, out);
    }

    // 树高：按"结点个数"计（空树 0，单结点 1），方便和 BST 对比
    int height(Node* x) {
        if (x == nil) return 0;
        return 1 + max(height(x->left), height(x->right));
    }
    int height() { return height(root); }

    /* ---------- 合法性校验：性质 4、5 + BST 有序性 ---------- */
    // 返回子树黑高；违规时输出错误信息并返回 -1
    int validate(Node* x, bool isRoot) {
        if (x == nil) return 1;                     // 哨兵算 1 个黑
        if (isRoot && x->color != BLACK) {
            cout << "[违规] 性质2：根结点不是黑色！\n";
            return -1;
        }
        if (x->color == RED &&                      // 性质 4：无连续红
            (x->left->color == RED || x->right->color == RED)) {
            cout << "[违规] 性质4：结点 " << x->key << " 出现连续红结点！\n";
            return -1;
        }
        int lh = validate(x->left, false);          // 性质 5：黑高一致
        int rh = validate(x->right, false);
        if (lh == -1 || rh == -1) return -1;
        if (lh != rh) {
            cout << "[违规] 性质5：结点 " << x->key
                 << " 左右黑高不等（" << lh << " vs " << rh << "）！\n";
            return -1;
        }
        return lh + (x->color == BLACK ? 1 : 0);
    }
    // 完整校验：性质 2/4/5 + 中序有序；返回 true 表示合法
    bool validate(const string& label) {
        int bh = validate(root, true);
        vector<int> keys;
        inorder(keys);
        bool sorted = is_sorted(keys.begin(), keys.end());
        bool ok = (bh != -1) && sorted;
        cout << "校验[" << label << "] 节点数=" << keys.size()
             << " 黑高=" << bh << " 中序有序=" << (sorted ? "是" : "否")
             << " => " << (ok ? "通过" : "失败") << "\n";
        return ok;
    }

    /* ---------- 横向树形打印（向左转 90 度：右边在上） ---------- */
    void print(stringstream& ss, Node* x, int depth) {
        if (x == nil) return;
        print(ss, x->right, depth + 1);                       // 先印右子树
        ss << string(depth * 6, ' ')
           << x->key << (x->color == RED ? "(R)" : "(B)") << "\n";
        print(ss, x->left, depth + 1);                        // 再印左子树
    }
    string pretty() {
        stringstream ss;
        print(ss, root, 0);
        return ss.str();
    }
};

/* ============================================================
 * 普通 BST（无平衡），只用于 (2) 中和红黑树对比树高
 * ============================================================ */
struct BSTNode { int key; BSTNode *l = nullptr, *r = nullptr; };
class PlainBST {
    BSTNode* root = nullptr;
    BSTNode* insertRec(BSTNode* t, int key) {
        if (!t) { t = new BSTNode; t->key = key; return t; }
        if (key < t->key) t->l = insertRec(t->l, key);
        else              t->r = insertRec(t->r, key);
        return t;
    }
    int h(BSTNode* t) { return t ? 1 + max(h(t->l), h(t->r)) : 0; }
public:
    void insert(int key) { root = insertRec(root, key); }
    int  height()        { return h(root); }
};

int main() {
    /* ---------- (1) 基本插入：覆盖插入修复的典型情形 ---------- */
    cout << "===== (1) 依次插入 10 20 30 15 25 5 1 =====\n";
    RBTree t1;
    int a1[] = {10, 20, 30, 15, 25, 5, 1};
    for (int k : a1) t1.insert(k);
    vector<int> out;
    t1.inorder(out);
    cout << "中序遍历（应为升序）:";
    for (int k : out) cout << " " << k;
    cout << "\n树形（缩进越深越靠上层，R=红 B=黑）:\n" << t1.pretty();
    t1.validate("(1) 插入后");

    /* ---------- (2) 顺序插入 1~15：普通 BST 退化 vs 红黑树平衡 ---------- */
    cout << "\n===== (2) 顺序插入 1~15：普通 BST vs 红黑树 =====\n";
    PlainBST bst;
    RBTree t2;
    for (int k = 1; k <= 15; k++) { bst.insert(k); t2.insert(k); }
    int n = 15;
    int bound = (int)floor(2 * log2(n + 1));      // 红黑树高度上界
    cout << "普通 BST 树高: " << bst.height()
         << "（链表退化，最坏 O(n)）\n";
    cout << "红黑树树高  : " << t2.height()
         << " ≤ 上界 2*log2(" << n << "+1) = " << bound << "\n";
    vector<int> out2;
    t2.inorder(out2);
    cout << "红黑树中序  :";
    for (int k : out2) cout << " " << k;
    cout << "\n";
    t2.validate("(2) 顺序插入后");

    /* ---------- (3) 查找 ---------- */
    cout << "\n===== (3) 查找演示 =====\n";
    int probes[] = {13, 99};
    for (int k : probes) {
        Node* p = t2.search(k);
        if (p != t2.nil)
            cout << "查找 " << k << "：找到，颜色为"
                 << (p->color == RED ? "红" : "黑") << "\n";
        else
            cout << "查找 " << k << "：不存在\n";
    }

    /* ---------- (4) 删除：覆盖叶/单孩/双孩三种 BST 情形 ---------- */
    cout << "\n===== (4) 依次删除 1 13 8 4 10 =====\n";
    RBTree t3;
    for (int k = 1; k <= 15; k++) t3.insert(k);
    int dels[] = {1, 13, 8, 4, 10};
    for (int k : dels) {
        t3.remove(k);
        cout << "删除 " << k << " 后 -> ";
        if (!t3.validate("删除" + to_string(k))) return 1;
    }
    vector<int> out3;
    t3.inorder(out3);
    cout << "最终中序:";
    for (int k : out3) cout << " " << k;
    cout << "\n删除不存在的键 99：\n";
    t3.remove(99);
    t3.validate("(4) 删除后");

    /* ---------- (5) 大规模压力测试：1023 个结点的黑高与高度 ---------- */
    cout << "\n===== (5) 压力测试：插入 1~1023 再删除一半 =====\n";
    RBTree t4;
    // 用固定步数的乘法散列打乱插入顺序（结果确定，便于核对预期输出）
    const long long A = 48271, M = 2147483647LL;
    long long seed = 1;
    for (int i = 1; i <= 1023; i++) {
        seed = seed * A % M;
        t4.insert((int)(seed % 2000));
    }
    int bh5 = t4.validate("(5a) 1023 次随机插入后") ? 0 : 1;
    (void)bh5;
    cout << "高度上界 2*log2(1024) = "
         << (int)floor(2 * log2(1024)) << "，实际树高 = "
         << t4.height() << "\n";
    // 再删除前 512 个键值（含中途不存在的键也安全）
    seed = 1;
    for (int i = 1; i <= 512; i++) {
        seed = seed * A % M;
        t4.remove((int)(seed % 2000));
    }
    t4.validate("(5b) 删除 512 次后");

    /* ---------- (6) 练习题 1 参考答案：CLRS 13.3-2 序列 ---------- */
    cout << "\n===== (6) 练习1参考：依次插入 41 38 31 12 19 8 =====\n";
    RBTree t5;
    int a2[] = {41, 38, 31, 12, 19, 8};
    for (int k : a2) t5.insert(k);
    vector<int> out5;
    t5.inorder(out5);
    cout << "中序遍历:";
    for (int k : out5) cout << " " << k;
    cout << "\n树形:\n" << t5.pretty();
    t5.validate("(6) 练习序列插入后");

    cout << "\n全部测试完成。\n";
    return 0;
}

/* ============================================================
 * 预期输出（已按实际运行回填）：
 * ===== (1) 依次插入 10 20 30 15 25 5 1 =====
 * 中序遍历（应为升序）: 1 5 10 15 20 25 30
 * 树形（缩进越深越靠上层，R=红 B=黑）:
 *       30(B)
 *             25(R)
 * 20(B)
 *             15(B)
 *       10(R)
 *             5(B)
 *                   1(R)
 * 校验[(1) 插入后] 节点数=7 黑高=3 中序有序=是 => 通过
 * 
 * ===== (2) 顺序插入 1~15：普通 BST vs 红黑树 =====
 * 普通 BST 树高: 15（链表退化，最坏 O(n)）
 * 红黑树树高  : 6 ≤ 上界 2*log2(15+1) = 8
 * 红黑树中序  : 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
 * 校验[(2) 顺序插入后] 节点数=15 黑高=4 中序有序=是 => 通过
 * 
 * ===== (3) 查找演示 =====
 * 查找 13：找到，颜色为红
 * 查找 99：不存在
 * 
 * ===== (4) 依次删除 1 13 8 4 10 =====
 * 删除 1 后 -> 校验[删除1] 节点数=14 黑高=4 中序有序=是 => 通过
 * 删除 13 后 -> 校验[删除13] 节点数=13 黑高=4 中序有序=是 => 通过
 * 删除 8 后 -> 校验[删除8] 节点数=12 黑高=4 中序有序=是 => 通过
 * 删除 4 后 -> 校验[删除4] 节点数=11 黑高=4 中序有序=是 => 通过
 * 删除 10 后 -> 校验[删除10] 节点数=10 黑高=4 中序有序=是 => 通过
 * 最终中序: 2 3 5 6 7 9 11 12 14 15
 * 删除不存在的键 99：
 * 删除失败：键 99 不存在
 * 校验[(4) 删除后] 节点数=10 黑高=4 中序有序=是 => 通过
 * 
 * ===== (5) 压力测试：插入 1~1023 再删除一半 =====
 * 校验[(5a) 1023 次随机插入后] 节点数=1023 黑高=7 中序有序=是 => 通过
 * 高度上界 2*log2(1024) = 20，实际树高 = 12
 * 校验[(5b) 删除 512 次后] 节点数=511 黑高=7 中序有序=是 => 通过
 * 
 * ===== (6) 练习1参考：依次插入 41 38 31 12 19 8 =====
 * 中序遍历: 8 12 19 31 38 41
 * 树形:
 *       41(B)
 * 38(B)
 *             31(B)
 *       19(R)
 *             12(B)
 *                   8(R)
 * 校验[(6) 练习序列插入后] 节点数=6 黑高=3 中序有序=是 => 通过
 * 
 * 全部测试完成。
 * ============================================================ */
