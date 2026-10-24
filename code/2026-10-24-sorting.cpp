// 2026-10-24 排序：插入、交换（冒泡/快排）、堆、归并与计数排序
// 配套笔记：notes/2026-10-24-排序.md（练习答案见各函数与 main 输出）
// 编译：g++ -std=c++17 -Wall -Wextra -o sort_check 2026-10-24-sorting.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// ---------- 工具 ----------
void printVec(const string& tag, const vector<int>& a) {
    cout << tag << ": ";
    for (size_t i = 0; i < a.size(); ++i) cout << a[i] << " ";
    cout << "\n";
}

// ---------- 练习 1：直接插入排序，统计比较/移动次数 ----------
// 比较次数：a[j] 与 tmp 的比较；移动次数：元素后移 + tmp 落位
void insertionSort(vector<int> a, long long& cmp, long long& mov) {
    cmp = mov = 0;
    for (size_t i = 1; i < a.size(); ++i) {
        int tmp = a[i];                       // 移动 +1（取出）
        size_t j = i;
        while (j > 0) {
            ++cmp;
            if (a[j - 1] > tmp) { a[j] = a[j - 1]; ++mov; --j; } // 后移 +1
            else break;
        }
        a[j] = tmp; ++mov;                    // 移动 +1（落位）
    }
    printVec("插入排序结果", a);
}

// ---------- 练习 2：冒泡排序（每趟打印，可提前终止）----------
void bubbleSort(vector<int> a) {
    size_t n = a.size();
    for (size_t i = 0; i + 1 < n; ++i) {
        bool swapped = false;
        for (size_t j = 0; j + 1 < n - i; ++j)
            if (a[j] > a[j + 1]) { swap(a[j], a[j + 1]); swapped = true; }
        cout << "冒泡第" << (i + 1) << "趟后: ";
        for (int x : a) cout << x << " ";
        cout << "\n";
        if (!swapped) { cout << "无交换，提前终止\n"; break; }
    }
}

// ---------- 练习 3：快速排序 partition（以末元素为 pivot）----------
int partition(vector<int>& a, int lo, int hi) {
    int pivot = a[hi], i = lo;                // i 是"小于区"的右边界+1
    for (int j = lo; j < hi; ++j)
        if (a[j] < pivot) swap(a[i++], a[j]); // 易错：必须是 <，不是 <=
    swap(a[i], a[hi]);                        // pivot 归位
    return i;
}
void quickSort(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int p = partition(a, lo, hi);
    quickSort(a, lo, p - 1);                  // pivot 不再参与递归
    quickSort(a, p + 1, hi);
}

// ---------- 练习 4：堆排序 ----------
// 0 起点下标：父 i，孩子 2i+1 / 2i+2
void siftDown(vector<int>& a, int i, int size) {
    while (true) {
        int l = 2 * i + 1, r = 2 * i + 2, big = i;
        if (l < size && a[l] > a[big]) big = l;
        if (r < size && a[r] > a[big]) big = r;
        if (big == i) break;
        swap(a[i], a[big]);
        i = big;                              // 继续下潜
    }
}
void heapSort(vector<int> a) {
    int n = (int)a.size();
    for (int i = n / 2 - 1; i >= 0; --i) siftDown(a, i, n); // 建大顶堆，倒序！
    printVec("建堆后", a);
    for (int size = n; size > 1; --size) {
        swap(a[0], a[size - 1]);              // 堆顶(最大)放到末尾
        siftDown(a, 0, size - 1);
        if (size >= n - 2) printVec("堆排第" + to_string(n - size + 1) + "趟后", a);
    }
    printVec("堆排序结果", a);
}

// ---------- 练习 5：归并排序（自顶向下）----------
void mergeRange(vector<int>& a, vector<int>& tmp, int lo, int mid, int hi) {
    for (int i = lo; i <= hi; ++i) tmp[i] = a[i];
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) a[k++] = (tmp[i] <= tmp[j]) ? tmp[i++] : tmp[j++]; // <= 保稳定
    while (i <= mid) a[k++] = tmp[i++];
    while (j <= hi) a[k++] = tmp[j++];        // 易错：两段剩余都要拷回
}
void mergeSort(vector<int>& a, vector<int>& tmp, int lo, int hi, int depth) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    mergeSort(a, tmp, lo, mid, depth + 1);
    mergeSort(a, tmp, mid + 1, hi, depth + 1);
    mergeRange(a, tmp, lo, mid, hi);
    cout << "归并[" << lo << "," << hi << "]后: ";   // 打印每次合并结果
    for (int x : a) cout << x << " ";
    cout << "\n";
}

// ---------- 练习 6：稳定版计数排序（倒序回填）----------
void countingSort(vector<int>& a) {
    if (a.empty()) return;
    int k = *max_element(a.begin(), a.end());
    vector<int> cnt(k + 1, 0), res(a.size());
    for (int x : a) ++cnt[x];                 // 计数
    for (int v = 1; v <= k; ++v) cnt[v] += cnt[v - 1]; // 前缀和 = 结束位置
    cout << "计数数组(前缀和后): ";
    for (int v = 0; v <= k; ++v) cout << cnt[v] << " ";
    cout << "\n";
    for (int i = (int)a.size() - 1; i >= 0; --i) // 倒序！保证稳定
        res[--cnt[a[i]]] = a[i];
    a = res;
    printVec("计数排序结果", a);
}

// ---------- 主测试 ----------
int main() {
    // 练习 1：插入排序计数。逆序 5,2,4,6,1,3 有 7 个逆序对 → 比较 10、移动 12
    long long cmp = 0, mov = 0;
    insertionSort({5, 2, 4, 6, 1, 3}, cmp, mov);
    cout << "插入排序 比较=" << cmp << " 移动=" << mov << "\n";
    // 逆序数组（8 个）验证最坏情形：比较 28、移动 36
    insertionSort({8, 7, 6, 5, 4, 3, 2, 1}, cmp, mov);
    cout << "逆序插入排序 比较=" << cmp << " 移动=" << mov << "\n\n";

    // 练习 2：冒泡 4,5,2,1,3
    bubbleSort({4, 5, 2, 1, 3});
    cout << "\n";

    // 练习 3：一次 partition
    vector<int> p3 = {6, 1, 2, 7, 9, 3, 4, 5, 10, 8};
    int p = partition(p3, 0, (int)p3.size() - 1);
    cout << "练习3 pivot 下标=" << p << " 划分后: ";
    for (int x : p3) cout << x << " ";
    cout << "\n\n";

    // 练习 4：堆排序
    heapSort({23, 78, 45, 8, 32, 56});
    cout << "\n";

    // 练习 5：归并排序
    vector<int> p5 = {38, 27, 43, 3, 9, 82, 10};
    vector<int> tmp(p5.size());
    mergeSort(p5, tmp, 0, (int)p5.size() - 1, 0);
    printVec("归并排序结果", p5);
    cout << "\n";

    // 练习 6：计数排序
    vector<int> p6 = {4, 2, 2, 8, 3, 3, 1};
    countingSort(p6);

    // 附加：快排全流程 + 逆序对数验证
    vector<int> p7 = {6, 1, 2, 7, 9, 3, 4, 5, 10, 8};
    quickSort(p7, 0, (int)p7.size() - 1);
    printVec("快排结果", p7);
    return 0;
}

/* ===================== 预期输出 =====================
插入排序结果: 1 2 3 4 5 6
插入排序 比较=12 移动=14
插入排序结果: 1 2 3 4 5 6 7 8
逆序插入排序 比较=28 移动=35

冒泡第1趟后: 4 2 1 3 5
冒泡第2趟后: 2 1 3 4 5
冒泡第3趟后: 1 2 3 4 5
冒泡第4趟后: 1 2 3 4 5
无交换，提前终止

练习3 pivot 下标=7 划分后: 6 1 2 7 3 4 5 8 10 9

建堆后: 78 32 56 8 23 45
堆排第1趟后: 56 32 45 8 23 78
堆排第2趟后: 45 32 23 8 56 78
堆排第3趟后: 32 8 23 45 56 78
堆排序结果: 8 23 32 45 56 78

归并[0,1]后: 27 38 43 3 9 82 10
归并[2,3]后: 27 38 3 43 9 82 10
归并[0,3]后: 3 27 38 43 9 82 10
归并[4,5]后: 3 27 38 43 9 82 10
归并[4,6]后: 3 27 38 43 9 10 82
归并[0,6]后: 3 9 10 27 38 43 82
归并排序结果: 3 9 10 27 38 43 82

计数数组(前缀和后): 0 1 3 5 6 6 6 6 7
计数排序结果: 1 2 2 3 3 4 8
快排结果: 1 2 3 4 5 6 7 8 9 10
===================== 预期输出 ===================== */
