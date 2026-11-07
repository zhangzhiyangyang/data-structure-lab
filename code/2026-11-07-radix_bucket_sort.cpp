/*======================================================================
  2026-11-07  基数排序与桶排序
  内容：
    (1) 稳定计数排序（基数排序的子过程）
    (2) LSD 基数排序（CLRS 经典示例，逐轮打印）
    (3) 练习1 答案：LSD 基数排序手算验证
    (4) 练习2 答案：双关键字排序（扑克牌：先点数后花色）
    (5) 练习3 答案：支持负数的基数排序（分组 + 绝对值）
    (6) MSD 基数排序（字符串，高位优先递归分桶）
    (7) 桶排序（[0,1) 浮点数，桶内插入排序）
    (8) 练习4 答案：桶排序最坏情况演示（全部落入一个桶）
    (9) 练习5 答案：最大间距问题（桶思想，不用两两比较）
======================================================================*/
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

/*--------------------------------------------------------------------
  通用工具：打印整型数组
--------------------------------------------------------------------*/
void printIntArr(const char *label, const vector<int> &a) {
    printf("%s", label);
    for (int x : a) printf(" %d", x);
    printf("\n");
}

/*====================================================================
  (1) 稳定计数排序（不带位的概念的普通版本）
      数据范围 0..k：统计每个值出现次数 → 前缀和（得到“<=该值的个数”
      即最后一个位置）→ 从后往前回填保证稳定性
====================================================================*/
void countingSort(vector<int> &a, int maxVal) {
    int n = (int)a.size();
    vector<int> cnt(maxVal + 1, 0);   // cnt[v] = 值 v 出现的次数
    for (int x : a) cnt[x]++;
    for (int v = 1; v <= maxVal; v++) // 前缀和：cnt[v] = <=v 的元素个数
        cnt[v] += cnt[v - 1];
    vector<int> out(n);
    for (int i = n - 1; i >= 0; i--) {    // 从后往前 → 稳定
        out[cnt[a[i]] - 1] = a[i];        // <=a[i] 有 cnt[a[i]] 个，
        cnt[a[i]]--;                      // 所以 a[i] 放在下标 cnt-1
    }
    a = out;
}

/*====================================================================
  取非负整数 x 的第 d 位数字（d=1 个位，d=2 十位……）
  例：digitOf(802, 1) = 2，digitOf(802, 3) = 8
====================================================================*/
int digitOf(int x, int d) {
    for (int i = 1; i < d; i++) x /= 10;
    return x % 10;
}

/*====================================================================
  稳定计数排序：按第 d 位排序（键只有 0..9 十种取值）
  这就是 LSD 基数排序的“每一轮”
====================================================================*/
void countingSortByDigit(vector<int> &a, int d) {
    int n = (int)a.size();
    vector<int> out(n);
    int cnt[10] = {0};
    for (int x : a) cnt[digitOf(x, d)]++;
    for (int v = 1; v < 10; v++) cnt[v] += cnt[v - 1];
    for (int i = n - 1; i >= 0; i--) {          // 从后往前 → 稳定
        int dig = digitOf(a[i], d);
        out[cnt[dig] - 1] = a[i];
        cnt[dig]--;
    }
    a = out;
}

/*====================================================================
  (2)(3) LSD 基数排序：从低位到高位，每轮做一次稳定计数排序
  参数 maxDigits：最大值的位数（决定排序轮数）
====================================================================*/
void radixSortLSD(vector<int> &a, int maxDigits, bool verbose) {
    static const char *dName[] = {"个位", "十位", "百位", "千位"};
    for (int d = 1; d <= maxDigits; d++) {
        countingSortByDigit(a, d);
        if (verbose) {                       // 打印每一轮的中间结果
            printf("第 %d 轮(%s)后:", d, dName[d - 1]);
            for (int x : a) printf(" %d", x);
            printf("\n");
        }
    }
}

/*====================================================================
  (5) 练习3 答案：支持负数的基数排序
  思路：把负数、非负数分成两组；
        负数取绝对值用 LSD 排好 → 反转后取负即为升序；
        非负数用 LSD 排好；两组拼接即可。
====================================================================*/
void radixSortWithNeg(vector<int> &a) {
    vector<int> neg, pos;
    for (int x : a) {                     // 按符号分组
        if (x < 0) neg.push_back(-x);     // 负数存绝对值
        else       pos.push_back(x);
    }
    // 分别求两组的最大位数
    int dNeg = 1, dPos = 1;
    for (int x : neg) { int t = 0; while (x > 0) { t++; x /= 10; } dNeg = max(dNeg, t); }
    for (int x : pos) { int t = 0; while (x > 0) { t++; x /= 10; } dPos = max(dPos, t); }
    radixSortLSD(neg, dNeg, false);       // 排绝对值
    reverse(neg.begin(), neg.end());      // 绝对值升序反转
    for (int &x : neg) x = -x;            // 恢复负号 → 负数部分升序
    radixSortLSD(pos, dPos, false);
    a.clear();
    a.insert(a.end(), neg.begin(), neg.end());
    a.insert(a.end(), pos.begin(), pos.end());
}

/*====================================================================
  (6) MSD 基数排序（字符串，变长）
      对 [lo, hi] 区间按第 pos 个字符递归分桶：
      桶 0 放“已经到末尾的短串”（不再递归），
      桶 c+1 放第 pos 个字符为 c 的串（继续递归下一位）。
====================================================================*/
void msdSort(vector<string> &a, int lo, int hi, int pos) {
    if (lo >= hi) return;
    vector<vector<string>> bucket(257);          // 0 号桶 + 256 个字符桶
    for (int i = lo; i <= hi; i++) {
        if (pos < (int)a[i].size())              // 还有字符 → 按字符分桶
            bucket[(unsigned char)a[i][pos] + 1].push_back(a[i]);
        else                                     // 已到末尾 → 0 号桶
            bucket[0].push_back(a[i]);
    }
    int idx = lo;
    for (auto &b : bucket)                       // 桶依次写回原区间
        for (auto &s : b) a[idx++] = s;
    idx = lo;
    for (int c = 0; c < 257; c++) {              // 对每个桶继续递归
        int sz = (int)bucket[c].size();
        if (sz > 1 && c != 0)                    // 0 号桶是短串，不用再排
            msdSort(a, idx, idx + sz - 1, pos + 1);
        idx += sz;
    }
}

/*====================================================================
  桶内排序用的插入排序（数据量小，插入排序最快且写法简单）
====================================================================*/
void insertionSort(vector<double> &b) {
    for (int i = 1; i < (int)b.size(); i++) {
        double key = b[i];
        int j = i - 1;
        while (j >= 0 && b[j] > key) { b[j + 1] = b[j]; j--; }
        b[j + 1] = key;
    }
}

/*====================================================================
  (7)(8) 桶排序：输入 n 个 [0,1) 的浮点数
      映射函数：桶号 = (int)(v * n)  → 平均每桶 1 个元素
      桶内插入排序，最后按桶顺序拼接
====================================================================*/
vector<vector<double>> bucketSort(vector<double> &a, bool showBuckets) {
    int n = (int)a.size();
    vector<vector<double>> buckets(n);
    for (double v : a) buckets[(int)(v * n)].push_back(v);  // 分桶
    for (auto &b : buckets) insertionSort(b);               // 桶内排序
    if (showBuckets) {                                      // 打印非空桶
        for (int i = 0; i < n; i++) {
            if (buckets[i].empty()) continue;
            printf("桶 %d:", i);
            for (double v : buckets[i]) printf(" %.2f", v);
            printf("\n");
        }
    }
    int idx = 0;                                            // 按桶拼接
    for (auto &b : buckets)
        for (double v : b) a[idx++] = v;
    return buckets;
}

/*====================================================================
  (9) 练习5 答案：最大间距（LeetCode 164 思想）
      n 个数，min~max 均分成 n-1 个桶，答案只能出现在
      “相邻两个非空桶 后桶最小值 - 前桶最大值” 之间
      （同桶内元素间距必小于桶宽，所以不需要比较同桶元素）
====================================================================*/
int maxGap(const vector<int> &a) {
    int n = (int)a.size();
    if (n < 2) return 0;
    int mn = *min_element(a.begin(), a.end());
    int mx = *max_element(a.begin(), a.end());
    if (mn == mx) return 0;
    // 桶宽 = 向上取整((mx-mn)/(n-1))，保证桶数 >= n-1 > 1
    int width = (mx - mn + n - 2) / (n - 1);
    if (width < 1) width = 1;
    int cnt = (mx - mn) / width + 1;            // 桶的个数
    vector<int> bMin(cnt, -1), bMax(cnt, -1);   // -1 表示空桶
    for (int x : a) {
        int i = (x - mn) / width;
        if (bMin[i] == -1 || x < bMin[i]) bMin[i] = x;
        if (bMax[i] == -1 || x > bMax[i]) bMax[i] = x;
    }
    int ans = 0, prevMax = bMax[0];             // 第一个桶必含最小值
    for (int i = 1; i < cnt; i++) {
        if (bMin[i] == -1) continue;            // 空桶跳过
        ans = max(ans, bMin[i] - prevMax);      // 跨桶的间距
        prevMax = bMax[i];
    }
    return ans;
}

/*====================================================================
  (4) 练习2 答案：双关键字排序（扑克牌）
      每张牌 = (花色 suit 0..3, 点数 rank 1..13)
      LSD 原则：先按次关键字（点数）稳定排序，再按主关键字（花色）
      稳定排序 → 结果就是花色有序、同花色内点数有序
====================================================================*/
// 对 (suit, rank) 数组按指定关键字做稳定计数排序：key=0 按花色，1 按点数
void countingSortPair(vector<pair<int,int>> &a, int key, int maxKey) {
    int n = (int)a.size();
    vector<int> cnt(maxKey + 1, 0);
    for (auto &p : a) cnt[key == 0 ? p.first : p.second]++;
    for (int v = 1; v <= maxKey; v++) cnt[v] += cnt[v - 1];
    vector<pair<int,int>> out(n);
    for (int i = n - 1; i >= 0; i--) {
        int k = key == 0 ? a[i].first : a[i].second;
        out[cnt[k] - 1] = a[i];
        cnt[k]--;
    }
    a = out;
}

void printCards(const char *label, const vector<pair<int,int>> &cards) {
    printf("%s", label);
    const char *suitName[] = {"黑桃", "红桃", "梅花", "方块"};
    for (auto &p : cards) printf(" (%s%d)", suitName[p.first], p.second);
    printf("\n");
}

/*====================================================================
  main：依次演示以上全部内容
====================================================================*/
int main() {
    printf("========== (1) 稳定计数排序 ==========\n");
    vector<int> a1 = {4, 2, 2, 8, 3, 3, 1};
    printIntArr("原数组      :", a1);
    countingSort(a1, 8);
    printIntArr("计数排序结果:", a1);
    printf("（稳定版从后往前回填，相等元素相对顺序不变）\n");

    printf("\n========== (2) LSD 基数排序（CLRS 经典示例） ==========\n");
    vector<int> a2 = {329, 457, 657, 839, 436, 720, 355};
    printIntArr("原数组  :", a2);
    radixSortLSD(a2, 3, true);
    printIntArr("最终结果:", a2);

    printf("\n========== (3) 练习1 答案：LSD 手算验证 ==========\n");
    vector<int> a3 = {170, 45, 75, 90, 2, 802, 24, 66};
    printIntArr("原数组:", a3);
    radixSortLSD(a3, 3, true);
    printIntArr("最终结果:", a3);

    printf("\n========== (4) 练习2 答案：扑克牌双关键字排序 ==========\n");
    vector<pair<int,int>> cards = {{0,3},{2,11},{1,3},{3,7},{0,13},{1,1}};
    printCards("发牌顺序        :", cards);
    countingSortPair(cards, 1, 13);   // 先按点数（次关键字）
    printCards("先按点数排序后  :", cards);
    countingSortPair(cards, 0, 3);    // 再按花色（主关键字）
    printCards("再按花色排序后  :", cards);

    printf("\n========== (5) 练习3 答案：含负数的基数排序 ==========\n");
    vector<int> a5 = {-5, 13, -32, 7, -3, 0, 25, -7};
    printIntArr("原数组    :", a5);
    radixSortWithNeg(a5);
    printIntArr("排序结果  :", a5);

    printf("\n========== (6) MSD 基数排序（字符串） ==========\n");
    vector<string> words = {"banana", "apple", "cherry", "banana", "grape", "avocado"};
    printf("原顺序 :");
    for (auto &w : words) printf(" %s", w.c_str());
    printf("\n按首字母分桶: ");
    bool firstGroup = true;                    // 组间用一个空格分隔，避免行尾空格
    for (char c = 'a'; c <= 'g'; c++) {
        if (!firstGroup) printf(" ");
        firstGroup = false;
        printf("%c组:", c);
        for (auto &w : words)
            if (w[0] == c) printf(" %s", w.c_str());
    }
    printf("\n");
    msdSort(words, 0, (int)words.size() - 1, 0);
    printf("排序后 :");
    for (auto &w : words) printf(" %s", w.c_str());
    printf("\n");

    printf("\n========== (7) 桶排序：[0,1) 浮点数 ==========\n");
    vector<double> a7 = {0.78, 0.17, 0.39, 0.26, 0.72, 0.94, 0.21, 0.12, 0.23, 0.68};
    printf("原数据:");
    for (double v : a7) printf(" %.2f", v);
    printf("\n");
    bucketSort(a7, true);
    printf("排序结果:");
    for (double v : a7) printf(" %.2f", v);
    printf("\n");

    printf("\n========== (8) 练习4 答案：最坏情况（全落一个桶） ==========\n");
    vector<double> a8 = {0.05, 0.02, 0.09, 0.01, 0.04, 0.07};
    printf("原数据:");
    for (double v : a8) printf(" %.2f", v);
    printf("\n");
    bucketSort(a8, true);
    printf("排序结果:");
    for (double v : a8) printf(" %.2f", v);
    printf("\n全部落入 1 个桶 → 桶内插入排序退化为 O(n^2)，\n"
           "缓解办法：多设桶、或在桶内改用 O(n log n) 排序。\n");

    printf("\n========== (9) 练习5 答案：最大间距（桶思想） ==========\n");
    vector<int> a9 = {3, 6, 8, 10, 15, 50};
    printIntArr("数据    :", a9);
    printf("相邻元素最大间距 = %d（50 - 15，跨越了空桶 2、3 号）\n", maxGap(a9));

    return 0;
}

/*====================== 预期输出 ======================
========== (1) 稳定计数排序 ==========
原数组      : 4 2 2 8 3 3 1
计数排序结果: 1 2 2 3 3 4 8
（稳定版从后往前回填，相等元素相对顺序不变）

========== (2) LSD 基数排序（CLRS 经典示例） ==========
原数组  : 329 457 657 839 436 720 355
第 1 轮(个位)后: 720 355 436 457 657 329 839
第 2 轮(十位)后: 720 329 436 839 355 457 657
第 3 轮(百位)后: 329 355 436 457 657 720 839
最终结果: 329 355 436 457 657 720 839

========== (3) 练习1 答案：LSD 手算验证 ==========
原数组: 170 45 75 90 2 802 24 66
第 1 轮(个位)后: 170 90 2 802 24 45 75 66
第 2 轮(十位)后: 2 802 24 45 66 170 75 90
第 3 轮(百位)后: 2 24 45 66 75 90 170 802
最终结果: 2 24 45 66 75 90 170 802

========== (4) 练习2 答案：扑克牌双关键字排序 ==========
发牌顺序        : (黑桃3) (梅花11) (红桃3) (方块7) (黑桃13) (红桃1)
先按点数排序后  : (红桃1) (黑桃3) (红桃3) (方块7) (梅花11) (黑桃13)
再按花色排序后  : (黑桃3) (黑桃13) (红桃1) (红桃3) (梅花11) (方块7)

========== (5) 练习3 答案：含负数的基数排序 ==========
原数组    : -5 13 -32 7 -3 0 25 -7
排序结果  : -32 -7 -5 -3 0 7 13 25

========== (6) MSD 基数排序（字符串） ==========
原顺序 : banana apple cherry banana grape avocado
按首字母分桶: a组: apple avocado b组: banana banana c组: cherry d组: e组: f组: g组: grape
排序后 : apple avocado banana banana cherry grape

========== (7) 桶排序：[0,1) 浮点数 ==========
原数据: 0.78 0.17 0.39 0.26 0.72 0.94 0.21 0.12 0.23 0.68
桶 1: 0.12 0.17
桶 2: 0.21 0.23 0.26
桶 3: 0.39
桶 6: 0.68
桶 7: 0.72 0.78
桶 9: 0.94
排序结果: 0.12 0.17 0.21 0.23 0.26 0.39 0.68 0.72 0.78 0.94

========== (8) 练习4 答案：最坏情况（全落一个桶） ==========
原数据: 0.05 0.02 0.09 0.01 0.04 0.07
桶 0: 0.01 0.02 0.04 0.05 0.07 0.09
排序结果: 0.01 0.02 0.04 0.05 0.07 0.09
全部落入 1 个桶 → 桶内插入排序退化为 O(n^2)，
缓解办法：多设桶、或在桶内改用 O(n log n) 排序。

========== (9) 练习5 答案：最大间距（桶思想） ==========
数据    : 3 6 8 10 15 50
相邻元素最大间距 = 35（50 - 15，跨越了空桶 2、3 号）
======================= 结束 ==========================*/
