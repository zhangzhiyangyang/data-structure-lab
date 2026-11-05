// ============================================================
// 关键路径（Critical Path）—— AOE 网上的最长路
// 对应笔记：notes/2026-11-05-关键路径.md
// 编译：g++ -std=c++17 -Wall -Wextra -o ds_check 2026-11-05-critical_path.cpp
//
// 一句话：AOE 网里顶点是"事件"、有向边是"活动"、边权是工期；
//         从源点到汇点的最长路径就是关键路径，它的长度就是最短总工期。
// 本文件包含：
//   1) 用"边集数组 + 出边邻接表"建 AOE 网
//   2) Kahn 拓扑排序（顺带判环，AOE 网必须是 DAG）
//   3) 正推 ve（事件最早发生时间）、逆推 vl（事件最迟发生时间）
//   4) 扫一遍活动算 e / l / 时间余量 d，挑出关键活动
//   5) 在"关键子图"上 DFS 枚举全部关键路径
//   6) 暴力枚举所有源→汇路径做交叉验证（练习 2）
//   7) 缩短活动的试验（练习 3）、双关键路径示例（练习 4）、负余量自检（练习 5）
// ============================================================
#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;

// 顶点内部用 0..n-1 编号，打印时转成 v1..vn，方便和笔记里的图对上
string evName(int v) { return "v" + to_string(v + 1); }

// ============================================================
// 存储结构（详见笔记 ③）
// ============================================================
struct Edge {
    int from;   // 起点事件
    int to;     // 终点事件
    int w;      // 活动持续时间（工期）
};

struct AOE {
    int n;                      // 事件数 V
    vector<Edge> edges;         // 边集：按活动编号下标访问，算 e/l/余量最方便
    vector<vector<int>> out;    // 出边表：out[u] 存"从 u 出发的活动"在 edges 中的下标

    explicit AOE(int n_) : n(n_), out(n_) {}

    // 加一条活动 u → v（耗时 w），返回它的活动编号（即 edges 下标）
    int addActivity(int u, int v, int w) {
        edges.push_back({u, v, w});
        out[u].push_back(static_cast<int>(edges.size()) - 1);
        return static_cast<int>(edges.size()) - 1;
    }
};

// ============================================================
// 第 1 步：Kahn 拓扑排序（入度表 + 队列）
// 返回 true  = 图是 DAG，order 里是完整拓扑序；
// 返回 false = 有环，AOE 网非法（工程存在循环依赖，永远开不了工）。
// 复杂度 O(V + E)。
// ============================================================
bool topoOrder(const AOE& g, vector<int>& order) {
    order.clear();

    // 统计入度：对每条边的"终点"加 1（不要加在起点上，这是常见笔误）
    vector<int> indeg(g.n, 0);
    for (const Edge& e : g.edges) ++indeg[e.to];

    // 入度为 0 的事件（没有前驱活动）先入队 —— 这些就是源点
    queue<int> q;
    for (int v = 0; v < g.n; ++v)
        if (indeg[v] == 0) q.push(v);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);                     // 出队即确定顺序

        for (int eid : g.out[u]) {              // 遍历 u 的全部后继活动
            int v = g.edges[eid].to;
            if (--indeg[v] == 0) q.push(v);     // 只剩这条入边没算 → 可以入队了
        }
    }

    // 输出顶点数不足 n，说明剩下的顶点困在环里
    return static_cast<int>(order.size()) == g.n;
}

// ============================================================
// 第 2~3 步：正推 ve、逆推 vl
//   ve[j] = max{ ve[i] + w(i,j) }   按【拓扑序】推
//   vl[i] = min{ vl[j] - w(i,j) }   按【逆拓扑序】推，初值全取总工期 T
// total 返回总工期 = max(ve[*])（有非负权时必然在某个汇点取到）
// 返回 false 表示图有环或存在从源点不可达的事件，数据不可用。
// 复杂度 O(V + E)。
// ============================================================
bool computeVEVL(const AOE& g, vector<int>& ve, vector<int>& vl,
                 vector<int>& order, int& total) {
    if (!topoOrder(g, order)) return false;     // 有环，直接失败

    // ---- 正推：沿拓扑序把"我从这里出发能最晚到什么时候"往后传 ----
    ve.assign(g.n, 0);
    for (int u : order)
        for (int eid : g.out[u]) {
            const Edge& e = g.edges[eid];
            if (ve[u] + e.w > ve[e.to]) ve[e.to] = ve[u] + e.w;  // 取 max！
        }

    // ---- 总工期 T：所有事件最早时间的最大值 ----
    // 用 max(ve[*]) 而不是 ve[唯一汇点]，这样多汇点的图也自动正确
    total = *max_element(ve.begin(), ve.end());

    // ---- 逆推：所有 vl 先置成 T（汇点没有出边，会一直保持 T）----
    vl.assign(g.n, total);
    for (int i = static_cast<int>(order.size()) - 1; i >= 0; --i) {
        int u = order[i];
        for (int eid : g.out[u]) {
            const Edge& e = g.edges[eid];
            if (vl[e.to] - e.w < vl[u]) vl[u] = vl[e.to] - e.w;  // 取 min！
        }
    }
    return true;
}

// 活动的时间余量 d = l - e = (vl[to] - w) - ve[from]，恒 ≥ 0，等于 0 即关键活动
int slackOf(const Edge& e, const vector<int>& ve, const vector<int>& vl) {
    return vl[e.to] - e.w - ve[e.from];
}

// ============================================================
// 第 5 步：枚举全部关键路径
// 只在"关键子图"（余量为 0 的边）上走：u 是关键事件时，一定存在一条
// 满 ve[u] + w == vl[v] 的出边，所以沿它走必然能走到汇点。
// 复杂度 O(关键路径条数 × 路径长度)。
// ============================================================
void dfsCriticalPath(const AOE& g, int u, const vector<int>& ve,
                     const vector<int>& vl, vector<int>& path,
                     vector<vector<int>>& result) {
    bool isSink = g.out[u].empty();             // 没有出边 = 汇点 = 到达终点

    for (int eid : g.out[u]) {
        const Edge& e = g.edges[eid];
        // d == 0 等价于 ve[from] + w == vl[to]，这样判关键活动最省事
        if (ve[e.from] + e.w != vl[e.to]) continue;

        path.push_back(e.to);
        dfsCriticalPath(g, e.to, ve, vl, path, result);
        path.pop_back();                        // 回溯，换下一条关键出边
    }

    if (isSink) result.push_back(path);         // 走到了汇点，记下这一条完整路径
}

vector<vector<int>> allCriticalPaths(const AOE& g, const vector<int>& ve,
                                     const vector<int>& vl) {
    vector<vector<int>> result;
    vector<int> path;

    // 从所有源点（入度为 0，ve == 0）出发
    vector<int> indeg(g.n, 0);
    for (const Edge& e : g.edges) ++indeg[e.to];

    for (int s = 0; s < g.n; ++s) {
        if (indeg[s] != 0) continue;
        path.assign(1, s);
        dfsCriticalPath(g, s, ve, vl, path, result);
    }
    return result;
}

// ============================================================
// 练习 2 的暴力解法：枚举所有源→汇路径，返回 (路径, 长度) 列表
// 只适合小图（路径条数可能指数级），这里纯粹用来交叉验证 ve/关键路径。
// ============================================================
void dfsAllPaths(const AOE& g, int u, int target, int len, vector<int>& path,
                 vector<vector<int>>& paths, vector<int>& lens) {
    if (u == target) {                          // 到达汇点，收一条
        paths.push_back(path);
        lens.push_back(len);
        return;
    }
    for (int eid : g.out[u]) {
        const Edge& e = g.edges[eid];
        path.push_back(e.to);
        dfsAllPaths(g, e.to, target, len + e.w, path, paths, lens);
        path.pop_back();
    }
}

// 从源点出发做一次 BFS，数一数有多少事件可达（用来发现"孤立"事件）
int reachableCount(const AOE& g) {
    vector<int> indeg(g.n, 0);
    for (const Edge& e : g.edges) ++indeg[e.to];

    vector<bool> vis(g.n, false);
    queue<int> q;
    for (int v = 0; v < g.n; ++v)
        if (indeg[v] == 0) { q.push(v); vis[v] = true; }

    int cnt = 0;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        ++cnt;
        for (int eid : g.out[u]) {
            int v = g.edges[eid].to;
            if (!vis[v]) { vis[v] = true; q.push(v); }
        }
    }
    return cnt;
}

// ============================================================
// 打印工具
// ============================================================
// 把一组事件编号拼成 "v1 v2 v3"，避免逐个打印时留下行尾空格
string joinEvents(const vector<int>& vs) {
    string s;
    for (size_t i = 0; i < vs.size(); ++i) {
        if (i) s += " ";
        s += evName(vs[i]);
    }
    return s;
}

void printEventTable(const AOE& g, const vector<int>& ve, const vector<int>& vl) {
    for (int v = 0; v < g.n; ++v) {
        cout << "    " << evName(v) << ": ve=" << ve[v] << "  vl=" << vl[v];
        // ve == vl 的事件称为关键事件；关键活动一定连接两个关键事件
        if (ve[v] == vl[v]) cout << "  ← 关键事件";
        cout << "\n";
    }
}

// 教材经典 AOE 网：6 个事件、8 项活动
// a1 v1→v2(3) a2 v1→v3(2) a3 v2→v4(2) a4 v2→v5(3)
// a5 v3→v4(4) a6 v3→v6(3) a7 v4→v6(2) a8 v5→v6(1)
// a7w / a1w 做成参数，方便做"缩短活动"的试验（练习 3）
AOE makeExample1(int a7w = 2, int a1w = 3) {
    AOE g(6);
    g.addActivity(0, 1, a1w);   // a1
    g.addActivity(0, 2, 2);     // a2
    g.addActivity(1, 3, 2);     // a3
    g.addActivity(1, 4, 3);     // a4
    g.addActivity(2, 3, 4);     // a5
    g.addActivity(2, 5, 3);     // a6
    g.addActivity(3, 5, a7w);   // a7
    g.addActivity(4, 5, 1);     // a8
    return g;
}

// 只算工期，用于缩短活动的对比试验
int durationOf(const AOE& g) {
    vector<int> ve, vl, order;
    int total = -1;
    if (!computeVEVL(g, ve, vl, order, total)) return -1;
    return total;
}

int main() {
    // ================= (1) AOE 网结构 =================
    cout << "(1) AOE 网结构（6 个事件，8 项活动）\n";
    AOE g = makeExample1();

    // 入度 0 = 源点（工程开始），出度 0 = 汇点（工程结束）
    vector<int> indeg(g.n, 0);
    for (const Edge& e : g.edges) ++indeg[e.to];

    vector<int> sources, sinks;
    for (int v = 0; v < g.n; ++v) {
        if (indeg[v] == 0) sources.push_back(v);
        if (g.out[v].empty()) sinks.push_back(v);
    }
    cout << "    源点: " << joinEvents(sources) << "\n";
    cout << "    汇点: " << joinEvents(sinks) << "\n";

    static const char* actName[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8"};
    for (int i = 0; i < static_cast<int>(g.edges.size()); ++i) {
        const Edge& e = g.edges[i];
        cout << "    " << actName[i] << ": " << evName(e.from) << "→"
             << evName(e.to) << " 工期 " << e.w << " 天\n";
    }
    cout << "    可达性: " << reachableCount(g) << "/" << g.n
         << " 个事件从源点可达\n";

    // ================= (2) 拓扑序 + ve/vl =================
    cout << "(2) 拓扑序与事件时间（ve 最早 / vl 最迟）\n";
    vector<int> ve, vl, order;
    int total = 0;
    if (!computeVEVL(g, ve, vl, order, total)) {
        cout << "    图有环，无法计算！\n";
        return 1;
    }
    cout << "    拓扑序: " << joinEvents(order) << "\n";
    printEventTable(g, ve, vl);
    cout << "    总工期 T = ve[汇点] = " << total << " 天\n";

    // ================= (3) 活动参数 =================
    cout << "(3) 活动参数（e 最早开始 / l 最迟开始 / d 时间余量）\n";
    vector<int> criticalEdge;
    int minSlack = 1 << 30;
    for (int i = 0; i < static_cast<int>(g.edges.size()); ++i) {
        const Edge& e = g.edges[i];
        int early = ve[e.from];                 // e = 起点的最早时间
        int late = vl[e.to] - e.w;              // l = 终点的最迟时间 - 工期
        int d = late - early;                   // 余量，理论上恒 ≥ 0
        minSlack = min(minSlack, d);
        cout << "    " << actName[i] << " " << evName(e.from) << "→"
             << evName(e.to) << " w=" << e.w
             << "  e=" << early << " l=" << late << " d=" << d;
        if (d == 0) {
            cout << "  ← 关键活动";
            criticalEdge.push_back(i);
        }
        cout << "\n";
    }
    cout << "    关键活动共 " << static_cast<int>(criticalEdge.size()) << " 个:";
    for (int i : criticalEdge) cout << " " << actName[i];
    cout << "\n";

    // ================= (4) 关键路径 =================
    cout << "(4) 关键路径（长度应等于总工期 " << total << "）\n";
    vector<vector<int>> cps = allCriticalPaths(g, ve, vl);
    cout << "    共 " << static_cast<int>(cps.size()) << " 条\n";
    for (const vector<int>& p : cps) {
        cout << "    ";
        for (size_t i = 0; i < p.size(); ++i) {
            if (i) cout << "→";
            cout << evName(p[i]);
        }
        cout << "\n";
    }

    // ================= (5) 练习 2：暴力枚举交叉验证 =================
    cout << "(5) 练习 2：暴力枚举所有源→汇路径，验证'最长即关键路径'\n";
    vector<vector<int>> paths;
    vector<int> lens;
    vector<int> path{0};            // 路径从源点 v1（下标 0）开始
    // 枚举到汇点 v6（下标 5）的所有路径
    dfsAllPaths(g, 0, 5, 0, path, paths, lens);
    int bruteMax = 0;
    for (size_t i = 0; i < paths.size(); ++i) {
        cout << "    ";
        for (size_t k = 0; k < paths[i].size(); ++k) {
            if (k) cout << "→";
            cout << evName(paths[i][k]);
        }
        cout << "  长度 " << lens[i] << "\n";
        bruteMax = max(bruteMax, lens[i]);
    }
    cout << "    最长路径长度 = " << bruteMax << "，ve[汇点] = " << total
         << "，" << (bruteMax == total ? "一致" : "不一致！") << "\n";

    // ================= (6) 练习 3：缩短活动 =================
    cout << "(6) 练习 3：缩短活动对总工期的影响\n";
    cout << "    a7 2→1 天 : 工期 " << durationOf(makeExample1(1, 3)) << " 天\n";
    cout << "    a7 2→0 天 : 工期 " << durationOf(makeExample1(0, 3))
         << " 天（继续缩短已无收益，另一条路径顶住了）\n";
    cout << "    a1 3→2 天 : 工期 " << durationOf(makeExample1(2, 2))
         << " 天（a1 余量为 1，属非关键活动，白缩）\n";

    // ================= (7) 练习 4：双关键路径 =================
    cout << "(7) 练习 4：两条并行等长支路（4 事件 4 活动）\n";
    AOE g2(4);
    g2.addActivity(0, 1, 2);    // v1→v2
    g2.addActivity(0, 2, 2);    // v1→v3
    g2.addActivity(1, 3, 3);    // v2→v4
    g2.addActivity(2, 3, 3);    // v3→v4
    vector<int> ve2, vl2, order2;
    int total2 = 0;
    computeVEVL(g2, ve2, vl2, order2, total2);
    cout << "    总工期 = " << total2 << " 天\n";
    vector<vector<int>> cps2 = allCriticalPaths(g2, ve2, vl2);
    cout << "    关键路径共 " << static_cast<int>(cps2.size()) << " 条:\n";
    for (const vector<int>& p : cps2) {
        cout << "      ";
        for (size_t i = 0; i < p.size(); ++i) {
            if (i) cout << "→";
            cout << evName(p[i]);
        }
        cout << "\n";
    }
    // 只缩短其中一条支路：另一条仍是 5 天，工期不变
    AOE g3(4);
    g3.addActivity(0, 1, 2);
    g3.addActivity(0, 2, 2);
    g3.addActivity(1, 3, 2);    // v2→v4 由 3 缩到 2
    g3.addActivity(2, 3, 3);
    cout << "    只把 v2→v4 由 3 缩到 2 天 → 工期 " << durationOf(g3)
         << " 天（另一条支路仍是 5 天，必须两条一起缩）\n";

    // ================= (8) 自检与异常 =================
    cout << "(8) 自检与异常处理\n";
    cout << "    最小时间余量 = " << minSlack << "（"
         << (minSlack >= 0 ? "≥0，ve/vl 计算自洽" : "为负，说明算错了！") << "）\n";

    // 有环的"工程"：三项活动互相等待，永远开不了工
    AOE cyc(3);
    cyc.addActivity(0, 1, 1);
    cyc.addActivity(1, 2, 1);
    cyc.addActivity(2, 0, 1);
    vector<int> cycOrder;
    bool ok = topoOrder(cyc, cycOrder);
    cout << "    有环图 v1→v2→v3→v1: 拓扑排序"
         << (ok ? "成功（不应发生）" : "失败") << "，只输出 "
         << static_cast<int>(cycOrder.size()) << "/" << cyc.n
         << " 个事件 → AOE 网必须无环\n";

    return 0;
}

/* ==================== 预期输出 ====================
(1) AOE 网结构（6 个事件，8 项活动）
    源点: v1
    汇点: v6
    a1: v1→v2 工期 3 天
    a2: v1→v3 工期 2 天
    a3: v2→v4 工期 2 天
    a4: v2→v5 工期 3 天
    a5: v3→v4 工期 4 天
    a6: v3→v6 工期 3 天
    a7: v4→v6 工期 2 天
    a8: v5→v6 工期 1 天
    可达性: 6/6 个事件从源点可达
(2) 拓扑序与事件时间（ve 最早 / vl 最迟）
    拓扑序: v1 v2 v3 v5 v4 v6
    v1: ve=0  vl=0  ← 关键事件
    v2: ve=3  vl=4
    v3: ve=2  vl=2  ← 关键事件
    v4: ve=6  vl=6  ← 关键事件
    v5: ve=6  vl=7
    v6: ve=8  vl=8  ← 关键事件
    总工期 T = ve[汇点] = 8 天
(3) 活动参数（e 最早开始 / l 最迟开始 / d 时间余量）
    a1 v1→v2 w=3  e=0 l=1 d=1
    a2 v1→v3 w=2  e=0 l=0 d=0  ← 关键活动
    a3 v2→v4 w=2  e=3 l=4 d=1
    a4 v2→v5 w=3  e=3 l=4 d=1
    a5 v3→v4 w=4  e=2 l=2 d=0  ← 关键活动
    a6 v3→v6 w=3  e=2 l=5 d=3
    a7 v4→v6 w=2  e=6 l=6 d=0  ← 关键活动
    a8 v5→v6 w=1  e=6 l=7 d=1
    关键活动共 3 个: a2 a5 a7
(4) 关键路径（长度应等于总工期 8）
    共 1 条
    v1→v3→v4→v6
(5) 练习 2：暴力枚举所有源→汇路径，验证'最长即关键路径'
    v1→v2→v4→v6  长度 7
    v1→v2→v5→v6  长度 7
    v1→v3→v4→v6  长度 8
    v1→v3→v6  长度 5
    最长路径长度 = 8，ve[汇点] = 8，一致
(6) 练习 3：缩短活动对总工期的影响
    a7 2→1 天 : 工期 7 天
    a7 2→0 天 : 工期 7 天（继续缩短已无收益，另一条路径顶住了）
    a1 3→2 天 : 工期 8 天（a1 余量为 1，属非关键活动，白缩）
(7) 练习 4：两条并行等长支路（4 事件 4 活动）
    总工期 = 5 天
    关键路径共 2 条:
      v1→v2→v4
      v1→v3→v4
    只把 v2→v4 由 3 缩到 2 天 → 工期 5 天（另一条支路仍是 5 天，必须两条一起缩）
(8) 自检与异常处理
    最小时间余量 = 0（≥0，ve/vl 计算自洽）
    有环图 v1→v2→v3→v1: 拓扑排序失败，只输出 0/3 个事件 → AOE 网必须无环
==================================================== */
