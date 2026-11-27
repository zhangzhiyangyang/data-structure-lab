/* ============================================================
 * 动态规划初探与 0/1 背包问题（2026-11-27）
 * 功能：演示五种解法的完整代码
 *   (1) 朴素递归（指数级，统计调用次数）
 *   (2) 记忆化搜索（缓存子问题）
 *   (3) 二维制表 dp[i][j]（含逐行打印，可手算验证）
 *   (4) 一维滚动数组（容量逆序枚举，打印每件物品后的表）
 *   (5) 方案还原（从二维表倒推选了哪些物品）
 *   (6) 完全背包对照（同一份代码只把容量枚举改成正序）
 *   (7) 贪心反例验证（按单位价值贪心对 0/1 背包不保证最优）
 * 编译：g++ -std=c++17 -Wall -Wextra -o dp_check dp_knapsack.cpp
 * ============================================================ */
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

/* "恰好装满"语义下的不可达标记（用足够小的负数防止 +v[i] 溢出伪装成合法值） */
const int NEG = INT_MIN / 2;

/* ---------- 物品数据（练习 1 的数据，全局共享） ---------- */
int n;                 // 物品件数
int W;                 // 背包容量
vector<int> w, v;      // w[i] 第 i 件重量、v[i] 第 i 件价值（下标从 0 开始）

long long recCalls = 0;            // 朴素递归的调用次数统计
vector<vector<int>> memo;          // 记忆化缓存，memo[i][r] == -1 表示未计算
long long memoMiss = 0;            // 记忆化实际计算的子问题个数

/* ---------- 方法 1：朴素递归 ----------
 * i     : 当前考虑第 i 件（从 0 开始）
 * remain: 剩余容量
 * 返回   : 从第 i 件开始能取得的最大价值 */
int rec(int i, int remain) {
    ++recCalls;                          // 统计调用次数，展示指数爆炸
    if (i == n || remain <= 0) return 0; // 没有物品或没容量：价值 0
    int best = rec(i + 1, remain);       // 决策 A：不选第 i 件
    if (w[i] <= remain)                  // 决策 B：选第 i 件（前提装得下）
        best = max(best, v[i] + rec(i + 1, remain - w[i]));
    return best;
}

/* ---------- 方法 2：记忆化搜索 ----------
 * 与 rec 结构完全相同，只是进入时先查缓存，算完写回缓存 */
int recMemo(int i, int remain) {
    if (i == n || remain <= 0) return 0;
    if (memo[i][remain] != -1) return memo[i][remain]; // 命中缓存直接返回
    ++memoMiss;                                        // 否则真的算一次
    int best = recMemo(i + 1, remain);
    if (w[i] <= remain)
        best = max(best, v[i] + recMemo(i + 1, remain - w[i]));
    memo[i][remain] = best;                            // 写回缓存
    return best;
}

/* ---------- 方法 3：二维制表 ----------
 * dp[i][j] = 前 i 件物品、容量 j（不超过 j）的最大价值
 * 第 0 行全 0：一件都不选时价值为 0 */
vector<vector<int>> knap2D() {
    vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));
    for (int i = 1; i <= n; ++i)            // 阶段 i：逐件引入物品（编号 i-1）
        for (int j = 0; j <= W; ++j) {      // 维度 j：容量从 0 到 W
            dp[i][j] = dp[i - 1][j];        // 默认：不选第 i 件
            if (j >= w[i - 1])              // 装得下才考虑"选"
                dp[i][j] = max(dp[i][j], dp[i - 1][j - w[i - 1]] + v[i - 1]);
        }
    return dp;
}

/* ---------- 方案还原：从 (n, W) 倒推选了哪些物品 ----------
 * 若 dp[i][j] != dp[i-1][j]，说明第 i 件（编号 i-1）被选中 */
vector<int> findPlan(const vector<vector<int>>& dp) {
    vector<int> picked;
    int i = n, j = W;
    while (i > 0) {
        if (dp[i][j] != dp[i - 1][j]) {   // 值变了 → 这件被选了
            picked.push_back(i);          // 记录 1-based 编号
            j -= w[i - 1];                // 容量扣掉它的重量
        }
        --i;                              // 回到上一行
    }
    reverse(picked.begin(), picked.end()); // 倒推得到的是逆序，翻转一下
    return picked;
}

/* ---------- 方法 4：一维滚动数组（容量必须逆序枚举） ----------
 * trace 非空时，记录每件物品处理完后的一维表（练习 2 用） */
int knap1D(vector<vector<int>>* trace = nullptr) {
    vector<int> dp(W + 1, 0);              // dp[j] 即"当前已处理物品、容量 j"的最优
    for (int i = 0; i < n; ++i) {
        for (int j = W; j >= w[i]; --j) {  // 关键：逆序！保证 dp[j-w[i]] 是旧值
            dp[j] = max(dp[j], dp[j - w[i]] + v[i]);
        }
        if (trace) trace->push_back(dp);   // 留存快照供打印
    }
    return dp[W];
}

/* ---------- 完全背包：一维数组 + 容量正序枚举 ----------
 * 正序时 dp[j-w[i]] 已被本轮更新 → 同一件物品可重复放 → 完全背包语义 */
int knapComplete() {
    vector<int> dp(W + 1, 0);
    for (int i = 0; i < n; ++i)
        for (int j = w[i]; j <= W; ++j)    // 与 0/1 背包唯一区别：方向
            dp[j] = max(dp[j], dp[j - w[i]] + v[i]);
    return dp[W];
}

/* ---------- 贪心（按单位价值 v/w 从高到低装），供反例验证 ---------- */
int greedy(int cap) {
    vector<int> idx(n);                    // 物品编号 0..n-1
    for (int i = 0; i < n; ++i) idx[i] = i;
    sort(idx.begin(), idx.end(), [](int a, int b) {
        return (double)v[a] / w[a] > (double)v[b] / w[b]; // 单位价值降序
    });
    int total = 0;
    for (int i : idx)                      // 依次尝试装（0/1 语义：装不下就跳过）
        if (w[i] <= cap) { total += v[i]; cap -= w[i]; }
    return total;
}

/* ---------- 打印一维表（供 (3)(4) 使用，行间空格分隔、无行尾空格） ---------- */
void printRow(const vector<int>& row) {
    for (size_t k = 0; k < row.size(); ++k) {
        if (k) cout << ' ';        // 数字之间才打印空格，避免行尾空格
        cout << row[k];
    }
    cout << '\n';
}

int main() {
    /* 物品：重量 {2,3,4,5}，价值 {3,4,5,6}，容量 8 */
    w = {2, 3, 4, 5};
    v = {3, 4, 5, 6};
    n = (int)w.size();
    W = 8;

    cout << "(1) 朴素递归（暴力，指数级）\n";
    recCalls = 0;
    int a1 = rec(0, W);
    cout << "最大价值 = " << a1 << "，递归调用次数 = " << recCalls << "\n";

    cout << "(2) 记忆化搜索\n";
    memo.assign(n + 1, vector<int>(W + 1, -1)); // -1 标记"未计算"
    memoMiss = 0;
    int a2 = recMemo(0, W);
    cout << "最大价值 = " << a2 << "，实际计算的子问题数 = " << memoMiss << "\n";

    cout << "(3) 二维制表（打印全部 n+1 行）\n";
    vector<vector<int>> dp = knap2D();
    for (int i = 0; i <= n; ++i) printRow(dp[i]);
    cout << "最大价值 = " << dp[n][W] << "\n";

    cout << "(4) 一维滚动数组（逆序枚举，每件物品处理后的表）\n";
    vector<vector<int>> trace;
    int a4 = knap1D(&trace);
    for (int i = 0; i < n; ++i) {
        cout << "放入物品" << i + 1 << "(w=" << w[i] << ",v=" << v[i] << "): ";
        printRow(trace[i]);
    }
    cout << "最大价值 = " << a4 << "\n";

    cout << "(5) 方案还原\n";
    vector<int> plan = findPlan(dp);
    cout << "选中物品编号（1-based）:";
    for (int k : plan) cout << ' ' << k;
    cout << "\n";
    int tw = 0, tv = 0;
    for (int k : plan) { tw += w[k - 1]; tv += v[k - 1]; }
    cout << "总重量 = " << tw << "，总价值 = " << tv << "\n";

    cout << "(6) 完全背包对照（容量正序）\n";
    int a6 = knapComplete();
    cout << "完全背包最大价值 = " << a6 << "（0/1 背包为 " << dp[n][W] << "）\n";

    cout << "(7) 贪心反例验证\n";
    /* 反例：物品 (w=4,v=6) (w=3,v=4) (w=3,v=4)，容量 6
     * 单位价值：1.5 > 1.33，贪心先装 (4,6)，剩 2 装不下 → 得 6
     * DP 取两件 (3,4) 恰好装满 → 得 8 */
    w = {4, 3, 3};
    v = {6, 4, 4};
    n = 3;
    W = 6;
    cout << "贪心最大价值 = " << greedy(W) << "\n";
    vector<vector<int>> dp2 = knap2D();
    cout << "DP 最大价值 = " << dp2[n][W] << "\n";

    return 0;
}

/* ================= 预期输出 =================
(1) 朴素递归（暴力，指数级）
最大价值 = 10，递归调用次数 = 24
(2) 记忆化搜索
最大价值 = 10，实际计算的子问题数 = 14
(3) 二维制表（打印全部 n+1 行）
0 0 0 0 0 0 0 0 0
0 0 3 3 3 3 3 3 3
0 0 3 4 4 7 7 7 7
0 0 3 4 5 7 8 9 9
0 0 3 4 5 7 8 9 10
最大价值 = 10
(4) 一维滚动数组（逆序枚举，每件物品处理后的表）
放入物品1(w=2,v=3): 0 0 3 3 3 3 3 3 3
放入物品2(w=3,v=4): 0 0 3 4 4 7 7 7 7
放入物品3(w=4,v=5): 0 0 3 4 5 7 8 9 9
放入物品4(w=5,v=6): 0 0 3 4 5 7 8 9 10
最大价值 = 10
(5) 方案还原
选中物品编号（1-based）: 2 4
总重量 = 8，总价值 = 10
(6) 完全背包对照（容量正序）
完全背包最大价值 = 12（0/1 背包为 10）
(7) 贪心反例验证
贪心最大价值 = 6
DP 最大价值 = 8
 ============================================ */
