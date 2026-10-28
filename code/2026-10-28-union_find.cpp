// ============================================================
// 并查集（Union-Find / Disjoint Set Union）—— 课程第 17~18 期配套代码
// 功能：
//   1. QuickFind：数组直接存"集合编号"，find O(1)，合并 O(n)
//   2. QuickUnion：数组存"父结点"，合并快但可能退化成链
//   3. 按大小合并 + 路径压缩的并查集（工程/考试标准写法）
//   4. 实测：朴素并查集的链式退化、路径压缩的效果、按大小合并的树高上界
//   5. 应用：统计连通分量（朋友圈/亲戚问题），判断两点是否连通
// 编译：g++ -std=c++17 -Wall -Wextra -o union_find.exe union_find.cpp
// ============================================================
#include <algorithm>  // std::swap, std::max
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// 版本 1：QuickFind —— 每个元素直接记住"我属于哪个集合"
// ------------------------------------------------------------
// 思路：id[x] 直接就是 x 所在集合的编号（代表元）。
//       find 只要一行，O(1)；但合并时要把另一个集合里所有元素
//       的编号全改一遍，O(n)。
// 适合：查询极多、合并极少的场景。
// ============================================================
class QuickFind {
public:
    explicit QuickFind(int n) : id_(n) {
        for (int i = 0; i < n; ++i) {
            id_[i] = i;  // 初始时每个人自成一个集合，编号就是自己
        }
    }

    // 查找 x 属于哪个集合：直接返回，O(1)
    int find(int x) const { return id_[x]; }

    // 判断 a、b 是否在同一个集合：比较集合编号即可，O(1)
    bool connected(int a, int b) const { return id_[a] == id_[b]; }

    // 合并 a、b 所在的两个集合：把"编号为 rb 的人"全部改成 ra，O(n)
    void unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) {
            return;  // 本来就在一个集合，什么都不用做
        }
        for (int i = 0; i < static_cast<int>(id_.size()); ++i) {
            if (id_[i] == rb) {
                id_[i] = ra;  // 逐个改编号（这正是 O(n) 的来源）
            }
        }
    }

    // 当前还剩几个集合：数一数 id 里出现了几种不同的编号
    int count() const {
        std::vector<char> seen(id_.size(), 0);
        int c = 0;
        for (int i = 0; i < static_cast<int>(id_.size()); ++i) {
            int r = id_[i];
            if (r >= 0 && r < static_cast<int>(id_.size()) && !seen[r]) {
                seen[r] = 1;
                ++c;
            }
        }
        return c;
    }

    // 打印整个 id 数组，方便观察合并效果
    void dump(const std::string& tag) const {
        std::cout << tag << " id :";
        for (int i = 0; i < static_cast<int>(id_.size()); ++i) {
            std::cout << ' ' << id_[i];
        }
        std::cout << "   (集合数 = " << count() << ")\n";
    }

private:
    std::vector<int> id_;  // id_[x] = x 所属集合的编号
};

// ============================================================
// 版本 2：QuickUnion —— 每个元素只记住"我的父结点"
// ------------------------------------------------------------
// 思路：parent[x] 指向 x 的父结点，一路往上走，根结点 parent[r] == r
//       就是集合代表元。合并时只需让一棵树的根指向另一棵的根，O(h)。
// 缺点：合并方向随便选，可能长成一条长链，h 退化成 n。
// ============================================================
class QuickUnion {
public:
    explicit QuickUnion(int n) : parent_(n), steps_(0) {
        for (int i = 0; i < n; ++i) {
            parent_[i] = i;  // 初始各自为根
        }
    }

    // 普通 find：沿父指针一直往上走，走到 parent[x] == x 的根
    // 顺便统计"走了几条边"，用来量化查找代价
    int find(int x) {
        int r = x;
        while (parent_[r] != r) {
            r = parent_[r];
            ++steps_;  // 每往上走一层，代价 +1
        }
        return r;
    }

    // 带路径压缩的 find：
    // 第一趟先找到根；第二趟把路上所有结点直接挂到根下面。
    // 之后再查这些结点就只要一步了。
    int findCompress(int x) {
        int r = x;
        while (parent_[r] != r) {  // 第一趟：找根
            r = parent_[r];
            ++steps_;
        }
        while (parent_[x] != r) {  // 第二趟：把路径压平
            int next = parent_[x];  // 先记住原来的父结点
            parent_[x] = r;         // 当前结点改为直接指向根
            x = next;               // 继续处理路径上的下一个
        }
        return r;
    }

    // 合并：把 a 的根挂到 b 的根下面（这里故意不做任何优化，
    // 方便演示"链式退化"）
    void unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra != rb) {
            parent_[ra] = rb;
        }
    }

    // 树的深度 = 从根到最深结点的边数，也等于单次 find 的最坏代价
    int depth() const {
        int best = 0;
        for (int i = 0; i < static_cast<int>(parent_.size()); ++i) {
            int d = 0;
            int x = i;
            while (parent_[x] != x) {  // 一直往上数
                x = parent_[x];
                ++d;
            }
            best = std::max(best, d);
        }
        return best;
    }

    int steps() const { return steps_; }
    void resetSteps() { steps_ = 0; }

private:
    std::vector<int> parent_;  // parent_[x] = x 的父结点（根指向自己）
    int steps_;                // 最近若干次 find 累计走过的边数
};

// ============================================================
// 版本 3：按大小合并 + 路径压缩（推荐写法）
// ------------------------------------------------------------
// 两条优化：
//   ① 按大小合并（union by size）：小树挂到大树下面，
//      保证任何一棵树的深度不超过 log2(n)；
//   ② 路径压缩（path compression）：find 时把路径上的点直接挂到根，
//      下次查找几乎 O(1)。
// 两者合用：n 次操作的均摊代价是 O(α(n))，α 是反阿克曼函数，
// 在实际能遇到的规模下 α(n) ≤ 4，可以当作常数。
// ============================================================
class UnionFind {
public:
    explicit UnionFind(int n) : parent_(n), size_(n, 1), cnt_(n) {
        for (int i = 0; i < n; ++i) {
            parent_[i] = i;  // 每个元素自成一个连通分量
        }
    }

    // 查找根 + 路径压缩（迭代写法，避免递归深度问题）
    int find(int x) {
        int r = x;
        while (parent_[r] != r) {  // 第一趟：找到根
            r = parent_[r];
        }
        while (parent_[x] != r) {  // 第二趟：路径上每个点都直接连到根
            int next = parent_[x];
            parent_[x] = r;
            x = next;
        }
        return r;
    }

    // 只统计代价、不做压缩的 find，用来对比优化前后
    int findSteps(int x) const {
        int d = 0;
        while (parent_[x] != x) {
            x = parent_[x];
            ++d;
        }
        return d;
    }

    // 合并：返回 true 表示这次合并真的把两个集合并成了一个
    bool unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) {
            return false;  // 已经在同一集合，合并无效（可用于判环）
        }
        if (size_[ra] < size_[rb]) {
            std::swap(ra, rb);  // 保证 ra 是较大的那棵树的根
        }
        parent_[rb] = ra;         // 小树挂到大树下面
        size_[ra] += size_[rb];   // 维护集合大小
        --cnt_;                   // 连通分量数减一
        return true;
    }

    bool connected(int a, int b) { return find(a) == find(b); }

    int count() const { return cnt_; }               // 连通分量个数
    int sizeOf(int x) { return size_[find(x)]; }     // x 所在集合的大小

    // 树深（不做压缩地统计），用于验证"按大小合并后深度 ≤ log2 n"
    int depth() const {
        int best = 0;
        for (int i = 0; i < static_cast<int>(parent_.size()); ++i) {
            best = std::max(best, findSteps(i));
        }
        return best;
    }

private:
    std::vector<int> parent_;  // 父结点数组（双亲表示法）
    std::vector<int> size_;    // 只有根结点的 size_ 有意义：该集合元素个数
    int cnt_;                  // 当前连通分量个数
};

int main() {
    // ---------- (1) QuickFind：合并的代价看得很清楚 ----------
    std::cout << "(1) QuickFind 演示（10 个元素）\n";
    QuickFind qf(10);
    std::cout << "    初始  ";
    qf.dump("");
    qf.unite(0, 1);  // 0 和 1 并成一个集合
    qf.unite(2, 3);  // 2 和 3 并成一个集合
    qf.unite(0, 3);  // 把 {0,1} 和 {2,3} 并起来
    qf.unite(5, 6);  // 再来一对
    std::cout << "    合并后";
    qf.dump("");
    std::cout << "    0 与 2 连通? " << (qf.connected(0, 2) ? "是" : "否")
              << "；0 与 5 连通? " << (qf.connected(0, 5) ? "是" : "否") << "\n\n";

    // ---------- (2) QuickUnion：同样一组合并，结果一致 ----------
    std::cout << "(2) QuickUnion 演示（同样一组合并，连通性判断应完全一致）\n";
    QuickUnion qu(10);
    qu.unite(0, 1);
    qu.unite(2, 3);
    qu.unite(0, 3);
    qu.unite(5, 6);
    std::cout << "    parent 数组:";
    for (int i = 0; i < 10; ++i) {
        std::cout << ' ' << i << "->" << qu.find(i);  // 打印每个点的根
    }
    std::cout << "\n";
    std::cout << "    0 与 2 连通? " << (qu.find(0) == qu.find(2) ? "是" : "否")
              << "；0 与 5 连通? " << (qu.find(0) == qu.find(5) ? "是" : "否") << "\n\n";

    // ---------- (3) 朴素 QuickUnion 的最坏情况：退化成一条链 ----------
    const int N = 1000;
    QuickUnion chain(N);
    for (int i = 0; i + 1 < N; ++i) {
        chain.unite(i, i + 1);  // 0-1, 1-2, 2-3 ... 每次都把小根挂到大根下
    }
    chain.resetSteps();
    chain.find(0);  // 从链的最深处往上找根
    std::cout << "(3) 朴素 QuickUnion 依次合并 0-1,1-2,...," << N - 1 << '-' << N
              << "\n    树深 = " << chain.depth()
              << "，find(0) 走了 " << chain.steps() << " 步（退化成 O(n)）\n\n";

    // ---------- (4) 对同一条链做路径压缩 ----------
    // 注意：这里复用 (3) 那个已经退化的结构，观察压缩前后的代价
    chain.resetSteps();
    chain.findCompress(0);  // 第一次：仍要走完整条链，但顺手把路径压平
    int firstCost = chain.steps();
    chain.resetSteps();
    chain.findCompress(0);  // 第二次：根已经在头顶，一步到位
    int secondCost = chain.steps();
    std::cout << "(4) 路径压缩效果（同一条链）\n";
    std::cout << "    第一次 findCompress(0) 走 " << firstCost << " 步，之后树深 = "
              << chain.depth() << "\n";
    std::cout << "    第二次 findCompress(0) 只走 " << secondCost << " 步\n\n";

    // ---------- (5) 按大小合并：树高被压到 log n ----------
    // 用"同样大的树两两合并"的最坏顺序：这样每一层都会让树再长高 1，
    // 是把树长得最高的合并方式，正好用来验证 log2 n 的上界。
    const int M = 1024;
    UnionFind uf(M);
    for (int step = 1; step < M; step *= 2) {  // 第 1 层、第 2 层、第 3 层……
        for (int i = 0; i + step < M; i += 2 * step) {
            uf.unite(i, i + step);  // 两棵大小相等的树合并
        }
    }
    std::cout << "(5) 按大小合并，用最坏顺序（同样大的树两两合并）n = " << M << "\n"
              << "    树深 = " << uf.depth() << "，log2(" << M << ") = 10"
              << "，可见树高被压在 log2 n 以内\n"
              << "    合并完成，连通分量 = " << uf.count()
              << "，集合大小 = " << uf.sizeOf(0) << "\n\n";

    // ---------- (6) 应用：朋友圈 / 连通分量计数 ----------
    // n = 8 个学生，给出 5 对"认识"关系：
    //   (0,1) (1,2) (3,4) (5,6) (6,7)
    // 认识关系可以传递：0-1-2 是一伙，3-4 是一伙，5-6-7 是一伙
    // 所以答案应该是 3 个朋友圈
    std::cout << "(6) 应用：8 个学生、5 对朋友关系，统计朋友圈个数\n";
    UnionFind circle(8);
    int edges[5][2] = {{0, 1}, {1, 2}, {3, 4}, {5, 6}, {6, 7}};
    int merged = 0;
    for (int i = 0; i < 5; ++i) {
        if (circle.unite(edges[i][0], edges[i][1])) {
            ++merged;  // 这条边真的把两伙人并成了一伙
        }
    }
    std::cout << "    5 条边中有效合并 " << merged << " 次，"
              << "最终 " << circle.count() << " 个朋友圈\n";
    std::cout << "    0 与 2 是朋友? " << (circle.connected(0, 2) ? "是" : "否")
              << "；0 与 3 是朋友? " << (circle.connected(0, 3) ? "是" : "否")
              << "；2 所在圈子人数 = " << circle.sizeOf(2) << "\n\n";

    // ---------- (7) 用并查集判环：Kruskal 的核心一步 ----------
    // 在图 (0-1, 1-2, 2-0) 上加边，第三条边两端已经连通 => 成环
    std::cout << "(7) 判环演示：三角形 0-1-2\n";
    UnionFind loop(3);
    int tri[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    for (int i = 0; i < 3; ++i) {
        bool ok = loop.unite(tri[i][0], tri[i][1]);
        std::cout << "    加边 " << tri[i][0] << '-' << tri[i][1] << " ："
                  << (ok ? "并入成功" : "两端已连通，形成环！") << "\n";
    }
    std::cout << "\n";

    // ---------- (8) 练习题判断题结论 ----------
    // (a) QuickFind 的 find 是 O(1)，所以它总是比 QuickUnion 好。
    //     => 错。合并是 O(n)，元素多、合并多时远慢于 QuickUnion。
    // (b) 按大小合并后，任何结点的查找代价都不超过 log2(n)+1。
    //     => 对。每被挂一次，所在树至少翻倍，所以最多挂 log2 n 次。
    // (c) 路径压缩后单次 find 的复杂度一定降到 O(1)。
    //     => 错。是"均摊"O(α(n))，个别操作仍可能走较长路径。
    std::cout << "(8) 判断题结论：a=错 b=对 c=错（理由见笔记第 6 节）\n";

    return 0;
}

/* ======================= 预期输出 =======================
(1) QuickFind 演示（10 个元素）
    初始   id : 0 1 2 3 4 5 6 7 8 9   (集合数 = 10)
    合并后 id : 0 0 0 0 4 5 5 7 8 9   (集合数 = 6)
    0 与 2 连通? 是；0 与 5 连通? 否

(2) QuickUnion 演示（同样一组合并，连通性判断应完全一致）
    parent 数组: 0->3 1->3 2->3 3->3 4->4 5->6 6->6 7->7 8->8 9->9
    0 与 2 连通? 是；0 与 5 连通? 否

(3) 朴素 QuickUnion 依次合并 0-1,1-2,...,999-1000
    树深 = 999，find(0) 走了 999 步（退化成 O(n)）

(4) 路径压缩效果（同一条链）
    第一次 findCompress(0) 走 999 步，之后树深 = 1
    第二次 findCompress(0) 只走 1 步

(5) 按大小合并，用最坏顺序（同样大的树两两合并）n = 1024
    树深 = 10，log2(1024) = 10，可见树高被压在 log2 n 以内
    合并完成，连通分量 = 1，集合大小 = 1024

(6) 应用：8 个学生、5 对朋友关系，统计朋友圈个数
    5 条边中有效合并 5 次，最终 3 个朋友圈
    0 与 2 是朋友? 是；0 与 3 是朋友? 否；2 所在圈子人数 = 3

(7) 判环演示：三角形 0-1-2
    加边 0-1 ：并入成功
    加边 1-2 ：并入成功
    加边 2-0 ：两端已连通，形成环！

(8) 判断题结论：a=错 b=对 c=错（理由见笔记第 6 节）
======================================================= */
