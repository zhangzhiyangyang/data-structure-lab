// ============================================================
// 图（Graph）：邻接矩阵/邻接表存储、DFS、BFS、
//             最小生成树（Prim、Kruskal）、最短路（Dijkstra 两个版本）
// 对应笔记：notes/2026-10-21-图.md
// 编译：g++ -std=c++17 -Wall -Wextra -o graph 2026-10-21-graph.cpp
// ============================================================
#include <algorithm>
#include <array>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

using namespace std;

const int INF = 0x3f3f3f3f;   // 哨兵：两倍相加不溢出 int

// ---------- 存储结构一：邻接矩阵（顺序存储，适合稠密图） ----------
struct MGraph {
    int n;
    vector<vector<int>> g;                     // g[u][v]=边权，无边为 INF
    explicit MGraph(int n) : n(n), g(n, vector<int>(n, INF)) {
        for (int i = 0; i < n; ++i) g[i][i] = 0;   // 自己到自己为 0
    }
    void addEdge(int u, int v, int w) {        // 无向图：对称赋值
        g[u][v] = w;
        g[v][u] = w;
    }
};

// ---------- 存储结构二：邻接表（适合稀疏图） ----------
struct Edge {
    int to;
    int w;
};
struct ALGraph {
    int n;
    vector<vector<Edge>> adj;                  // adj[u] = u 的出边表
    explicit ALGraph(int n) : n(n), adj(n) {}
    void addEdge(int u, int v, int w) {        // 无向边存两份
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
    void addDirectedEdge(int u, int v) {       // 有向边只存一份（拓扑排序用）
        adj[u].push_back({v, 1});
    }
};

// ---------- DFS（递归）：vis 防止沿环死循环 ----------
void dfs(const ALGraph& G, int u, vector<bool>& vis, vector<int>& order) {
    vis[u] = true;
    order.push_back(u);
    for (const Edge& e : G.adj[u])
        if (!vis[e.to]) dfs(G, e.to, vis, order);
}

// ---------- BFS（队列）：入队时立刻标记，防止重复入队 ----------
vector<int> bfs(const ALGraph& G, int s) {
    vector<bool> vis(G.n, false);
    vector<int> order;
    queue<int> q;
    q.push(s);
    vis[s] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (const Edge& e : G.adj[u])
            if (!vis[e.to]) {                  // 先标记再入队！
                vis[e.to] = true;
                q.push(e.to);
            }
    }
    return order;
}

// ---------- Prim O(n^2)：加点法，返回 MST 总权值，不连通返回 -1 ----------
int prim(const MGraph& G, vector<int>& parent) {
    int n = G.n;
    vector<int> low(n, INF);                   // low[v]: v 到当前树的最小边权
    vector<bool> in(n, false);
    parent.assign(n, -1);
    low[0] = 0;
    int total = 0;
    for (int it = 0; it < n; ++it) {
        int u = -1;                            // 挑树外 low 最小的点
        for (int v = 0; v < n; ++v)
            if (!in[v] && (u == -1 || low[v] < low[u])) u = v;
        if (u == -1 || low[u] == INF) return -1;   // 图不连通
        in[u] = true;
        total += low[u];
        for (int v = 0; v < n; ++v)            // 用新点松弛 low（只动树外点）
            if (!in[v] && G.g[u][v] < low[v]) {
                low[v] = G.g[u][v];
                parent[v] = u;
            }
    }
    return total;
}

// ---------- 并查集（路径压缩）：Kruskal 判成环、统计连通分量都要用 ----------
struct DSU {
    vector<int> f;
    explicit DSU(int n) : f(n) {
        for (int i = 0; i < n; ++i) f[i] = i;
    }
    int find(int x) { return f[x] == x ? x : f[x] = find(f[x]); }  // 路径压缩
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;              // 已连通：选这条边会成环
        f[a] = b;
        return true;
    }
};

// ---------- Kruskal O(e log e)：加边法，适合稀疏图 ----------
int kruskal(int n, vector<array<int, 3>> edges, vector<pair<int, int>>& mst) {
    // 边格式 {u, v, w}，按权值 e[2] 升序排序（stable_sort 保证同权时顺序确定）
    stable_sort(edges.begin(), edges.end(),
                [](const array<int, 3>& a, const array<int, 3>& b) { return a[2] < b[2]; });
    DSU d(n);
    int total = 0, cnt = 0;
    mst.clear();
    for (const auto& e : edges) {
        if (d.unite(e[0], e[1])) {
            total += e[2];
            mst.push_back({min(e[0], e[1]), max(e[0], e[1])});
            if (++cnt == n - 1) break;         // 选够 n-1 条即完成
        }
    }
    return cnt == n - 1 ? total : -1;          // 不连通则无生成树
}

// ---------- Dijkstra O(n^2)：邻接矩阵版，要求边权非负 ----------
vector<int> dijkstra(const MGraph& G, int s) {
    int n = G.n;
    vector<int> dist(n, INF);
    vector<bool> done(n, false);
    dist[s] = 0;
    for (int it = 0; it < n; ++it) {
        int u = -1;                            // 选未确定点中 dist 最小者
        for (int v = 0; v < n; ++v)
            if (!done[v] && (u == -1 || dist[v] < dist[u])) u = v;
        if (u == -1 || dist[u] == INF) break;  // 剩余点不可达，及时退出
        done[u] = true;
        for (int v = 0; v < n; ++v)
            if (G.g[u][v] != INF && G.g[u][v] > 0 && dist[u] + G.g[u][v] < dist[v])
                dist[v] = dist[u] + G.g[u][v];
    }
    return dist;
}

// ---------- Dijkstra 堆优化 O((n+e)logn)：邻接表 + 小根堆懒删除 ----------
vector<int> dijkstraHeap(const ALGraph& G, int s) {
    vector<int> dist(G.n, INF);
    dist[s] = 0;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    pq.push({0, s});
    while (!pq.empty()) {
        pair<int, int> top = pq.top();
        pq.pop();
        int d = top.first, u = top.second;
        if (d > dist[u]) continue;             // 过期条目（懒删除）
        for (const Edge& e : G.adj[u])
            if (dist[u] + e.w < dist[e.to]) {
                dist[e.to] = dist[u] + e.w;
                pq.push({dist[e.to], e.to});
            }
    }
    return dist;
}

// ---------- 练习1：判断无向图是否为一棵树（n-1 条边 且 连通） ----------
bool isTree(const ALGraph& G) {
    int edges = 0;                             // 无向边存两份，总数除以 2
    for (int u = 0; u < G.n; ++u) edges += (int)G.adj[u].size();
    if (edges / 2 != G.n - 1) return false;
    return (int)bfs(G, 0).size() == G.n;       // 从 0 出发能覆盖全部顶点
}

// ---------- 练习2：BFS 统计无权图单源最短路长度与条数 ----------
void bfsCountPaths(const ALGraph& G, int s, vector<int>& dist, vector<long long>& cnt) {
    dist.assign(G.n, -1);
    cnt.assign(G.n, 0);
    queue<int> q;
    dist[s] = 0;
    cnt[s] = 1;
    q.push(s);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const Edge& e : G.adj[u]) {
            if (dist[e.to] == -1) {            // 首次到达：距离+1，条数继承
                dist[e.to] = dist[u] + 1;
                cnt[e.to] = cnt[u];
                q.push(e.to);
            } else if (dist[e.to] == dist[u] + 1) {
                cnt[e.to] += cnt[u];           // 又发现一条等长最短路
            }
        }
    }
}

// ---------- 练习3：统计连通分量个数（并查集） ----------
int countComponents(const ALGraph& G) {
    DSU d(G.n);
    for (int u = 0; u < G.n; ++u)
        for (const Edge& e : G.adj[u])
            if (e.to > u) d.unite(u, e.to);    // 每条边只合并一次
    int c = 0;
    for (int i = 0; i < G.n; ++i)
        if (d.find(i) == i) ++c;               // 根的个数 = 分量个数
    return c;
}

// ---------- 练习4：Kahn 算法拓扑排序，有环则返回空序列 ----------
vector<int> topoSort(const ALGraph& G) {
    vector<int> indeg(G.n, 0);
    for (int u = 0; u < G.n; ++u)
        for (const Edge& e : G.adj[u]) ++indeg[e.to];
    queue<int> q;
    for (int i = 0; i < G.n; ++i)
        if (indeg[i] == 0) q.push(i);
    vector<int> order;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (const Edge& e : G.adj[u])
            if (--indeg[e.to] == 0) q.push(e.to);
    }
    if ((int)order.size() != G.n) return {};   // 出队数 < n：存在环
    return order;
}

// ---------- 输出辅助 ----------
void printVec(const vector<int>& v, const char* label) {
    cout << label << ": ";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ' ';
        cout << v[i];
    }
    cout << '\n';
}

int main() {
    // 教材经典例图：6 顶点 10 边（严蔚敏《数据结构》求 MST 的例图）
    vector<array<int, 3>> es = {
        {0, 1, 6}, {0, 2, 1}, {0, 3, 5}, {1, 2, 5}, {1, 4, 3},
        {2, 3, 5}, {2, 4, 6}, {2, 5, 4}, {3, 5, 2}, {4, 5, 6}};

    cout << "===== 存储结构与遍历 =====\n";
    ALGraph G(6);
    for (const auto& e : es) G.addEdge(e[0], e[1], e[2]);

    vector<bool> vis(G.n, false);
    vector<int> dOrder;
    dfs(G, 0, vis, dOrder);
    printVec(dOrder, "DFS(从0, 递归)");
    printVec(bfs(G, 0), "BFS(从0)      ");

    cout << "\n===== 最小生成树 =====\n";
    MGraph MG(6);
    for (const auto& e : es) MG.addEdge(e[0], e[1], e[2]);

    vector<int> parent;
    int pw = prim(MG, parent);
    cout << "Prim 总权值: " << pw << "\n";
    cout << "Prim 选边: ";
    for (int v = 0; v < MG.n; ++v)
        if (parent[v] != -1)
            cout << "(" << min(parent[v], v) << "-" << max(parent[v], v) << ") ";
    cout << "\n";

    vector<pair<int, int>> mst;
    int kw = kruskal(MG.n, es, mst);
    cout << "Kruskal 总权值: " << kw << "\n";
    cout << "Kruskal 选边: ";
    for (const auto& p : mst) cout << p.first << "-" << p.second << " ";
    cout << "\n";

    cout << "\n===== 最短路(Dijkstra, 源点0) =====\n";
    printVec(dijkstra(MG, 0), "O(n^2)版 ");
    printVec(dijkstraHeap(G, 0), "堆优化版 ");

    cout << "\n===== 练习1: 判断无向图是否为树 =====\n";
    ALGraph T(5);
    T.addEdge(0, 1, 1);
    T.addEdge(1, 2, 1);
    T.addEdge(1, 3, 1);
    T.addEdge(3, 4, 1);
    cout << "图1(5顶点4边): " << (isTree(T) ? "是一棵树" : "不是树") << "\n";
    cout << "图2(6顶点10边): " << (isTree(G) ? "是一棵树" : "不是树") << "\n";

    cout << "\n===== 练习2: BFS 最短路长度与条数(无权图, 源点0) =====\n";
    ALGraph U(6);
    U.addEdge(0, 1, 1);
    U.addEdge(0, 2, 1);
    U.addEdge(1, 3, 1);
    U.addEdge(2, 3, 1);
    U.addEdge(2, 4, 1);
    U.addEdge(3, 5, 1);
    U.addEdge(4, 5, 1);
    vector<int> ud;
    vector<long long> uc;
    bfsCountPaths(U, 0, ud, uc);
    for (int v = 1; v < U.n; ++v)
        cout << "到" << v << ": 长度" << ud[v] << " 条数" << uc[v] << "\n";

    cout << "\n===== 练习3: 连通分量个数 =====\n";
    ALGraph C(8);
    C.addEdge(0, 1, 1);
    C.addEdge(1, 2, 1);
    C.addEdge(3, 4, 1);
    C.addEdge(5, 6, 1);
    cout << "连通分量个数: " << countComponents(C) << "\n";

    cout << "\n===== 练习4: 拓扑排序(Kahn) =====\n";
    ALGraph D(5);
    D.addDirectedEdge(0, 1);
    D.addDirectedEdge(0, 2);
    D.addDirectedEdge(1, 3);
    D.addDirectedEdge(2, 3);
    D.addDirectedEdge(3, 4);
    vector<int> topo = topoSort(D);
    if (topo.empty()) {
        cout << "DAG 图中: 存在环，无法完成拓扑排序\n";
    } else {
        cout << "DAG 图拓扑序列: ";
        for (size_t i = 0; i < topo.size(); ++i) cout << topo[i] << ' ';
        cout << "\n";
    }

    ALGraph E(3);                              // 有环：0->1->2->0
    E.addDirectedEdge(0, 1);
    E.addDirectedEdge(1, 2);
    E.addDirectedEdge(2, 0);
    topo = topoSort(E);
    if (topo.empty()) cout << "有环图中: 存在环，无法完成拓扑排序\n";
    else {
        cout << "有环图拓扑序列: ";
        for (int x : topo) cout << x << ' ';
        cout << "\n";
    }

    return 0;
}

/* ====================== 预期输出 ======================
===== 存储结构与遍历 =====
DFS(从0, 递归): 0 1 2 3 5 4
BFS(从0)      : 0 1 2 3 4 5

===== 最小生成树 =====
Prim 总权值: 15
Prim 选边: (1-2) (0-2) (3-5) (1-4) (2-5)
Kruskal 总权值: 15
Kruskal 选边: 0-2 3-5 1-4 2-5 1-2

===== 最短路(Dijkstra, 源点0) =====
O(n^2)版 : 0 6 1 5 7 5
堆优化版 : 0 6 1 5 7 5

===== 练习1: 判断无向图是否为树 =====
图1(5顶点4边): 是一棵树
图2(6顶点10边): 不是树

===== 练习2: BFS 最短路长度与条数(无权图, 源点0) =====
到1: 长度1 条数1
到2: 长度1 条数1
到3: 长度2 条数2
到4: 长度2 条数1
到5: 长度3 条数3

===== 练习3: 连通分量个数 =====
连通分量个数: 4

===== 练习4: 拓扑排序(Kahn) =====
DAG 图拓扑序列: 0 1 2 3 4
有环图中: 存在环，无法完成拓扑排序
====================================================== */
