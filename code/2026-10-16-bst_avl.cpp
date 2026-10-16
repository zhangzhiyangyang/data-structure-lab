/*
 * 二叉搜索树（BST）与平衡二叉树（AVL）核心操作的完整实现
 * ------------------------------------------------------------------
 * 配套笔记：notes/2026-10-16-二叉搜索树与平衡二叉树.md
 * 编译：g++ -std=c++17 -Wall -Wextra -o bst_avl code/2026-10-16-bst_avl.cpp
 * 运行：./bst_avl
 *
 * 内容：
 *   一、二叉搜索树：插入、查找、删除（三种情况）、极值、求高/结点数/叶子数、
 *       中序/层次遍历、判合法性、第 k 小
 *   二、平衡二叉树：四种旋转（LL/RR/LR/RL）、插入与删除后的再平衡、
 *       平衡因子与高度域校验
 *   三、练习答案：判 BST、第 k 小、删除、判平衡、AVL 校验
 */

#include <algorithm>
#include <climits>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

// ==================================================================
// 一、二叉搜索树（Binary Search Tree）
// ==================================================================

// 链式存储结构：数据域 + 左右孩子指针
struct BSTNode {
    int data;          // 数据域
    BSTNode* left;     // 左孩子
    BSTNode* right;    // 右孩子
    explicit BSTNode(int v) : data(v), left(nullptr), right(nullptr) {}
};

// 练习 1 的前置声明：判断一棵二叉树是否为二叉搜索树
bool isBST(const BSTNode* t, long long lo, long long hi);

// 练习 4：判断一棵二叉树是否为平衡二叉树
int checkBalanced(const BSTNode* t, bool& ok);
bool isBalancedTree(const BSTNode* t);
// 练习 2：求 BST 中第 k 小的元素（k 从 1 开始，k 越界返回 -1）
int kthSmallest(const BSTNode* root, int k);

class BST {
public:
    BST() : root_(nullptr) {}
    ~BST() { destroy(root_); }

    // 递归删除整棵树，避免浅拷贝造成的二次释放
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    BSTNode* root() const { return root_; }

    // ------- 核心操作 -------
    void insert(int v) { root_ = insert(root_, v); }   // 插入
    bool search(int v) const { return search(root_, v); }  // 查找
    void remove(int v) { root_ = remove(root_, v); }   // 删除

    // ------- 极值：最左下 / 最右下结点 -------
    int minValue() const { return root_ ? minNode(root_)->data : -1; }
    int maxValue() const { return root_ ? maxNode(root_)->data : -1; }

    // ------- 统计量 -------
    int height() const { return height(root_); }
    int nodeCount() const { return nodeCount(root_); }
    int leafCount() const { return leafCount(root_); }

    // ------- 合法性校验 -------
    bool valid() const { return isBST(root_, LLONG_MIN, LLONG_MAX); }

    // ------- 遍历 -------
    std::vector<int> inorder() const {
        std::vector<int> out;
        inorder(root_, out);
        return out;   // BST 的中序序列必然升序
    }

    std::vector<int> levelOrder() const {
        std::vector<int> out;
        if (!root_) return out;
        std::queue<BSTNode*> q;
        q.push(root_);
        while (!q.empty()) {
            BSTNode* n = q.front();
            q.pop();
            out.push_back(n->data);
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }
        return out;
    }

private:
    BSTNode* root_;

    static void destroy(BSTNode* t) {
        if (!t) return;
        destroy(t->left);
        destroy(t->right);
        delete t;
    }

    // 插入：比当前结点小往左走，大往右走，相等则不重复插入
    static BSTNode* insert(BSTNode* t, int v) {
        if (!t) return new BSTNode(v);
        if (v < t->data)      t->left = insert(t->left, v);
        else if (v > t->data) t->right = insert(t->right, v);
        // v == t->data：已存在，直接返回（保证元素唯一）
        return t;
    }

    // 查找：平均能砍掉一半子树，退化成链时退化为顺序查找
    static bool search(const BSTNode* t, int v) {
        while (t) {
            if (v == t->data) return true;
            t = (v < t->data) ? t->left : t->right;
        }
        return false;
    }

    // 删除：分三种情况
    //   1) 叶子        -> 直接摘除
    //   2) 只有一个孩子 -> 用孩子顶替自己
    //   3) 两个孩子    -> 用中序后继（右子树最小结点）覆盖，再递归删除该后继
    static BSTNode* remove(BSTNode* t, int v) {
        if (!t) return nullptr;
        if (v < t->data) {
            t->left = remove(t->left, v);
        } else if (v > t->data) {
            t->right = remove(t->right, v);
        } else {
            if (!t->left || !t->right) {                 // 情况 1、2
                BSTNode* child = t->left ? t->left : t->right;
                delete t;
                return child;
            }
            BSTNode* s = minNode(t->right);              // 情况 3
            t->data = s->data;
            t->right = remove(t->right, s->data);
        }
        return t;
    }

    static BSTNode* minNode(BSTNode* t) {
        if (!t) return nullptr;
        while (t->left) t = t->left;   // 一路向左
        return t;
    }

    static BSTNode* maxNode(BSTNode* t) {
        if (!t) return nullptr;
        while (t->right) t = t->right; // 一路向右
        return t;
    }

    static int height(const BSTNode* t) {
        if (!t) return 0;
        return std::max(height(t->left), height(t->right)) + 1;
    }

    static int nodeCount(const BSTNode* t) {
        if (!t) return 0;
        return 1 + nodeCount(t->left) + nodeCount(t->right);
    }

    static int leafCount(const BSTNode* t) {
        if (!t) return 0;
        if (!t->left && !t->right) return 1;
        return leafCount(t->left) + leafCount(t->right);
    }

    static void inorder(const BSTNode* t, std::vector<int>& out) {
        if (!t) return;
        inorder(t->left, out);
        out.push_back(t->data);
        inorder(t->right, out);
    }
};

// ==================================================================
// 二、平衡二叉树（AVL 树）
// ==================================================================

// 结点比 BST 多存一个「以自己为根的子树高度」，用于 O(1) 取平衡因子
struct AVLNode {
    int key;           // 关键字
    int height;        // 子树高度（叶子为 1，空树为 0）
    AVLNode* left;
    AVLNode* right;
    explicit AVLNode(int k) : key(k), height(1), left(nullptr), right(nullptr) {}
};

// 练习 5 的校验函数
bool isAVLTree(const AVLNode* t);

class AVL {
public:
    AVL() : root_(nullptr), rotations_(0) {}
    ~AVL() { destroy(root_); }
    AVL(const AVL&) = delete;
    AVL& operator=(const AVL&) = delete;

    AVLNode* root() const { return root_; }

    void insert(int k) { root_ = insert(root_, k); }
    void remove(int k) { root_ = remove(root_, k); }

    bool search(int k) const {
        AVLNode* p = root_;
        while (p) {
            if (k == p->key) return true;
            p = (k < p->key) ? p->left : p->right;
        }
        return false;
    }

    int height() const { return h(root_); }
    int rotations() const { return rotations_; }     // 累计旋转次数
    bool balanced() const { return balanced(root_); }

    std::vector<int> inorder() const {
        std::vector<int> out;
        inorder(root_, out);
        return out;
    }

    std::vector<int> levelOrder() const {
        std::vector<int> out;
        if (!root_) return out;
        std::queue<AVLNode*> q;
        q.push(root_);
        while (!q.empty()) {
            AVLNode* n = q.front();
            q.pop();
            out.push_back(n->key);
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }
        return out;
    }

    // 按层次序输出每个结点的平衡因子，形如 "30(0) 20(0) 40(-1)"
    std::string factors() const {
        std::string s;
        if (!root_) return s;
        std::queue<AVLNode*> q;
        q.push(root_);
        while (!q.empty()) {
            AVLNode* n = q.front();
            q.pop();
            s += std::to_string(n->key) + "(" + std::to_string(bf(n)) + ") ";
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }
        if (!s.empty()) s.pop_back();   // 去掉末尾空格
        return s;
    }

private:
    AVLNode* root_;
    int rotations_;   // 旋转计数，用于观察失衡调整的代价

    static int h(const AVLNode* t) { return t ? t->height : 0; }
    static void update(AVLNode* t) {
        t->height = std::max(h(t->left), h(t->right)) + 1;
    }
    // 平衡因子 BF = 左子树高 - 右子树高
    static int bf(const AVLNode* t) {
        return t ? h(t->left) - h(t->right) : 0;
    }

    static void destroy(AVLNode* t) {
        if (!t) return;
        destroy(t->left);
        destroy(t->right);
        delete t;
    }

    // 右旋：适用于 LL 型（新结点插在左子树的左孩子上）
    // 旋转前：y 为根，y 的左孩子是 x，x 的左右孩子是 T1、T2，y 的右孩子是 T3
    // 旋转后：x 上升为根；x 的左孩子仍为 T1，x 的右孩子改为 y；
    //         y 的左孩子改为 T2（即 x 原来的右孩子），y 的右孩子仍为 T3
    // 效果：原来偏高的左边被「提」上来，T1 与 T3 的高度差缩小；中序序列不变
    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* t2 = x->right;
        x->right = y;
        y->left = t2;
        update(y);   // 先更新下面的 y
        update(x);   // 再更新上面的 x
        ++rotations_;
        return x;
    }

    // 左旋：适用于 RR 型（新结点插在右子树的右孩子上），是右旋的镜像
    // 旋转前：x 为根，x 的右孩子是 y，y 的左右孩子是 T2、T3，x 的左孩子是 T1
    // 旋转后：y 上升为根；y 的右孩子仍为 T3，y 的左孩子改为 x；
    //         x 的右孩子改为 T2（即 y 原来的左孩子），x 的左孩子仍为 T1
    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* t2 = y->left;
        y->left = x;
        x->right = t2;
        update(x);
        update(y);
        ++rotations_;
        return y;
    }

    // 统一再平衡入口：先刷新高度，再按 BF 与孩子 BF 判断四种失衡型
    AVLNode* rebalance(AVLNode* t) {
        update(t);
        int balance = bf(t);
        if (balance > 1) {                    // 左边高
            if (bf(t->left) < 0) t->left = rotateLeft(t->left);   // LR -> 先左旋左孩子
            return rotateRight(t);                                 // LL / 处理后的 LR
        }
        if (balance < -1) {                   // 右边高
            if (bf(t->right) > 0) t->right = rotateRight(t->right); // RL -> 先右旋右孩子
            return rotateLeft(t);                                   // RR / 处理后的 RL
        }
        return t;
    }

    AVLNode* insert(AVLNode* t, int k) {
        if (!t) return new AVLNode(k);
        if (k < t->key)      t->left = insert(t->left, k);
        else if (k > t->key) t->right = insert(t->right, k);
        else return t;                        // 重复关键字不插入
        return rebalance(t);                   // 回溯路上逐层再平衡
    }

    AVLNode* remove(AVLNode* t, int k) {
        if (!t) return nullptr;
        if (k < t->key) {
            t->left = remove(t->left, k);
        } else if (k > t->key) {
            t->right = remove(t->right, k);
        } else {
            if (!t->left || !t->right) {      // 叶子或单孩子：直接顶替
                AVLNode* child = t->left ? t->left : t->right;
                delete t;
                return child;
            }
            AVLNode* s = minNode(t->right);   // 两个孩子：中序后继顶替
            t->key = s->key;
            t->right = remove(t->right, s->key);
        }
        return rebalance(t);                   // 删除同样可能引起失衡
    }

    static AVLNode* minNode(AVLNode* t) {
        while (t && t->left) t = t->left;
        return t;
    }

    static int balanced(const AVLNode* t) {
        if (!t) return 1;
        int diff = h(t->left) - h(t->right);
        if (diff > 1 || diff < -1) return 0;
        return balanced(t->left) && balanced(t->right);
    }

    static void inorder(const AVLNode* t, std::vector<int>& out) {
        if (!t) return;
        inorder(t->left, out);
        out.push_back(t->key);
        inorder(t->right, out);
    }
};

// ==================================================================
// 三、练习答案
// ==================================================================

// 练习 1：判断一棵二叉树是否为 BST
// 关键：不能只比较「父子」，必须维护整条路径上的上下界
//       lo = 从祖先传下来的下界，hi = 上界；用 long long 防止 INT_MIN/MAX 溢出
bool isBST(const BSTNode* t, long long lo, long long hi) {
    if (!t) return true;
    if (t->data <= lo || t->data >= hi) return false;
    return isBST(t->left, lo, t->data) && isBST(t->right, t->data, hi);
}

// 练习 2：求 BST 中第 k 小的元素（中序遍历的第 k 个结点）
// 用显式栈做中序，不额外保存整个序列，找到即返回
int kthSmallest(const BSTNode* root, int k) {
    std::vector<const BSTNode*> st;
    const BSTNode* p = root;
    while (p || !st.empty()) {
        while (p) {                 // 一路向左压栈
            st.push_back(p);
            p = p->left;
        }
        p = st.back();              // 弹出即访问
        st.pop_back();
        if (--k == 0) return p->data;
        p = p->right;               // 转向右子树
    }
    return -1;                      // k 越界
}

// 练习 4：判断一棵二叉树是否为平衡二叉树（每个结点左右子树高度差 ≤ 1）
int checkBalanced(const BSTNode* t, bool& ok) {
    if (!t) return 0;
    int lh = checkBalanced(t->left, ok);
    int rh = checkBalanced(t->right, ok);
    int diff = (lh > rh) ? (lh - rh) : (rh - lh);
    if (diff > 1) ok = false;
    return (lh > rh ? lh : rh) + 1;
}
bool isBalancedTree(const BSTNode* t) {
    bool ok = true;
    checkBalanced(t, ok);
    return ok;
}

// 练习 5：校验 AVL 树——既要 |BF| ≤ 1，也要每个结点存的 height 域正确
bool isAVLTree(const AVLNode* t) {
    if (!t) return true;
    int lh = t->left ? t->left->height : 0;
    int rh = t->right ? t->right->height : 0;
    if (t->height != (lh > rh ? lh : rh) + 1) return false;   // 高度域算错
    int diff = lh - rh;
    if (diff > 1 || diff < -1) return false;                  // 失去平衡
    return isAVLTree(t->left) && isAVLTree(t->right);
}

// ==================================================================
// 四、测试主函数
// ==================================================================

static void printVec(const std::string& label, const std::vector<int>& v) {
    std::cout << label;
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i] << (i + 1 == v.size() ? "" : " ");
    }
    std::cout << "\n";
}

int main() {
    std::cout << "===== 一、二叉搜索树（BST） =====\n";
    BST bst;
    for (int v : {50, 30, 70, 20, 40, 60, 80}) bst.insert(v);

    printVec("中序遍历（应升序）：", bst.inorder());
    printVec("层次遍历：", bst.levelOrder());
    std::cout << "结点数=" << bst.nodeCount()
              << "  叶子数=" << bst.leafCount()
              << "  树高=" << bst.height() << "\n";
    std::cout << "最小值=" << bst.minValue() << "  最大值=" << bst.maxValue() << "\n";
    std::cout << "查找 40：" << (bst.search(40) ? "找到" : "未找到") << "\n";
    std::cout << "查找 45：" << (bst.search(45) ? "找到" : "未找到") << "\n";
    std::cout << "第 3 小的元素=" << kthSmallest(bst.root(), 3) << "\n";
    std::cout << "是否为 BST（练习 1）：" << (bst.valid() ? "是" : "否") << "\n";
    std::cout << "是否平衡（练习 4）：" << (isBalancedTree(bst.root()) ? "是" : "否") << "\n";

    std::cout << "\n-- 删除操作（练习 3）--\n";
    bst.remove(20);                                   // 情况 1：叶子
    printVec("删除 20（叶子）：", bst.inorder());
    bst.remove(70);                                   // 情况 2：单孩子
    printVec("删除 70（单孩子）：", bst.inorder());
    bst.remove(30);                                   // 情况 3：两个孩子
    printVec("删除 30（两个孩子）：", bst.inorder());
    std::cout << "删除后仍为 BST：" << (bst.valid() ? "是" : "否")
              << "  结点数=" << bst.nodeCount() << "\n";

    // 手工构造一棵违反 BST 性质的树，验证 isBST 能识破
    BSTNode* bad = new BSTNode(10);
    bad->left = new BSTNode(20);                      // 左孩子比根大 -> 非法
    std::cout << "手工非法树（10 的左孩子为 20）判为 BST："
              << (isBST(bad, LLONG_MIN, LLONG_MAX) ? "是" : "否") << "\n";
    bad->left->data = 5;                              // 改成合法
    std::cout << "改为 5 后判为 BST："
              << (isBST(bad, LLONG_MIN, LLONG_MAX) ? "是" : "否") << "\n";
    delete bad->left;
    delete bad;

    std::cout << "\n===== 二、平衡二叉树（AVL） =====\n";
    AVL avl;
    for (int k : {30, 20, 10}) avl.insert(k);         // 触发 LL 右旋
    std::cout << "依次插入 30 20 10（LL 型）：\n";
    printVec("  中序：", avl.inorder());
    printVec("  层次：", avl.levelOrder());
    std::cout << "  平衡因子：" << avl.factors() << "\n";
    std::cout << "  树高=" << avl.height()
              << "  旋转次数=" << avl.rotations() << "\n";

    for (int k : {40, 50}) avl.insert(k);             // 触发 RR 左旋
    std::cout << "继续插入 40 50（RR 型）：\n";
    printVec("  中序：", avl.inorder());
    printVec("  层次：", avl.levelOrder());
    std::cout << "  平衡因子：" << avl.factors() << "\n";
    std::cout << "  树高=" << avl.height()
              << "  旋转次数=" << avl.rotations() << "\n";

    avl.insert(25);                                   // 触发 RL 双旋
    std::cout << "插入 25（RL 型，先右旋再左旋）：\n";
    printVec("  中序：", avl.inorder());
    printVec("  层次：", avl.levelOrder());
    std::cout << "  平衡因子：" << avl.factors() << "\n";
    std::cout << "  树高=" << avl.height()
              << "  旋转次数=" << avl.rotations() << "\n";

    avl.remove(50);                                   // 删除叶子，仍平衡
    avl.remove(40);                                   // 删除后失衡，右旋修正
    std::cout << "删除 50、40（40 被删后触发右旋）：\n";
    printVec("  中序：", avl.inorder());
    printVec("  层次：", avl.levelOrder());
    std::cout << "  平衡因子：" << avl.factors() << "\n";
    std::cout << "  树高=" << avl.height()
              << "  旋转次数=" << avl.rotations() << "\n";

    std::cout << "AVL 校验（练习 5：结构 + 高度域）："
              << (isAVLTree(avl.root()) ? "通过" : "失败") << "\n";
    std::cout << "查找 25：" << (avl.search(25) ? "找到" : "未找到")
              << "  查找 99：" << (avl.search(99) ? "找到" : "未找到") << "\n";

    return 0;
}

/* ==================== 预期输出 ====================

===== 一、二叉搜索树（BST） =====
中序遍历（应升序）：20 30 40 50 60 70 80
层次遍历：50 30 70 20 40 60 80
结点数=7  叶子数=4  树高=3
最小值=20  最大值=80
查找 40：找到
查找 45：未找到
第 3 小的元素=40
是否为 BST（练习 1）：是
是否平衡（练习 4）：是

-- 删除操作（练习 3）--
删除 20（叶子）：30 40 50 60 70 80
删除 70（单孩子）：30 40 50 60 80
删除 30（两个孩子）：40 50 60 80
删除后仍为 BST：是  结点数=4
手工非法树（10 的左孩子为 20）判为 BST：否
改为 5 后判为 BST：是

===== 二、平衡二叉树（AVL） =====
依次插入 30 20 10（LL 型）：
  中序：10 20 30
  层次：20 10 30
  平衡因子：20(0) 10(0) 30(0)
  树高=2  旋转次数=1
继续插入 40 50（RR 型）：
  中序：10 20 30 40 50
  层次：20 10 40 30 50
  平衡因子：20(-1) 10(0) 40(0) 30(0) 50(0)
  树高=3  旋转次数=2
插入 25（RL 型，先右旋再左旋）：
  中序：10 20 25 30 40 50
  层次：30 20 40 10 25 50
  平衡因子：30(0) 20(0) 40(-1) 10(0) 25(0) 50(0)
  树高=3  旋转次数=4
删除 50、40（40 被删后触发右旋）：
  中序：10 20 25 30
  层次：20 10 30 25
  平衡因子：20(-1) 10(0) 30(1) 25(0)
  树高=3  旋转次数=5
AVL 校验（练习 5：结构 + 高度域）：通过
查找 25：找到  查找 99：未找到

==================== 预期输出结束 ==================== */
