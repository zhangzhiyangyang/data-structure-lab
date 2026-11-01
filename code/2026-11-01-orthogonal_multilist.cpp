// ============================================================
// 图的链式存储进阶：十字链表（有向图）与邻接多重表（无向图）
// 对应笔记：notes/2026-11-01-图的十字链表和邻接多重表.md
// 编译：g++ -std=c++17 -Wall -Wextra -o ds_check 2026-11-01-orthogonal_multilist.cpp
//
// 一句话区别：
//   十字链表  = 邻接表 + 逆邻接表 合体，一条弧同时挂在"出边链"和"入边链"上，
//               所以求入度从 O(n+e) 变成 O(入度)。只用于有向图。
//   邻接多重表 = 无向图的边只存一个结点，两个端点共享，删边不用改两处。
// ============================================================
#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;

// ============================================================
// 第一部分：十字链表（Orthogonal List）—— 有向图
//
//   顶点结点 VexNode： name | firstin | firstout
//   弧结点   ArcBox ： tail | head | weight | hlink | tlink
//
//   一条弧 <u,v> 会被同时挂到两条链上：
//     · u 的出边链：从 VexNode[u].firstout 出发，沿 tlink 走
//     · v 的入边链：从 VexNode[v].firstin  出发，沿 hlink 走
//   两条链像十字一样交叉在这条弧上，故名"十字链表"。
// ============================================================
struct ArcBox {
    int tail;       // 弧尾顶点下标
    int head;       // 弧头顶点下标
    int weight;     // 权值
    ArcBox* hlink;  // 指向"弧头相同"的下一条弧（即入边链的后继）
    ArcBox* tlink;  // 指向"弧尾相同"的下一条弧（即出边链的后继）

    ArcBox(int t, int h, int w)
        : tail(t), head(h), weight(w), hlink(nullptr), tlink(nullptr) {}
};

struct VexNode {
    string name;        // 顶点信息
    ArcBox* firstin;    // 指向第一条以该顶点为弧头的弧
    ArcBox* firstout;   // 指向第一条以该顶点为弧尾的弧

    explicit VexNode(string n) : name(std::move(n)), firstin(nullptr), firstout(nullptr) {}
};

class OrthoList {
public:
    int n;                       // 顶点数
    vector<VexNode> vex;         // 顶点数组：顺序存顶点，链式存弧
    vector<ArcBox*> arcs;        // 仅用于统一释放内存，不参与算法

    explicit OrthoList(const vector<string>& names) : n(static_cast<int>(names.size())) {
        for (const string& s : names) vex.emplace_back(s);
    }
    ~OrthoList() {
        for (ArcBox* a : arcs) delete a;
    }
    // 存裸指针的类禁止拷贝，避免二次释放
    OrthoList(const OrthoList&) = delete;
    OrthoList& operator=(const OrthoList&) = delete;

    // ---------- 插入一条弧 <u,v>：头插法，O(1) ----------
    void addArc(int u, int v, int w = 1) {
        ArcBox* a = new ArcBox(u, v, w);
        a->tlink = vex[u].firstout;      // 挂到 u 的出边链（头插）
        vex[u].firstout = a;
        a->hlink = vex[v].firstin;       // 同时挂到 v 的入边链（头插）
        vex[v].firstin = a;
        arcs.push_back(a);
    }

    int arcCount() const { return static_cast<int>(arcs.size()); }

    // ---------- 出度：只走自己的一条链，O(出度) ----------
    int outDegree(int u) const {
        int c = 0;
        for (ArcBox* a = vex[u].firstout; a != nullptr; a = a->tlink) ++c;
        return c;
    }

    // ---------- 入度：邻接表要扫全表，这里同样 O(入度) ----------
    int inDegree(int u) const {
        int c = 0;
        for (ArcBox* a = vex[u].firstin; a != nullptr; a = a->hlink) ++c;
        return c;
    }

    // ---------- 查找弧 <u,v>，O(出度) ----------
    ArcBox* findArc(int u, int v) const {
        for (ArcBox* a = vex[u].firstout; a != nullptr; a = a->tlink)
            if (a->head == v) return a;
        return nullptr;
    }

    // ---------- 删除弧 <u,v>：两条链各摘一次 ----------
    bool removeArc(int u, int v) {
        // 1) 先在 u 的出边链上找到它（顺带记下前驱）
        ArcBox* prev = nullptr;
        ArcBox* cur = vex[u].firstout;
        while (cur != nullptr && cur->head != v) {
            prev = cur;
            cur = cur->tlink;
        }
        if (cur == nullptr) return false;                 // 不存在这条弧

        if (prev != nullptr) prev->tlink = cur->tlink;    // 摘链：跳过 cur
        else vex[u].firstout = cur->tlink;

        // 2) 再在 v 的入边链上摘掉同一个结点（按指针找，不按数据找）
        prev = nullptr;
        ArcBox* c2 = vex[v].firstin;
        while (c2 != nullptr && c2 != cur) {
            prev = c2;
            c2 = c2->hlink;
        }
        if (prev != nullptr) prev->hlink = c2->hlink;
        else vex[v].firstin = c2->hlink;

        // 3) 释放
        arcs.erase(find(arcs.begin(), arcs.end(), cur));
        delete cur;
        return true;
    }

    // ---------- DFS：沿出边链深入，O(n+e) ----------
    void dfsRec(int u, vector<bool>& vis, vector<int>& order) const {
        vis[u] = true;
        order.push_back(u);
        for (ArcBox* a = vex[u].firstout; a != nullptr; a = a->tlink)
            if (!vis[a->head]) dfsRec(a->head, vis, order);
    }
    vector<int> dfsFrom(int s) const {
        vector<bool> vis(n, false);
        vector<int> order;
        dfsRec(s, vis, order);
        return order;
    }

    // ---------- BFS：队列 + 入队即标记，O(n+e) ----------
    vector<int> bfsFrom(int s) const {
        vector<bool> vis(n, false);
        vector<int> order;
        queue<int> q;
        vis[s] = true;
        q.push(s);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (ArcBox* a = vex[u].firstout; a != nullptr; a = a->tlink) {
                if (!vis[a->head]) {
                    vis[a->head] = true;
                    q.push(a->head);
                }
            }
        }
        return order;
    }

    // ---------- 转置（求逆图）：十字链表的独门绝技，O(n+e) ----------
    // 思路：不用重建任何结点。
    //   ① 每个顶点的 firstin / firstout 互换 → 出边表变成原来的入边表
    //   ② 每条弧的 hlink / tlink 互换、tail / head 互换 → 链的方向也对上了
    void transpose() {
        for (VexNode& vn : vex) swap(vn.firstin, vn.firstout);
        for (ArcBox* a : arcs) {
            swap(a->tail, a->head);
            swap(a->hlink, a->tlink);
        }
    }

    // ---------- 无向意义下的 DFS（出边、入边都当邻边），用于判连通 ----------
    void dfsUnderlying(int v, vector<bool>& vis) const {
        vis[v] = true;
        for (ArcBox* a = vex[v].firstout; a != nullptr; a = a->tlink)
            if (!vis[a->head]) dfsUnderlying(a->head, vis);
        for (ArcBox* a = vex[v].firstin; a != nullptr; a = a->hlink)
            if (!vis[a->tail]) dfsUnderlying(a->tail, vis);
    }

    // ---------- 有向图欧拉回路判定 ----------
    // 条件：① 每个顶点入度 = 出度；② 去掉孤立点后底图连通
    bool isEulerian() const {
        for (int v = 0; v < n; ++v)
            if (inDegree(v) != outDegree(v)) return false;

        int start = -1;
        for (int v = 0; v < n; ++v)
            if (inDegree(v) + outDegree(v) > 0) { start = v; break; }
        if (start == -1) return true;             // 没有边，约定成立

        vector<bool> vis(n, false);
        dfsUnderlying(start, vis);
        for (int v = 0; v < n; ++v)
            if (inDegree(v) + outDegree(v) > 0 && !vis[v]) return false;   // 底图断开
        return true;
    }

    // ---------- 打印 ----------
    void printOutLists() const {
        cout << "出边表（firstout → tlink）：" << endl;
        for (int u = 0; u < n; ++u) {
            cout << "  " << vex[u].name << " ->";
            if (vex[u].firstout == nullptr) cout << " (空)";
            for (ArcBox* a = vex[u].firstout; a != nullptr; a = a->tlink)
                cout << " " << vex[a->head].name << "(" << a->weight << ")";
            cout << endl;
        }
    }
    void printInLists() const {
        cout << "入边表（firstin → hlink）：" << endl;
        for (int u = 0; u < n; ++u) {
            cout << "  " << vex[u].name << " <-";
            if (vex[u].firstin == nullptr) cout << " (空)";
            for (ArcBox* a = vex[u].firstin; a != nullptr; a = a->hlink)
                cout << " " << vex[a->tail].name << "(" << a->weight << ")";
            cout << endl;
        }
    }
    void printOrder(const vector<int>& order) const {
        for (size_t i = 0; i < order.size(); ++i) {
            if (i) cout << " ";
            cout << vex[order[i]].name;
        }
    }
};

// ============================================================
// 第二部分：邻接多重表（Adjacency Multilist）—— 无向图
//
//   顶点结点 MVexNode： name | firstedge
//   边结点   EdgeBox ： ivex | jvex | weight | mark | ilink | jlink
//
//   一条边 (i,j) 只生成一个结点，同时挂到两条链上：
//     · 依附于 ivex 的边链：沿 ilink 走
//     · 依附于 jvex 的边链：沿 jlink 走
//   于是"无向图有 e 条边"就只需要 e 个边结点，而邻接表需要 2e 个。
//   mark 字段是邻接多重表的传统设计，用来标记"这条边是否走过"，
//   求欧拉路径 / 打印每条边恰好一次时非常方便。
// ============================================================
struct EdgeBox {
    int ivex;        // 端点一（约定放编号小的）
    int jvex;        // 端点二（约定放编号大的）
    int weight;      // 权值
    bool mark;       // 访问标记：这条边是否已被走过
    EdgeBox* ilink;  // 下一条依附于 ivex 的边
    EdgeBox* jlink;  // 下一条依附于 jvex 的边

    EdgeBox(int i, int j, int w)
        : ivex(i), jvex(j), weight(w), mark(false), ilink(nullptr), jlink(nullptr) {}
};

struct MVexNode {
    string name;
    EdgeBox* firstedge;
    explicit MVexNode(string n) : name(std::move(n)), firstedge(nullptr) {}
};

class AMList {
public:
    int n;
    vector<MVexNode> vex;
    vector<EdgeBox*> edges;      // 全部边结点，便于计数与释放

    explicit AMList(const vector<string>& names) : n(static_cast<int>(names.size())) {
        for (const string& s : names) vex.emplace_back(s);
    }
    ~AMList() {
        for (EdgeBox* e : edges) delete e;
    }
    AMList(const AMList&) = delete;
    AMList& operator=(const AMList&) = delete;

    // 把一个边结点头插进顶点 v 的链：看 v 是 ivex 还是 jvex，决定用哪个指针
    static void attach(MVexNode& vn, int v, EdgeBox* e) {
        if (e->ivex == v) e->ilink = vn.firstedge;
        else              e->jlink = vn.firstedge;
        vn.firstedge = e;
    }

    // ---------- 插入一条无向边：只 new 一个结点，O(1) ----------
    void addEdge(int u, int v, int w = 1) {
        if (u == v) return;                          // 自环在多重表里没有意义，忽略
        int i = min(u, v);
        int j = max(u, v);                           // 约定 ivex < jvex，便于统一判断
        EdgeBox* e = new EdgeBox(i, j, w);
        attach(vex[u], u, e);                        // 挂到 u 的链
        attach(vex[v], v, e);                        // 同一条边再挂到 v 的链（不同字段）
        edges.push_back(e);
    }

    int edgeCount() const { return static_cast<int>(edges.size()); }

    // ---------- 度：走完 v 的链即可，O(度) ----------
    int degree(int v) const {
        int c = 0;
        for (EdgeBox* e = vex[v].firstedge; e != nullptr;) {
            ++c;
            e = (e->ivex == v) ? e->ilink : e->jlink;   // 沿"依附于 v"的那条链走
        }
        return c;
    }

    // 取边 e 上除 v 之外的另一个端点
    static int other(const EdgeBox* e, int v) {
        return (e->ivex == v) ? e->jvex : e->ivex;
    }

    EdgeBox* findEdge(int u, int v) const {
        for (EdgeBox* e = vex[u].firstedge; e != nullptr;) {
            if (other(e, u) == v) return e;
            e = (e->ivex == u) ? e->ilink : e->jlink;
        }
        return nullptr;
    }

    // 从顶点 v 的链上摘掉 target（需要找前驱，O(度)）
    static void unlink(MVexNode& vn, int v, EdgeBox* target) {
        EdgeBox* prev = nullptr;
        EdgeBox* cur = vn.firstedge;
        while (cur != nullptr && cur != target) {
            prev = cur;
            cur = (cur->ivex == v) ? cur->ilink : cur->jlink;
        }
        if (cur == nullptr) return;
        EdgeBox* nxt = (cur->ivex == v) ? cur->ilink : cur->jlink;
        if (prev == nullptr) vn.firstedge = nxt;
        else if (prev->ivex == v) prev->ilink = nxt;
        else prev->jlink = nxt;
    }

    // ---------- 删除边 (u,v)：边只有一个结点，不会出现"两处不一致" ----------
    bool removeEdge(int u, int v) {
        EdgeBox* e = findEdge(u, v);
        if (e == nullptr) return false;
        unlink(vex[u], u, e);
        unlink(vex[v], v, e);
        edges.erase(find(edges.begin(), edges.end(), e));
        delete e;
        return true;
    }

    // ---------- 连通分量个数：普通顶点 DFS ----------
    void dfsVertex(int v, vector<bool>& vis) const {
        vis[v] = true;
        for (EdgeBox* e = vex[v].firstedge; e != nullptr;) {
            int w = other(e, v);
            if (!vis[w]) dfsVertex(w, vis);
            e = (e->ivex == v) ? e->ilink : e->jlink;
        }
    }
    int countComponents() const {
        vector<bool> vis(n, false);
        int comp = 0;
        for (int s = 0; s < n; ++s) {
            if (vis[s]) continue;
            ++comp;
            dfsVertex(s, vis);
        }
        return comp;
    }

    // ---------- 用 mark 字段：DFS 时每条边只走一次 ----------
    void dfsEdgeRec(int v, vector<bool>& vis, int& treeCnt) const {
        vis[v] = true;
        for (EdgeBox* e = vex[v].firstedge; e != nullptr;) {
            EdgeBox* nxt = (e->ivex == v) ? e->ilink : e->jlink;
            if (!e->mark) {                       // 这条边还没走过
                e->mark = true;                   // 立刻标记，防止别的路径重复走
                int w = other(e, v);
                if (!vis[w]) {                    // 通向未访问顶点 → 生成树的树边
                    cout << vex[v].name << "-" << vex[w].name << " ";
                    ++treeCnt;
                    dfsEdgeRec(w, vis, treeCnt);
                }
            }
            e = nxt;
        }
    }
    void dfsPrintEdges(int s) const {
        vector<bool> vis(n, false);
        int treeCnt = 0;
        cout << "DFS 树边：";
        dfsEdgeRec(s, vis, treeCnt);
        cout << endl;
        cout << "树边条数 = " << treeCnt << "（n-1 = " << n - 1 << "）" << endl;
        int marked = 0;
        for (EdgeBox* e : edges)
            if (e->mark) ++marked;
        cout << "带 mark 的边 = " << marked << " / " << edgeCount()
             << "（每条边恰好被访问一次）" << endl;
        // 恢复 mark，方便后面继续使用
        for (EdgeBox* e : edges) e->mark = false;
    }

    bool allDegreeEven() const {
        for (int v = 0; v < n; ++v)
            if (degree(v) % 2 != 0) return false;
        return true;
    }

    // ---------- 打印邻接链 ----------
    void printAdjacency() const {
        cout << "邻接链（一条边一个结点，两个端点共用）：" << endl;
        for (int v = 0; v < n; ++v) {
            cout << "  " << vex[v].name << ":";
            if (vex[v].firstedge == nullptr) cout << " (空)";
            for (EdgeBox* e = vex[v].firstedge; e != nullptr;) {
                cout << " " << vex[other(e, v)].name << "(" << e->weight << ")";
                e = (e->ivex == v) ? e->ilink : e->jlink;
            }
            cout << "   度=" << degree(v) << endl;
        }
    }
};

int main() {
    // ========================================================
    // (1) 建立一张有向图（5 个顶点，6 条弧，E 是孤立点）
    //      A->B(6)  A->C(3)  B->C(2)  B->D(5)  C->D(4)  D->A(1)
    // ========================================================
    OrthoList g({"A", "B", "C", "D", "E"});
    g.addArc(0, 1, 6);
    g.addArc(0, 2, 3);
    g.addArc(1, 2, 2);
    g.addArc(1, 3, 5);
    g.addArc(2, 3, 4);
    g.addArc(3, 0, 1);

    cout << "=== (1) 十字链表：建立有向图 ===" << endl;
    cout << "顶点数 " << g.n << "，弧数 " << g.arcCount() << endl;
    g.printOutLists();
    g.printInLists();

    // ========================================================
    // (2) 出度 / 入度统计：入度也能 O(入度) 求出，这是十字链表的意义
    // ========================================================
    cout << endl << "=== (2) 出度与入度 ===" << endl;
    int sumOut = 0, sumIn = 0, zeroOut = 0, zeroIn = 0;
    for (int v = 0; v < g.n; ++v) {
        int o = g.outDegree(v);
        int i = g.inDegree(v);
        sumOut += o;
        sumIn += i;
        if (o == 0) ++zeroOut;
        if (i == 0) ++zeroIn;
        cout << "  " << g.vex[v].name << "：出度=" << o << " 入度=" << i << endl;
    }
    cout << "出度总和=" << sumOut << "，入度总和=" << sumIn
         << "（都应等于弧数 " << g.arcCount() << "）" << endl;
    cout << "出度为 0 的顶点数=" << zeroOut << "，入度为 0 的顶点数=" << zeroIn
         << "  ← 练习 1" << endl;

    // ========================================================
    // (3) 查弧：只走 u 的出边链
    // ========================================================
    cout << endl << "=== (3) 查找弧 ===" << endl;
    for (int k = 0; k < 2; ++k) {
        int u = (k == 0) ? 0 : 2;
        int v = (k == 0) ? 2 : 1;
        ArcBox* a = g.findArc(u, v);
        cout << "  " << g.vex[u].name << "->" << g.vex[v].name << "：";
        if (a != nullptr) cout << "存在，权值 " << a->weight << endl;
        else cout << "不存在" << endl;
    }

    // ========================================================
    // (4) 有向图欧拉回路判定 ← 练习 2
    // ========================================================
    cout << endl << "=== (4) 欧拉回路判定 ===" << endl;
    cout << "  主图（A~E，E 为孤立点）：";
    cout << (g.isEulerian() ? "存在欧拉回路" : "不存在欧拉回路");
    cout << "（A 出度 2、入度 1，不满足入度=出度）" << endl;

    OrthoList ring({"A", "B", "C"});
    ring.addArc(0, 1, 1);
    ring.addArc(1, 2, 1);
    ring.addArc(2, 0, 1);
    cout << "  有向环 A->B->C->A：";
    cout << (ring.isEulerian() ? "存在欧拉回路" : "不存在欧拉回路") << endl;

    // ========================================================
    // (5) 转置：O(n+e) 求逆图，不用重建任何结点
    // ========================================================
    cout << endl << "=== (5) 转置（求逆图）===" << endl;
    vector<int> oldIn(g.n);
    vector<int> oldOut(g.n);
    for (int v = 0; v < g.n; ++v) {
        oldIn[v] = g.inDegree(v);
        oldOut[v] = g.outDegree(v);
    }
    g.transpose();
    g.printOutLists();
    bool same = true;
    for (int v = 0; v < g.n; ++v)
        if (g.outDegree(v) != oldIn[v] || g.inDegree(v) != oldOut[v]) same = false;
    cout << "校验：转置后「出度=原入度、入度=原出度」→ "
         << (same ? "一致" : "不一致") << endl;
    g.transpose();                          // 转回去，后面的操作仍针对原图
    cout << "已转置回原图，A 的出边表恢复为：";
    for (ArcBox* a = g.vex[0].firstout; a != nullptr; a = a->tlink)
        cout << g.vex[a->head].name << "(" << a->weight << ") ";
    cout << endl;

    // ========================================================
    // (6) 删除弧 A->C：两条链各摘一次
    // ========================================================
    cout << endl << "=== (6) 删除弧 A->C ===" << endl;
    cout << "  删除结果：" << (g.removeArc(0, 2) ? "成功" : "失败") << endl;
    cout << "  再删一次：" << (g.removeArc(0, 2) ? "成功" : "失败（已不存在）") << endl;
    cout << "  弧数变为 " << g.arcCount() << endl;
    g.printOutLists();
    cout << "  A 出度=" << g.outDegree(0) << "，C 入度=" << g.inDegree(2) << endl;

    // ========================================================
    // (7) 邻接多重表：建立无向图（5 个顶点，6 条边）
    //      A-B(2) A-C(3) B-C(1) B-D(4) C-D(5) D-E(6)
    // ========================================================
    AMList m({"A", "B", "C", "D", "E"});
    m.addEdge(0, 1, 2);
    m.addEdge(0, 2, 3);
    m.addEdge(1, 2, 1);
    m.addEdge(1, 3, 4);
    m.addEdge(2, 3, 5);
    m.addEdge(3, 4, 6);

    cout << endl << "=== (7) 邻接多重表：建立无向图 ===" << endl;
    cout << "顶点数 " << m.n << "，边数 " << m.edgeCount() << endl;
    cout << "边结点个数 = " << m.edgeCount()
         << "；若用邻接表则需 " << 2 * m.edgeCount() << " 个（每条边存两份）" << endl;
    m.printAdjacency();

    // ========================================================
    // (8) 连通分量与欧拉判定 ← 练习 5
    // ========================================================
    cout << endl << "=== (8) 连通性判断 ===" << endl;
    cout << "  连通分量数 = " << m.countComponents() << endl;
    cout << "  所有顶点度都是偶数？" << (m.allDegreeEven() ? "是" : "否")
         << "（E 的度为 1）→ 无欧拉回路" << endl;

    // ========================================================
    // (9) 用 mark 打印每条边一次 ← 练习 4
    // ========================================================
    cout << endl << "=== (9) mark 字段：DFS 每条边只走一次 ===" << endl;
    m.dfsPrintEdges(0);

    // ========================================================
    // (10) 删除边 C-D ← 练习 3
    // ========================================================
    cout << endl << "=== (10) 删除边 C-D ===" << endl;
    cout << "  删除结果：" << (m.removeEdge(2, 3) ? "成功" : "失败") << endl;
    cout << "  边数变为 " << m.edgeCount() << endl;
    m.printAdjacency();
    cout << "  现在所有顶点度都是偶数？" << (m.allDegreeEven() ? "是" : "否") << endl;

    return 0;
}

// ============================================================
// 预期输出
// ============================================================
// === (1) 十字链表：建立有向图 ===
// 顶点数 5，弧数 6
// 出边表（firstout → tlink）：
//   A -> C(3) B(6)
//   B -> D(5) C(2)
//   C -> D(4)
//   D -> A(1)
//   E -> (空)
// 入边表（firstin → hlink）：
//   A <- D(1)
//   B <- A(6)
//   C <- B(2) A(3)
//   D <- C(4) B(5)
//   E <- (空)
// 
// === (2) 出度与入度 ===
//   A：出度=2 入度=1
//   B：出度=2 入度=1
//   C：出度=1 入度=2
//   D：出度=1 入度=2
//   E：出度=0 入度=0
// 出度总和=6，入度总和=6（都应等于弧数 6）
// 出度为 0 的顶点数=1，入度为 0 的顶点数=1  ← 练习 1
// 
// === (3) 查找弧 ===
//   A->C：存在，权值 3
//   C->B：不存在
// 
// === (4) 欧拉回路判定 ===
//   主图（A~E，E 为孤立点）：不存在欧拉回路（A 出度 2、入度 1，不满足入度=出度）
//   有向环 A->B->C->A：存在欧拉回路
// 
// === (5) 转置（求逆图）===
// 出边表（firstout → tlink）：
//   A -> D(1)
//   B -> A(6)
//   C -> B(2) A(3)
//   D -> C(4) B(5)
//   E -> (空)
// 校验：转置后「出度=原入度、入度=原出度」→ 一致
// 已转置回原图，A 的出边表恢复为：C(3) B(6) 
// 
// === (6) 删除弧 A->C ===
//   删除结果：成功
//   再删一次：失败（已不存在）
//   弧数变为 5
// 出边表（firstout → tlink）：
//   A -> B(6)
//   B -> D(5) C(2)
//   C -> D(4)
//   D -> A(1)
//   E -> (空)
//   A 出度=1，C 入度=1
// 
// === (7) 邻接多重表：建立无向图 ===
// 顶点数 5，边数 6
// 边结点个数 = 6；若用邻接表则需 12 个（每条边存两份）
// 邻接链（一条边一个结点，两个端点共用）：
//   A: C(3) B(2)   度=2
//   B: D(4) C(1) A(2)   度=3
//   C: D(5) B(1) A(3)   度=3
//   D: E(6) C(5) B(4)   度=3
//   E: D(6)   度=1
// 
// === (8) 连通性判断 ===
//   连通分量数 = 1
//   所有顶点度都是偶数？否（E 的度为 1）→ 无欧拉回路
// 
// === (9) mark 字段：DFS 每条边只走一次 ===
// DFS 树边：A-C C-D D-E D-B 
// 树边条数 = 4（n-1 = 4）
// 带 mark 的边 = 6 / 6（每条边恰好被访问一次）
// 
// === (10) 删除边 C-D ===
//   删除结果：成功
//   边数变为 5
// 邻接链（一条边一个结点，两个端点共用）：
//   A: C(3) B(2)   度=2
//   B: D(4) C(1) A(2)   度=3
//   C: B(1) A(3)   度=2
//   D: E(6) B(4)   度=2
//   E: D(6)   度=1
//   现在所有顶点度都是偶数？否
