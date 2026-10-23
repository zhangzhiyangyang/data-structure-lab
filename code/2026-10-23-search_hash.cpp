// 2026-10-23 查找：顺序查找、折半查找、散列表（开放定址 + 链地址）
// 编译: g++ -std=c++17 -Wall -Wextra -o search_hash search_hash.cpp
// 覆盖笔记中的核心操作，并作为练习 1/3/4/5 的参考答案。

#include <iostream>
#include <vector>

// ---------------- 1. 顺序查找（带哨兵） ----------------
// data[0] 作哨兵，从后往前比较，成功返回下标，失败返回 0
int seqSearch(std::vector<int>& a, int n, int key) {
    a[0] = key;                 // 哨兵
    int i = n;
    while (a[i] != key) --i;    // 只剩一个判断条件
    return i;                   // i==0 即失败
}

// ---------------- 2. 折半查找 ----------------
// 有序数组（下标 1..n），mid = (low+high)/2，成功返回下标，失败返回 0
int binSearch(const std::vector<int>& a, int n, int key) {
    int low = 1, high = n;
    std::vector<int> trace;     // 记录比较路径，供练习 1 输出
    while (low <= high) {                       // 易错点 1：必须是 <=
        int mid = low + (high - low) / 2;       // 防溢出写法
        trace.push_back(mid);
        if (a[mid] == key) {
            std::cout << "  比较位置:";
            for (int m : trace) std::cout << " " << m << "(" << a[m] << ")";
            std::cout << "  -> 命中\n";
            return mid;
        } else if (a[mid] > key) high = mid - 1;
        else low = mid + 1;
    }
    std::cout << "  比较位置:";
    for (int m : trace) std::cout << " " << m << "(" << a[m] << ")";
    std::cout << "  -> 失败\n";
    return 0;
}

// ---------------- 3. 开放定址散列表（线性探测 + 惰性删除） ----------------
struct OpenHash {
    static const int M = 16;
    enum State { EMPTY, USED, DELETED };
    int key[M];
    State st[M];
    long long cmp;              // 最近一次查找的比较次数
    OpenHash() : st{}, cmp(0) {}

    int h(int k) const { return k % 13; }       // 除留余数法 p=13（素数），M=16

    // 查找：返回槽位，失败返回 -1。遇到 EMPTY 才停，DELETED 继续走（墓碑规则）
    int search(int k) {
        cmp = 0;
        for (int d = 0; d < M; ++d) {
            int pos = (h(k) + d) % M;
            ++cmp;
            if (st[pos] == EMPTY) return -1;    // 空位截断，必然不存在
            if (st[pos] == USED && key[pos] == k) return pos;
            // DELETED：继续探测
        }
        return -1;
    }

    // 插入：先确认不存在，再占用首个 DELETED 或 EMPTY 位
    bool insert(int k) {
        if (search(k) != -1) return false;
        for (int d = 0; d < M; ++d) {
            int pos = (h(k) + d) % M;
            if (st[pos] != USED) { st[pos] = USED; key[pos] = k; return true; }
        }
        return false;           // 表满
    }

    // 惰性删除：打墓碑标记，保证探测链不断（易错点 6）
    bool remove(int k) {
        int pos = search(k);
        if (pos == -1) return false;
        st[pos] = DELETED;
        return true;
    }
};

// ---------------- 4. 链地址散列表（头插法） ----------------
struct Node { int key; Node* next; };
struct ChainHash {
    static const int M = 13;
    Node* bucket[M];
    ChainHash() : bucket{} {}
    int h(int k) const { return k % M; }

    void insert(int k) {
        int b = h(k);
        bucket[b] = new Node{k, bucket[b]};     // 头插 O(1)
    }
    Node* search(int k) const {
        for (Node* p = bucket[h(k)]; p; p = p->next)
            if (p->key == k) return p;
        return nullptr;
    }
    void remove(int k) {
        for (Node** pp = &bucket[h(k)]; *pp; pp = &(*pp)->next)
            if ((*pp)->key == k) { Node* t = *pp; *pp = t->next; delete t; return; }
    }
};

// 统计链地址表的一次成功查找比较次数（练习 4 用）
int chainCmp(const ChainHash& t, int k) {
    int c = 0;
    for (Node* p = t.bucket[t.h(k)]; p; p = p->next) {
        ++c;
        if (p->key == k) return c;
    }
    return c;
}

int main() {
    // ---- 顺序查找演示 ----
    std::vector<int> sa(64);
    int n = 8;
    int raw[] = {0, 5, 13, 19, 21, 37, 56, 64, 75};   // raw[0] 会被哨兵覆盖
    for (int i = 0; i <= n; ++i) sa[i] = raw[i];
    std::cout << "[顺序查找] 37 在下标 " << seqSearch(sa, n, 37)
              << "，99 在下标 " << seqSearch(sa, n, 99) << "（0=失败）\n";

    // ---- 练习 1：折半查找 38 与 45 ----
    int sorted[] = {0, 7, 14, 18, 21, 23, 29, 31, 35, 38, 42, 46, 49, 52}; // 1..13
    std::vector<int> ba(std::begin(sorted), std::end(sorted));
    std::cout << "[折半查找] 38:\n";
    binSearch(ba, 13, 38);
    std::cout << "[折半查找] 45:\n";
    binSearch(ba, 13, 45);

    // ---- 练习 3：线性探测建表 ----
    int keys[] = {19, 14, 23, 1, 68, 20, 84, 27, 55, 11, 10, 79};
    int nk = sizeof(keys) / sizeof(keys[0]);
    OpenHash oh;
    for (int i = 0; i < nk; ++i) oh.insert(keys[i]);
    std::cout << "[开放定址] 各关键字存放位置:\n";
    for (int i = 0; i < nk; ++i) {
        int pos = oh.search(keys[i]);
        std::cout << "  " << keys[i] << " -> 槽 " << pos << "（H=" << keys[i] % 13
                  << " 基准，探测 " << oh.cmp << " 次）\n";
    }
    long long total = 0;
    for (int i = 0; i < nk; ++i) { oh.search(keys[i]); total += oh.cmp; }
    std::cout << "  ASL成功 = " << total << "/" << nk << " = "
              << (double)total / nk << "\n";

    // ---- 练习 5：墓碑机制验证 ----
    std::cout << "[惰性删除] 删除 27（槽 " << oh.search(27) << "）后:\n";
    oh.remove(27);
    int p = oh.search(27);
    std::cout << "  查 27: " << (p == -1 ? "失败(符合预期)" : "异常!") << "\n";
    oh.insert(40);                           // 27 的同义词（40%13=27%13=1）
    p = oh.search(40);
    std::cout << "  插入同义词 40 后查 40: 槽 " << p
              << "，比较 " << oh.cmp << " 次（墓碑后仍可达）\n";

    // ---- 练习 4：链地址法建表与 ASL ----
    ChainHash ch;
    for (int i = 0; i < nk; ++i) ch.insert(keys[i]);
    total = 0;
    std::cout << "[链地址] 各关键字查找比较次数:\n";
    for (int i = 0; i < nk; ++i) {
        int c = chainCmp(ch, keys[i]);
        total += c;
        std::cout << "  " << keys[i] << ": " << c << " 次\n";
    }
    std::cout << "  ASL成功 = " << total << "/" << nk << " = "
              << (double)total / nk << "\n";
    ch.remove(27);
    std::cout << "[链地址] 删除 27 后查找: "
              << (ch.search(27) ? "存在(异常)" : "不存在(符合预期)") << "\n";
    return 0;
}

/* 预期输出:
[顺序查找] 37 在下标 5，99 在下标 0（0=失败）
[折半查找] 38:
  比较位置: 7(31) 10(42) 8(35) 9(38)  -> 命中
[折半查找] 45:
  比较位置: 7(31) 10(42) 12(49) 11(46)  -> 失败
[开放定址] 各关键字存放位置:
  19 -> 槽 6（H=6 基准，探测 1 次）
  14 -> 槽 1（H=1 基准，探测 1 次）
  23 -> 槽 10（H=10 基准，探测 1 次）
  1 -> 槽 2（H=1 基准，探测 2 次）
  68 -> 槽 3（H=3 基准，探测 1 次）
  20 -> 槽 7（H=7 基准，探测 1 次）
  84 -> 槽 8（H=6 基准，探测 3 次）
  27 -> 槽 4（H=1 基准，探测 4 次）
  55 -> 槽 5（H=3 基准，探测 3 次）
  11 -> 槽 11（H=11 基准，探测 1 次）
  10 -> 槽 12（H=10 基准，探测 3 次）
  79 -> 槽 9（H=1 基准，探测 9 次）
  ASL成功 = 30/12 = 2.5
[惰性删除] 删除 27（槽 4）后:
  查 27: 失败(符合预期)
  插入同义词 40 后查 40: 槽 4，比较 4 次（墓碑后仍可达）
[链地址] 各关键字查找比较次数:
  19: 2 次
  14: 4 次
  23: 2 次
  1: 3 次
  68: 2 次
  20: 1 次
  84: 1 次
  27: 2 次
  55: 1 次
  11: 1 次
  10: 1 次
  79: 1 次
  ASL成功 = 21/12 = 1.75
[链地址] 删除 27 后查找: 不存在(符合预期)
*/
