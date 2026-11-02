// ============================================================
// 拓扑排序（Topological Sort）—— AOV 网上的排序
// 对应笔记：notes/2026-11-03-拓扑排序.md
// 编译：g++ -std=c++17 -Wall -Wextra -o ds_check 2026-11-03-topological_sort.cpp
//
// 一句话：拓扑排序把 DAG（有向无环图）的顶点排成一列，
//         保证每条边 u→v 中 u 都排在 v 前面。
// 本文件包含：
//   1) Kahn 算法（入度表 + 队列，顺带判环）—— 最常用
//   2) DFS 法（按退出时间的逆序输出）
//   3) 回溯法统计拓扑序列的个数（练习 4 答案）
//   4) 拓扑序唯一性判断（练习 5 答案：任何时刻队列最多 1 个顶点 ⇔ 唯一）
// ============================================================
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

// 顶点用编号 0..n-1，打印时转成字母 A~Z 方便阅读
char vname(int v) { return static_cast<char>('A' + v); }

// ============================================================
// 图的存储：邻接表（拓扑排序的最优选择，理由见笔记 ③）
// ============================================================
struct Graph {
    int n;                      // 顶点数
    vector<vector<int>> adj;    // adj[u] = u 的所有后继（出边表）

    explicit Graph(int n_) : n(n_), adj(n_) {}

    // 加一条有向边 u → v：任务 u 必须在 v 之前完成，方向是 u 指向 v
    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }
};

// ============================================================
// Kahn 算法（BFS 思想）
//
// 思路四步（详见笔记 ④）：
//   1. 扫一遍所有边，统计每个顶点的入度；
//   2. 所有入度为 0 的顶点入队（没有前驱，可以立刻"开工"）；
//   3. 出队 u 记入结果，u 的每个后继 v 入度减 1，减到 0 就入队；
//   4. 结束时输出的顶点数 < n ⇒ 剩下的顶点互相等待，图中有环。
//
// 返回值：true = 图是 DAG，order 里是一组合法拓扑序；
//         false = 有环，order 不完整。
// 复杂度：每个顶点进出队各一次 O(V)，每条边被看一次 O(E)，共 O(V+E)。
// ============================================================
bool kahnTopoSort(const Graph& g, vector<int>& order) {
    order.clear();

    // 第 1 步：统计入度。注意是对"边表里的终点 v"加 1，不是对 u 加（易错点 2）
    vector<int> indeg(g.n, 0);
    for (int u = 0; u < g.n; ++u)
        for (int v : g.adj[u])
            ++indeg[v];

    // 第 2 步：所有入度为 0 的顶点入队（按下标从小到大扫，保证输出可复现）
    queue<int> q;
    for (int v = 0; v < g.n; ++v)
        if (indeg[v] == 0) q.push(v);

    // 第 3 步：不断出队、删边（入度减 1）、入度为 0 的后继入队
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int v : g.adj[u]) {
            --indeg[v];                 // "删掉"边 u→v
            if (indeg[v] == 0)          // 只有减到 0 才入队（易错点 4）
                q.push(v);
        }
    }

    // 第 4 步：输出顶点数不足 n ⇒ 有环（易错点 3，务必检查！）
    return static_cast<int>(order.size()) == g.n;
}

// ============================================================
// DFS 法求拓扑序
//
// 性质（笔记 ②-3）：DAG 里边 u→v 只能是树边/前进边/横向边（没有回边），
// 无论哪种情况 u 都比 v 晚退出 DFS；按退出时间的逆序排列就是拓扑序。
//
// 实现：DFS 从 u 返回前把 u 压入 finish（访问标记防重复），
// 全部结束后把 finish 整个反转即为拓扑序。
// 颜色数组在这里只用于防重复访问；若要 DFS 法判环需三色标记（0/1/2）。
// ============================================================
void dfs(int u, const Graph& g, vector<bool>& visited, vector<int>& finish) {
    visited[u] = true;
    for (int v : g.adj[u])
        if (!visited[v])
            dfs(v, g, visited, finish);
    finish.push_back(u);        // 从 u 返回之前记录"退出时间"
}

vector<int> dfsTopoSort(const Graph& g) {
    vector<bool> visited(g.n, false);
    vector<int> finish;                 // 按退出时间从小到大
    for (int v = 0; v < g.n; ++v)
        if (!visited[v])
            dfs(v, g, visited, finish);
    // 逆序输出：退出越晚说明依赖越底层，应该排在越前面
    return vector<int>(finish.rbegin(), finish.rend());
}

// ============================================================
// 练习 4 答案：回溯法统计拓扑序列的个数
//
// 思路：维护一份入度数组，每一步从"当前入度为 0 且未用"的顶点里
// 任选一个放在下一个位置，递归下去；选完 n 个就计数 +1。
// 适合小图（指数级），用于验证手算结果。
// ============================================================
int countAll(int depth, int n, vector<int>& indeg, vector<bool>& used,
             const Graph& g) {
    if (depth == n) return 1;           // 选满 n 个顶点，找到一条拓扑序
    int cnt = 0;
    for (int v = 0; v < n; ++v) {
        if (used[v] || indeg[v] > 0) continue;   // 只能选当前入度为 0 的
        used[v] = true;                          // 选 v
        for (int w : g.adj[v]) --indeg[w];       // "删掉" v 的所有出边
        cnt += countAll(depth + 1, n, indeg, used, g);
        for (int w : g.adj[v]) ++indeg[w];       // 回溯：恢复现场
        used[v] = false;
    }
    return cnt;
}

int countTopoOrders(const Graph& g) {
    vector<int> indeg(g.n, 0);
    for (int u = 0; u < g.n; ++u)
        for (int v : g.adj[u])
            ++indeg[v];
    vector<bool> used(g.n, false);
    return countAll(0, g.n, indeg, used, g);
}

// ============================================================
// 练习 5 答案：判断拓扑序是否唯一
//
// 结论：Kahn 过程中任何时刻队列里最多只有 1 个顶点 ⇔ 拓扑序唯一。
// 因为队列里若有 ≥2 个顶点，它们之间的先后可以任意交换，序列就不唯一。
// ============================================================
bool isUniqueTopoOrder(const Graph& g) {
    vector<int> indeg(g.n, 0);
    for (int u = 0; u < g.n; ++u)
        for (int v : g.adj[u])
            ++indeg[v];
    queue<int> q;
    for (int v = 0; v < g.n; ++v)
        if (indeg[v] == 0) q.push(v);

    int outCnt = 0;
    while (!q.empty()) {
        if (q.size() > 1) return false;   // 候选 ≥2 个，序列必不唯一
        int u = q.front(); q.pop();
        ++outCnt;
        for (int v : g.adj[u])
            if (--indeg[v] == 0) q.push(v);
    }
    // 有环时拓扑序根本不存在，谈不上"唯一"
    return outCnt == g.n;
}

// 打印一条拓扑序列（编号转字母）
void printOrder(const vector<int>& order) {
    for (size_t i = 0; i < order.size(); ++i) {
        if (i) cout << ' ';
        cout << vname(order[i]);
    }
    cout << '\n';
}

int main() {
    // ----------------------------------------------------------
    // 练习 1 的 DAG：6 个顶点 A~F，6 条边
    //   F→C, F→A, E→A, E→B, C→D, D→B
    // 编号映射：A=0 B=1 C=2 D=3 E=4 F=5
    // ----------------------------------------------------------
    Graph dag(6);
    dag.addEdge(5, 2);   // F→C
    dag.addEdge(5, 0);   // F→A
    dag.addEdge(4, 0);   // E→A
    dag.addEdge(4, 1);   // E→B
    dag.addEdge(2, 3);   // C→D
    dag.addEdge(3, 1);   // D→B

    // (1) Kahn 算法：先看入度表，再看拓扑序
    cout << "(1) Kahn 算法求拓扑序（6 顶点 DAG：F→C F→A E→A E→B C→D D→B）\n";
    {
        vector<int> indeg(6, 0);
        for (int u = 0; u < 6; ++u)
            for (int v : dag.adj[u]) ++indeg[v];
        cout << "    各顶点入度: ";
        for (int v = 0; v < 6; ++v)
            cout << vname(v) << '=' << indeg[v] << (v == 5 ? "" : " ");
        cout << '\n';
    }
    vector<int> order;
    bool ok = kahnTopoSort(dag, order);
    cout << "    拓扑序列: ";
    printOrder(order);
    cout << "    共输出 " << order.size() << "/6 个顶点，"
         << (ok ? "图无环\n" : "图存在环，无拓扑序！\n");

    // (2) 练习 2：有环图 A→B→C→A，看 Kahn 怎么"发现"环
    cout << "(2) 有环图检测（A→B, B→C, C→A）\n";
    Graph cyc(3);
    cyc.addEdge(0, 1);   // A→B
    cyc.addEdge(1, 2);   // B→C
    cyc.addEdge(2, 0);   // C→A
    ok = kahnTopoSort(cyc, order);
    cout << "    拓扑序列: ";
    printOrder(order);
    cout << "    共输出 " << order.size() << "/3 个顶点，"
         << (ok ? "图无环\n" : "图存在环，无拓扑序！\n");

    // (3) 练习 3：DFS 法求同一 DAG 的拓扑序
    cout << "(3) DFS 法求拓扑序（同一 6 顶点 DAG）\n";
    vector<int> dfsOrder = dfsTopoSort(dag);
    cout << "    拓扑序列: ";
    printOrder(dfsOrder);

    // (4) 练习 4：4 顶点小图 0→2, 1→2, 2→3 的拓扑序列个数
    cout << "(4) 回溯统计拓扑序列个数（4 顶点：0→2, 1→2, 2→3）\n";
    Graph small(4);
    small.addEdge(0, 2);
    small.addEdge(1, 2);
    small.addEdge(2, 3);
    cout << "    拓扑序列个数 = " << countTopoOrders(small) << '\n';

    // (5) 练习 5：拓扑序唯一性判断
    cout << "(5) 拓扑序唯一性判断\n";
    Graph chain(4);                 // 链 A→B→C→D：每步只有 1 个候选
    chain.addEdge(0, 1);
    chain.addEdge(1, 2);
    chain.addEdge(2, 3);
    cout << "    链 A→B→C→D: "
         << (isUniqueTopoOrder(chain) ? "唯一" : "不唯一") << '\n';
    cout << "    练习 1 的 DAG: "
         << (isUniqueTopoOrder(dag) ? "唯一" : "不唯一") << '\n';

    return 0;
}

/* ==================== 预期输出 ====================
(1) Kahn 算法求拓扑序（6 顶点 DAG：F→C F→A E→A E→B C→D D→B）
    各顶点入度: A=2 B=2 C=1 D=1 E=0 F=0
    拓扑序列: E F C A D B
    共输出 6/6 个顶点，图无环
(2) 有环图检测（A→B, B→C, C→A）
    拓扑序列: 
    共输出 0/3 个顶点，图存在环，无拓扑序！
(3) DFS 法求拓扑序（同一 6 顶点 DAG）
    拓扑序列: F E C D B A
(4) 回溯统计拓扑序列个数（4 顶点：0→2, 1→2, 2→3）
    拓扑序列个数 = 2
(5) 拓扑序唯一性判断
    链 A→B→C→D: 唯一
    练习 1 的 DAG: 不唯一
==================================================== */
