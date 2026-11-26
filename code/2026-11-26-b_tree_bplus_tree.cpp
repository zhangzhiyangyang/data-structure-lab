// ============================================================================
// 2026-11-26-b_tree_bplus_tree.cpp
// 内容：B 树（最小度 t = 3，即 6 阶，CLRS 写法）的
//         查找 / 插入（预分裂）/ 删除（前驱后继替换、借位、合并）/ 结构校验
//       B+ 树（M = 4 阶）的插入（叶子分裂 + 索引上移）、叶子链范围查询、结构校验
// 编译：g++ -std=c++17 -Wall -Wextra -o ds_check 2026-11-26-b_tree_bplus_tree.cpp
// 说明：每个函数上方都有中文注释，按「结点 -> 操作 -> 校验 -> 打印」顺序阅读。
// ============================================================================
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <climits>

// ============================================================================
// 一、B 树
// ============================================================================
class BTree {
public:
    // 最小度 t=3：每个结点 2~5 个关键字，对应国内教材的 6 阶 B 树（m = 2t）
    static constexpr int T = 3;

    // ---------------- 结点定义 ----------------
    struct Node {
        bool leaf = true;           // 是否叶结点
        std::vector<int> key;       // 关键字，升序；size() 即关键字个数 n
        std::vector<Node*> child;   // 孩子，内部结点满足 size() == n + 1
        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    // ---------------- 校验结果 ----------------
    struct Check {
        int totalKeys = 0;          // 整棵树的关键字总数
        int height = 0;             // 树高（根算第 1 层）
        bool leafSameDepth = true;  // 所有叶子是否同层
        bool sorted = true;         // 中序是否严格递增（同时校验父子区间关系）
        bool keyCountOK = true;     // 关键字个数是否落在 [t-1, 2t-1]（根可少）
        bool childCountOK = true;   // 孩子个数是否等于关键字数 + 1
        bool ok() const {
            return leafSameDepth && sorted && keyCountOK && childCountOK;
        }
    };

private:
    Node* root = nullptr;

    // 递归释放整棵子树
    static void destroy(Node* x) {
        if (!x) return;
        for (Node* c : x->child) destroy(c);
        delete x;
    }

    // 中序遍历收集所有关键字（B 树的中序天然就是升序）
    static void collectKeys(const Node* x, std::vector<int>& out) {
        if (!x) return;
        for (int i = 0; i < (int)x->key.size(); ++i) {
            if (!x->leaf) collectKeys(x->child[i], out);
            out.push_back(x->key[i]);
        }
        if (!x->leaf) collectKeys(x->child[x->key.size()], out);
    }

    // ---------------- 分裂：把已满孩子 x->child[i] 一分为二 ----------------
    // 满结点有 2t-1 个关键字（下标 0..2t-2），中间那个 key[t-1] 上移到父结点，
    // 左边留在原结点（t-1 个），右边放进新结点 z（t-1 个）。
    void splitChild(Node* x, int i) {
        Node* y = x->child[i];                 // 已满的孩子
        Node* z = new Node(y->leaf);           // 新兄弟结点
        int midKey = y->key[T - 1];            // 中间关键字，稍后上移
        z->key.assign(y->key.begin() + T, y->key.end());   // 右半 t-1 个
        y->key.resize(T - 1);                              // 左半 t-1 个
        if (!y->leaf) {                                    // 内部结点还要搬孩子
            z->child.assign(y->child.begin() + T, y->child.end());
            y->child.resize(T);
        }
        x->key.insert(x->key.begin() + i, midKey);         // 中间键插到父结点
        x->child.insert(x->child.begin() + i + 1, z);      // 新孩子插到父结点
    }

    // ---------------- 插入：结点未满时递归下探 ----------------
    void insertNonFull(Node* x, int k) {
        if (x->leaf) {
            // 叶子：直接插到有序位置（块内数组插入，代价 O(m)）
            auto it = std::lower_bound(x->key.begin(), x->key.end(), k);
            x->key.insert(it, k);
            return;
        }
        // 找到应下探的孩子：第一个大于 k 的关键字的下标即孩子下标
        int i = (int)(std::upper_bound(x->key.begin(), x->key.end(), k)
                      - x->key.begin());
        if ((int)x->child[i]->key.size() == 2 * T - 1) {   // 孩子已满，先分裂
            splitChild(x, i);
            if (k > x->key[i]) ++i;                        // 落在右半边则改走新结点
        }
        insertNonFull(x->child[i], k);
    }

    // ---------------- 删除辅助 ----------------
    // 找到第一个 >= k 的关键字下标
    static int findKey(const Node* x, int k) {
        int idx = 0;
        while (idx < (int)x->key.size() && x->key[idx] < k) ++idx;
        return idx;
    }

    // 向兄弟借一个关键字（经父结点旋转）：左兄弟 -> 当前结点
    void borrowFromPrev(Node* x, int idx) {
        Node* child = x->child[idx];
        Node* sibling = x->child[idx - 1];
        child->key.insert(child->key.begin(), x->key[idx - 1]);   // 父分隔键下移
        if (!child->leaf) {
            child->child.insert(child->child.begin(), sibling->child.back());
            sibling->child.pop_back();
        }
        x->key[idx - 1] = sibling->key.back();                    // 兄弟末键上移
        sibling->key.pop_back();
    }

    // 向兄弟借一个关键字（经父结点旋转）：右兄弟 -> 当前结点
    void borrowFromNext(Node* x, int idx) {
        Node* child = x->child[idx];
        Node* sibling = x->child[idx + 1];
        child->key.push_back(x->key[idx]);                        // 父分隔键下移
        if (!child->leaf) {
            child->child.push_back(sibling->child.front());
            sibling->child.erase(sibling->child.begin());
        }
        x->key[idx] = sibling->key.front();                       // 兄弟首键上移
        sibling->key.erase(sibling->key.begin());
    }

    // 合并：child[idx] + 父分隔键 + child[idx+1] 合成一个结点
    void merge(Node* x, int idx) {
        Node* child = x->child[idx];
        Node* sibling = x->child[idx + 1];
        child->key.push_back(x->key[idx]);                        // 分隔键下移
        child->key.insert(child->key.end(), sibling->key.begin(), sibling->key.end());
        if (!child->leaf) {
            child->child.insert(child->child.end(),
                                sibling->child.begin(), sibling->child.end());
        }
        x->key.erase(x->key.begin() + idx);                       // 父结点少一个键
        x->child.erase(x->child.begin() + idx + 1);               // 父结点少一个孩子
        delete sibling;
    }

    // 孩子 child[idx] 欠账时的修补：先借，借不到就合并
    void fill(Node* x, int idx) {
        if (idx > 0 && (int)x->child[idx - 1]->key.size() >= T) {
            borrowFromPrev(x, idx);
        } else if (idx < (int)x->child.size() - 1 &&
                   (int)x->child[idx + 1]->key.size() >= T) {
            borrowFromNext(x, idx);
        } else {
            if (idx < (int)x->child.size() - 1) merge(x, idx);
            else merge(x, idx - 1);                               // 最后一个孩子与左兄弟合并
        }
    }

    // 取子树最大键（前驱）
    static int getPred(const Node* x, int idx) {
        const Node* cur = x->child[idx];
        while (!cur->leaf) cur = cur->child.back();
        return cur->key.back();
    }

    // 取子树最小键（后继）
    static int getSucc(const Node* x, int idx) {
        const Node* cur = x->child[idx + 1];
        while (!cur->leaf) cur = cur->child.front();
        return cur->key.front();
    }

    // 删除内部结点上的 key[idx]：用前驱/后继替换，问题转成在叶子上删
    void removeFromInternal(Node* x, int idx) {
        int k = x->key[idx];
        if ((int)x->child[idx]->key.size() >= T) {          // 左孩子够借，用前驱
            int pred = getPred(x, idx);
            x->key[idx] = pred;
            removeRec(x->child[idx], pred);
        } else if ((int)x->child[idx + 1]->key.size() >= T) { // 右孩子够借，用后继
            int succ = getSucc(x, idx);
            x->key[idx] = succ;
            removeRec(x->child[idx + 1], succ);
        } else {                                             // 两边都只有 t-1 个，合并
            merge(x, idx);
            removeRec(x->child[idx], k);
        }
    }

    // 删除主递归：保证进入孩子前，该孩子至少有 t 个关键字（不会删崩）
    void removeRec(Node* x, int k) {
        int idx = findKey(x, k);
        if (idx < (int)x->key.size() && x->key[idx] == k) {
            if (x->leaf) x->key.erase(x->key.begin() + idx);   // 情况 1：叶子直接删
            else removeFromInternal(x, idx);                    // 情况 2：内部结点替换
            return;
        }
        if (x->leaf) return;                                    // 兜底：未找到
        bool last = (idx == (int)x->key.size());
        if ((int)x->child[idx]->key.size() < T) fill(x, idx);   // 情况 3：先补够再下去
        // 合并会让下标发生偏移，需要修正（CLRS 的处理）
        if (last && idx > (int)x->key.size()) removeRec(x->child[idx - 1], k);
        else removeRec(x->child[idx], k);
    }

    // 递归校验：关键字数上下界、结点内有序、叶子同层、孩子个数
    void checkNode(const Node* x, int depth, int& leafDepth, Check& c) const {
        int n = (int)x->key.size();
        if (n > 2 * T - 1) c.keyCountOK = false;                // 上限
        if (x != root && n < T - 1) c.keyCountOK = false;       // 非根下限
        if (x == root && !x->leaf && n < 1) c.keyCountOK = false;
        for (int i = 1; i < n; ++i)
            if (x->key[i - 1] >= x->key[i]) c.sorted = false;
        if (x->leaf) {
            if (leafDepth < 0) leafDepth = depth;
            else if (leafDepth != depth) c.leafSameDepth = false;
            if (!x->child.empty()) c.childCountOK = false;
            return;
        }
        if ((int)x->child.size() != n + 1) c.childCountOK = false;
        for (const Node* ch : x->child) checkNode(ch, depth + 1, leafDepth, c);
    }

public:
    BTree() = default;
    ~BTree() { destroy(root); }
    BTree(const BTree&) = delete;             // 树持有裸指针，禁止拷贝
    BTree& operator=(const BTree&) = delete;

    bool empty() const { return root == nullptr; }

    // ---------------- 查找 ----------------
    // 返回 {所在结点, 下标}；未找到返回 {nullptr, -1}
    std::pair<Node*, int> search(int k) const {
        Node* x = root;
        while (x) {
            int i = 0;
            while (i < (int)x->key.size() && k > x->key[i]) ++i;   // 块内顺序找
            if (i < (int)x->key.size() && x->key[i] == k) return {x, i};
            if (x->leaf) break;
            x = x->child[i];
        }
        return {nullptr, -1};
    }

    // ---------------- 插入 ----------------
    void insert(int k) {
        if (search(k).first) return;            // 本例不允许重复关键字
        if (!root) {                            // 空树
            root = new Node(true);
            root->key.push_back(k);
            return;
        }
        if ((int)root->key.size() == 2 * T - 1) {   // 根满了，先长高一层
            Node* s = new Node(false);
            s->child.push_back(root);
            splitChild(s, 0);
            root = s;
        }
        insertNonFull(root, k);
    }

    // ---------------- 删除 ----------------
    bool remove(int k) {
        if (!root || !search(k).first) return false;
        removeRec(root, k);
        if (root->key.empty() && !root->leaf) {     // 根空了但有孩子：降层
            Node* old = root;
            root = root->child[0];
            delete old;
        }
        if (root && root->key.empty() && root->leaf) {  // 整棵树空了
            delete root;
            root = nullptr;
        }
        return true;
    }

    // ---------------- 校验 ----------------
    Check validate() const {
        Check c;
        if (!root) return c;
        int leafDepth = -1;
        checkNode(root, 1, leafDepth, c);
        std::vector<int> v;
        collectKeys(root, v);
        for (size_t i = 1; i < v.size(); ++i)
            if (v[i - 1] >= v[i]) c.sorted = false;     // 全局中序必须严格递增
        c.totalKeys = (int)v.size();
        const Node* p = root;
        while (p) {                                     // 树高 = 最左路径层数
            ++c.height;
            p = p->leaf ? nullptr : p->child[0];
        }
        return c;
    }

    // ---------------- 打印 ----------------
    // 按层打印，结点之间用 " | " 分隔，结点内关键字用空格分隔
    void printLevels(std::ostream& os) const {
        if (!root) { os << "  （空树）\n"; return; }
        std::queue<Node*> q;
        q.push(root);
        int level = 1;
        while (!q.empty()) {
            int cnt = (int)q.size();
            os << "  第 " << level << " 层: ";
            for (int i = 0; i < cnt; ++i) {
                Node* x = q.front();
                q.pop();
                if (i) os << " | ";
                for (int j = 0; j < (int)x->key.size(); ++j) {
                    if (j) os << ' ';
                    os << x->key[j];
                }
                for (Node* c : x->child) q.push(c);
            }
            os << "\n";
            ++level;
        }
    }

    // 中序输出（即升序）
    void printInorder(std::ostream& os) const {
        std::vector<int> v;
        collectKeys(root, v);
        os << "  中序: ";
        if (v.empty()) os << "（空）";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) os << ' ';
            os << v[i];
        }
        os << "\n";
    }

    // 一行式结构校验报告
    void report(std::ostream& os, const std::string& tag) const {
        Check c = validate();
        os << "  校验[" << tag << "]: 关键字总数=" << c.totalKeys
           << " 树高=" << c.height
           << " 叶层同深=" << (c.leafSameDepth ? "是" : "否")
           << " 关键字有序=" << (c.sorted ? "是" : "否")
           << " 关键字数合法=" << (c.keyCountOK ? "是" : "否")
           << " 孩子数合法=" << (c.childCountOK ? "是" : "否")
           << " => " << (c.ok() ? "通过" : "失败") << "\n";
    }
};

// ============================================================================
// 二、B+ 树（M = 4 阶：每结点最多 4 个孩子、3 个关键字，至少 2 个孩子）
// ============================================================================
class BPlusTree {
public:
    static constexpr int M = 4;

    // 内部结点和叶子共用一个结构，用 leaf 区分
    struct Node {
        bool leaf;
        std::vector<int> key;       // 内部结点：key[i] == 第 i+1 棵子树的最小键
        std::vector<Node*> child;   // 内部结点专用
        Node* next = nullptr;       // 叶子专用：后继叶子（范围查询的顺序通道）
        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    // 校验结果
    struct BCheck {
        int totalKeys = 0;          // 叶子中的关键字总数
        int leafCount = 0;          // 叶子结点个数
        int height = 0;             // 树高（叶子也算一层）
        bool leafSameDepth = true;
        bool sorted = true;         // 结点内有序 + 整条叶子链全局递增
        bool countOK = true;        // 结点关键字数 / 孩子数是否合法
        bool routingOK = true;      // 内部键是否等于右子树最小键
        bool ok() const { return leafSameDepth && sorted && countOK && routingOK; }
    };

    // 插入的返回值：是否发生分裂、上移的键、新产生的右结点
    struct Split {
        bool split = false;
        int upKey = 0;
        Node* right = nullptr;
    };

private:
    Node* root;
    Node* firstLeaf;                // 最左叶子，叶子链的头

    static void destroy(Node* x) {
        if (!x) return;
        for (Node* c : x->child) destroy(c);
        delete x;
    }

    // 子树中的最小键：一路向左走到叶子
    static int minKeyOf(const Node* x) {
        while (!x->leaf) x = x->child[0];
        return x->key.empty() ? 0 : x->key.front();
    }

    // 递归插入，返回是否溢出以及溢出后的上移键/新结点
    Split insertRec(Node* x, int k) {
        Split res;
        if (x->leaf) {
            auto it = std::lower_bound(x->key.begin(), x->key.end(), k);
            if (it != x->key.end() && *it == k) return res;      // 已存在，忽略
            x->key.insert(it, k);
            if ((int)x->key.size() <= M - 1) return res;         // 没超上限
            // 叶子溢出：前一半留左边，后一半进新叶子，右叶首键复制到父结点
            int half = (int)x->key.size() / 2;
            Node* right = new Node(true);
            right->key.assign(x->key.begin() + half, x->key.end());
            x->key.resize(half);
            right->next = x->next;    // 插到叶子链：原叶 -> 新叶 -> 原后继
            x->next = right;
            res.split = true;
            res.upKey = right->key.front();
            res.right = right;
            return res;
        }
        // 内部结点：按索引键路由（upper_bound 得到孩子下标）
        int i = (int)(std::upper_bound(x->key.begin(), x->key.end(), k)
                      - x->key.begin());
        Split cr = insertRec(x->child[i], k);
        if (!cr.split) return res;
        x->key.insert(x->key.begin() + i, cr.upKey);
        x->child.insert(x->child.begin() + i + 1, cr.right);
        if ((int)x->child.size() <= M) return res;               // 没超上限
        // 内部结点分裂：mid 号孩子归右，key[mid-1] 上移（移动，不复制）
        int mid = (int)x->child.size() / 2;
        Node* right = new Node(false);
        right->key.assign(x->key.begin() + mid, x->key.end());
        right->child.assign(x->child.begin() + mid, x->child.end());
        int up = x->key[mid - 1];
        x->key.resize(mid - 1);
        x->child.resize(mid);
        res.split = true;
        res.upKey = up;
        res.right = right;
        return res;
    }

    // 递归校验
    void checkNode(const Node* x, int depth, int& leafDepth, BCheck& c) const {
        int n = (int)x->key.size();
        for (int i = 1; i < n; ++i)
            if (x->key[i - 1] >= x->key[i]) c.sorted = false;
        if (x->leaf) {
            if (leafDepth < 0) leafDepth = depth;
            else if (leafDepth != depth) c.leafSameDepth = false;
            if (n > M - 1) c.countOK = false;
            if (x != root && n < (M + 1) / 2 - 1) c.countOK = false;  // 叶子下限 1
            if (!x->child.empty()) c.countOK = false;
            return;
        }
        if (n > M - 1) c.countOK = false;
        if (x != root && (int)x->child.size() < (M + 1) / 2) c.countOK = false;
        if ((int)x->child.size() != n + 1) {
            c.countOK = false;                     // 孩子数不对，下面的路由不再检查
            return;
        }
        for (int i = 0; i < n; ++i)                 // 索引键必须等于右子树最小键
            if (minKeyOf(x->child[i + 1]) != x->key[i]) c.routingOK = false;
        for (const Node* ch : x->child) checkNode(ch, depth + 1, leafDepth, c);
    }

public:
    BPlusTree() {
        root = new Node(true);      // 初始只有空的根叶子
        firstLeaf = root;
    }
    ~BPlusTree() { destroy(root); }
    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    bool empty() const { return root->leaf && root->key.empty(); }

    // ---------------- 插入 ----------------
    void insert(int k) {
        Split s = insertRec(root, k);
        if (s.split) {                  // 根分裂：树高 +1
            Node* nr = new Node(false);
            nr->key.push_back(s.upKey);
            nr->child.push_back(root);
            nr->child.push_back(s.right);
            root = nr;
        }
    }

    // ---------------- 范围查询 ----------------
    // 先按 lo 路由到叶子，再沿 next 链顺序扫描，返回值与访问的叶子数
    std::vector<int> rangeQuery(int lo, int hi, int& leavesVisited) const {
        std::vector<int> out;
        leavesVisited = 0;
        Node* x = root;
        while (!x->leaf) {
            int i = (int)(std::upper_bound(x->key.begin(), x->key.end(), lo)
                          - x->key.begin());
            x = x->child[i];
        }
        for (Node* p = x; p != nullptr; p = p->next) {
            ++leavesVisited;
            bool stop = false;
            for (int v : p->key) {
                if (v >= lo && v <= hi) out.push_back(v);
                if (v > hi) { stop = true; break; }   // 越过上界，整条链都可以停
            }
            if (stop) break;
        }
        return out;
    }

    // ---------------- 校验 ----------------
    BCheck validate() const {
        BCheck c;
        if (!root) return c;
        int leafDepth = -1;
        checkNode(root, 1, leafDepth, c);
        bool hasPrev = false;
        int prevMax = 0;
        for (const Node* p = firstLeaf; p; p = p->next) {   // 顺着叶子链检查
            ++c.leafCount;
            c.totalKeys += (int)p->key.size();
            for (size_t i = 0; i < p->key.size(); ++i) {
                if (i && p->key[i - 1] >= p->key[i]) c.sorted = false;
                if (hasPrev && p->key[i] <= prevMax) c.sorted = false;
                prevMax = p->key[i];
                hasPrev = true;
            }
        }
        const Node* q = root;
        while (q) {
            ++c.height;
            q = q->leaf ? nullptr : q->child[0];
        }
        return c;
    }

    // ---------------- 打印 ----------------
    // 按层打印（内部结点的键用 [] 括起，直观区分"索引"与"数据"）
    void printLevels(std::ostream& os) const {
        std::queue<Node*> q;
        q.push(root);
        int level = 1;
        while (!q.empty()) {
            int cnt = (int)q.size();
            os << "  第 " << level << " 层: ";
            for (int i = 0; i < cnt; ++i) {
                Node* x = q.front();
                q.pop();
                if (i) os << " | ";
                os << '[';
                for (int j = 0; j < (int)x->key.size(); ++j) {
                    if (j) os << ' ';
                    os << x->key[j];
                }
                os << ']';
                for (Node* c : x->child) q.push(c);
            }
            os << "\n";
            ++level;
        }
    }

    // 打印叶子链
    void printLeaves(std::ostream& os) const {
        os << "  叶子链: ";
        for (const Node* p = firstLeaf; p; p = p->next) {
            os << '[';
            for (size_t i = 0; i < p->key.size(); ++i) {
                if (i) os << ' ';
                os << p->key[i];
            }
            os << ']';
            if (p->next) os << " -> ";
        }
        os << "\n";
    }

    void report(std::ostream& os, const std::string& tag) const {
        BCheck c = validate();
        os << "  校验[" << tag << "]: 关键字总数=" << c.totalKeys
           << " 叶子数=" << c.leafCount
           << " 树高=" << c.height
           << " 叶层同深=" << (c.leafSameDepth ? "是" : "否")
           << " 有序=" << (c.sorted ? "是" : "否")
           << " 数量合法=" << (c.countOK ? "是" : "否")
           << " 索引正确=" << (c.routingOK ? "是" : "否")
           << " => " << (c.ok() ? "通过" : "失败") << "\n";
    }
};

// ============================================================================
// 三、测试
// ============================================================================
// 打印 B 树查找结果
static void showSearch(BTree& bt, int k) {
    auto [node, idx] = bt.search(k);
    if (node) {
        std::cout << "  查找 " << k << ": 命中，所在结点关键字 =";
        for (int v : node->key) std::cout << ' ' << v;
        std::cout << "（下标 " << idx << "）\n";
    } else {
        std::cout << "  查找 " << k << ": 未命中（到达叶子仍未找到，查找失败）\n";
    }
}

int main() {
    std::cout << "===== (1) B 树 t=3（6 阶）：依次插入 1~10 =====\n";
    BTree bt;
    for (int i = 1; i <= 10; ++i) bt.insert(i);
    bt.printLevels(std::cout);
    bt.printInorder(std::cout);
    bt.report(std::cout, "(1) 插入 1~10 后");

    std::cout << "\n===== (2) 查找：命中与失败 =====\n";
    showSearch(bt, 7);
    showSearch(bt, 11);

    std::cout << "\n===== (3) 删除单个关键字 =====\n";
    std::cout << "  删除 10（在叶结点，直接删，不欠账）\n";
    bt.remove(10);
    bt.printLevels(std::cout);
    bt.printInorder(std::cout);
    bt.report(std::cout, "删除 10 后");

    std::cout << "  删除 3（在根结点即内部结点：两个孩子都只有 t-1=2 个关键字，\n";
    std::cout << "           借不到 -> 先与右孩子合并，再把 3 从合并后的叶子中删掉）\n";
    bt.remove(3);
    bt.printLevels(std::cout);
    bt.printInorder(std::cout);
    bt.report(std::cout, "删除 3 后");

    std::cout << "  删除不存在的 99\n";
    std::cout << "  删除结果 = " << (bt.remove(99) ? "成功" : "失败（键不存在）") << "\n";

    std::cout << "\n===== (4) 连续删除到空树：1 2 4 5 6 7 8 9 =====\n";
    int leftKeys[] = {1, 2, 4, 5, 6, 7, 8, 9};
    for (int k : leftKeys) {
        bt.remove(k);
        bt.report(std::cout, "删除 " + std::to_string(k) + " 后");
    }
    std::cout << "  空树判断: " << (bt.empty() ? "是空树" : "非空") << "\n";

    std::cout << "\n===== (5) B+ 树 M=4：插入 1~12，范围查询 [4,9] =====\n";
    BPlusTree bp;
    for (int i = 1; i <= 12; ++i) bp.insert(i);
    bp.printLevels(std::cout);
    bp.printLeaves(std::cout);
    bp.report(std::cout, "(5) 插入 1~12 后");

    int visited = 0;
    std::vector<int> r = bp.rangeQuery(4, 9, visited);
    std::cout << "  范围查询 [4,9]: 结果 =";
    for (int v : r) std::cout << ' ' << v;
    std::cout << "（访问叶子数 = " << visited << "，返回 " << r.size() << " 个值）\n";

    int visited2 = 0;
    std::vector<int> r2 = bp.rangeQuery(10, 20, visited2);
    std::cout << "  范围查询 [10,20]: 结果 =";
    for (int v : r2) std::cout << ' ' << v;
    std::cout << "（访问叶子数 = " << visited2 << "）\n";

    std::cout << "\n===== (6) 练习 2：m=5、n=1000 的高度上界与最少关键字 =====\n";
    // 高度上界：h <= log_ceil(m/2)((n+1)/2) + 1，这里 ceil(5/2)=3
    int m = 5, n1000 = 1000;
    int k = (m + 1) / 2;                      // ceil(m/2) = 3，即最少孩子数
    std::cout << "  ceil(m/2) = " << k << "\n";
    // 用递推求最大高度：N_min(h) = 2*k^(h-1) - 1，且严格递增
    long long nm = 1;                         // N_min(1) = 1
    int hMax = 1;
    while (k * (nm + 1) - 1 <= n1000) {       // N_min(h+1) = k*(N_min(h)+1)-1
        nm = k * (nm + 1) - 1;
        ++hMax;
    }
    std::cout << "  n = " << n1000 << " 时高度上限 h = " << hMax
              << "（N_min(" << hMax << ") = " << nm
              << " <= 1000 < N_min(" << hMax + 1 << ") = "
              << k * (nm + 1) - 1 << "）\n";
    // 撑起高度 4 的最少关键字：N_min(4) = 2*3^3 - 1 = 53
    std::cout << "  撑起高度 4 至少需要关键字数 = 2*3^3 - 1 = 53\n";

    std::cout << "\n===== (7) 用 1~127 实测 6 阶 B 树高度，验证上界 =====\n";
    BTree big;
    for (int i = 1; i <= 127; ++i) big.insert(i);
    BTree::Check bc = big.validate();
    std::cout << "  关键字总数 = " << bc.totalKeys << "，实测树高 = " << bc.height << "\n";
    std::cout << "  理论：2*3^3-1 = 53 <= 127 < 2*3^4-1 = 161，故高度应为 4\n";
    big.report(std::cout, "(7) 1~127 插入后");

    std::cout << "\n===== (8) 练习 5 提示：B+ 树为何适合做磁盘索引 =====\n";
    std::cout << "  内部结点只存键与指针 -> 扇出大 -> 树更矮 -> I/O 次数少\n";
    std::cout << "  叶子链有序相连 -> 范围查询顺序扫描 -> 顺序 I/O 而非随机回溯\n";
    std::cout << "  所有查找都落到叶子 -> 路径长度恒定 -> 延迟可预测\n";

    std::cout << "\n全部测试完成。\n";
    return 0;
}

/* ============================================================================
 * 预期输出（与下面内容逐行一致，由实际运行结果回填）
 * ----------------------------------------------------------------------------
 * ===== (1) B 树 t=3（6 阶）：依次插入 1~10 =====
 *   第 1 层: 3 6
 *   第 2 层: 1 2 | 4 5 | 7 8 9 10
 *   中序: 1 2 3 4 5 6 7 8 9 10
 *   校验[(1) 插入 1~10 后]: 关键字总数=10 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *
 * ===== (2) 查找：命中与失败 =====
 *   查找 7: 命中，所在结点关键字 = 7 8 9 10（下标 0）
 *   查找 11: 未命中（到达叶子仍未找到，查找失败）
 *
 * ===== (3) 删除单个关键字 =====
 *   删除 10（在叶结点，直接删，不欠账）
 *   第 1 层: 3 6
 *   第 2 层: 1 2 | 4 5 | 7 8 9
 *   中序: 1 2 3 4 5 6 7 8 9
 *   校验[删除 10 后]: 关键字总数=9 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   删除 3（在根结点即内部结点：两个孩子都只有 t-1=2 个关键字，
 *            借不到 -> 先与右孩子合并，再把 3 从合并后的叶子中删掉）
 *   第 1 层: 6
 *   第 2 层: 1 2 4 5 | 7 8 9
 *   中序: 1 2 4 5 6 7 8 9
 *   校验[删除 3 后]: 关键字总数=8 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   删除不存在的 99
 *   删除结果 = 失败（键不存在）
 *
 * ===== (4) 连续删除到空树：1 2 4 5 6 7 8 9 =====
 *   校验[删除 1 后]: 关键字总数=7 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 2 后]: 关键字总数=6 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 4 后]: 关键字总数=5 树高=2 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 5 后]: 关键字总数=4 树高=1 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 6 后]: 关键字总数=3 树高=1 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 7 后]: 关键字总数=2 树高=1 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 8 后]: 关键字总数=1 树高=1 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   校验[删除 9 后]: 关键字总数=0 树高=0 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *   空树判断: 是空树
 *
 * ===== (5) B+ 树 M=4：插入 1~12，范围查询 [4,9] =====
 *   第 1 层: [5]
 *   第 2 层: [3] | [7 9 11]
 *   第 3 层: [1 2] | [3 4] | [5 6] | [7 8] | [9 10] | [11 12]
 *   叶子链: [1 2] -> [3 4] -> [5 6] -> [7 8] -> [9 10] -> [11 12]
 *   校验[(5) 插入 1~12 后]: 关键字总数=12 叶子数=6 树高=3 叶层同深=是 有序=是 数量合法=是 索引正确=是 => 通过
 *   范围查询 [4,9]: 结果 = 4 5 6 7 8 9（访问叶子数 = 4，返回 6 个值）
 *   范围查询 [10,20]: 结果 = 10 11 12（访问叶子数 = 2）
 *
 * ===== (6) 练习 2：m=5、n=1000 的高度上界与最少关键字 =====
 *   ceil(m/2) = 3
 *   n = 1000 时高度上限 h = 6（N_min(6) = 485 <= 1000 < N_min(7) = 1457）
 *   撑起高度 4 至少需要关键字数 = 2*3^3 - 1 = 53
 *
 * ===== (7) 用 1~127 实测 6 阶 B 树高度，验证上界 =====
 *   关键字总数 = 127，实测树高 = 4
 *   理论：2*3^3-1 = 53 <= 127 < 2*3^4-1 = 161，故高度应为 4
 *   校验[(7) 1~127 插入后]: 关键字总数=127 树高=4 叶层同深=是 关键字有序=是 关键字数合法=是 孩子数合法=是 => 通过
 *
 * ===== (8) 练习 5 提示：B+ 树为何适合做磁盘索引 =====
 *   内部结点只存键与指针 -> 扇出大 -> 树更矮 -> I/O 次数少
 *   叶子链有序相连 -> 范围查询顺序扫描 -> 顺序 I/O 而非随机回溯
 *   所有查找都落到叶子 -> 路径长度恒定 -> 延迟可预测
 *
 * 全部测试完成。
 * ============================================================================ */
