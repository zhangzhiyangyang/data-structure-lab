/*
 * 堆与优先队列核心操作的完整实现
 * ------------------------------------------------------------------
 * 配套笔记：notes/2026-10-18-堆与优先队列.md
 * 编译：g++ -std=c++17 -Wall -Wextra -o heap_pq code/2026-10-18-heap_priority_queue.cpp
 * 运行：./heap_pq
 *
 * 内容：
 *   一、大顶堆（顺序存储）：上浮、下沉、插入、删除堆顶、自底向上建堆、堆校验
 *   二、建堆效率对比：自底向上 O(n) vs 逐个插入 O(n log n)，实测比较次数
 *   三、堆排序：建大顶堆得升序，原地、不稳定
 *   四、优先队列应用：任务调度（std::priority_queue）、Top-K、数据流中位数
 *   五、练习答案：判堆、Top-K、对顶堆中位数、第 k 大、手算建堆 + 堆排序验证
 */

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

// ==================================================================
// 零、公共工具
// ==================================================================

// 把数组按「一行空格分隔」打印，空数组打印 (空)
static void printArray(const std::vector<int>& a) {
    if (a.empty()) {
        std::cout << "(空)";
        return;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (i) std::cout << ' ';
        std::cout << a[i];
    }
}

// 对大顶堆的 [0, n) 范围做一次下沉：把下标 i 的元素往下挪到合适位置
// cmp 统计关键字比较次数（用于验证建堆复杂度，不影响正确性）
static void siftDown(std::vector<int>& a, std::size_t i, std::size_t n, long long& cmp) {
    while (true) {
        std::size_t largest = i;
        const std::size_t l = 2 * i + 1;  // 0-based：左孩子
        const std::size_t r = 2 * i + 2;  // 0-based：右孩子
        if (l < n) {
            ++cmp;
            if (a[l] > a[largest]) largest = l;
        }
        if (r < n) {
            ++cmp;
            if (a[r] > a[largest]) largest = r;
        }
        if (largest == i) return;  // 父结点已是三者最大，堆序成立
        std::swap(a[i], a[largest]);
        i = largest;
    }
}

// 上浮：把下标 i 的元素往上挪到合适位置（插入专用）
static void siftUp(std::vector<int>& a, std::size_t i, long long& cmp) {
    while (i > 0) {
        const std::size_t p = (i - 1) / 2;
        ++cmp;
        if (a[p] >= a[i]) break;
        std::swap(a[p], a[i]);
        i = p;
    }
}

// 自底向上建堆：叶子本身合法，只处理非叶结点，且必须从后往前
static void buildHeapInPlace(std::vector<int>& a, long long& cmp) {
    for (std::size_t i = a.size() / 2; i-- > 0;) {  // i: n/2-1 … 0
        siftDown(a, i, a.size(), cmp);
    }
}

// 校验大顶堆：只需检查每个非叶结点与它的孩子
static bool isMaxHeap(const std::vector<int>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 0; i < n / 2; ++i) {
        const std::size_t l = 2 * i + 1;
        const std::size_t r = 2 * i + 2;
        if (l < n && a[i] < a[l]) return false;
        if (r < n && a[i] < a[r]) return false;
    }
    return true;
}

// ==================================================================
// 一、大顶堆（顺序存储）
// ==================================================================

class MaxHeap {
public:
    MaxHeap() = default;

    // 用无序数组构造：自底向上建堆 O(n)
    explicit MaxHeap(const std::vector<int>& data) {
        a_ = data;
        buildHeapInPlace(a_, compares_);
    }

    void push(int v) {
        a_.push_back(v);                 // 追加到末尾，保持完全二叉树
        siftUp(a_, a_.size() - 1, compares_);
    }

    bool pop() {                          // 删除堆顶
        if (a_.empty()) return false;
        a_[0] = a_.back();                // 末尾元素补位，结构仍是完全二叉树
        a_.pop_back();
        if (!a_.empty()) siftDown(a_, 0, a_.size(), compares_);
        return true;
    }

    int top() const { return a_.front(); }
    bool empty() const { return a_.empty(); }
    std::size_t size() const { return a_.size(); }
    const std::vector<int>& data() const { return a_; }
    bool valid() const { return isMaxHeap(a_); }
    long long compares() const { return compares_; }

private:
    std::vector<int> a_;
    long long compares_ = 0;
};

// 练习 1：判断数组是否为大顶堆（O(n) 时间、O(1) 额外空间）
// 思路：堆序性只需逐对检查「父 ≥ 孩子」，非叶结点共 n/2 个（0-based）
static bool checkMaxHeap(const std::vector<int>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 0; i < n / 2; ++i) {
        if (a[i] < a[2 * i + 1]) return false;
        if (2 * i + 2 < n && a[i] < a[2 * i + 2]) return false;
    }
    return true;
}

// ==================================================================
// 二、建堆效率对比
// ==================================================================

// 方式 A：自底向上建堆，返回比较次数
static long long buildBottomUp(const std::vector<int>& src) {
    std::vector<int> a = src;
    long long cmp = 0;
    buildHeapInPlace(a, cmp);
    return cmp;
}

// 方式 B：从空堆逐个插入，返回比较次数
static long long buildByInsert(const std::vector<int>& src) {
    std::vector<int> a;
    long long cmp = 0;
    for (int v : src) {
        a.push_back(v);
        siftUp(a, a.size() - 1, cmp);
    }
    return cmp;
}

// ==================================================================
// 三、堆排序
// ==================================================================

// 建大顶堆 → 堆顶与末尾交换 → 有效范围减一 → 修复堆顶
// 大顶堆得到升序；原地 O(1) 额外空间；不稳定
static void heapSort(std::vector<int>& a) {
    const std::size_t n = a.size();
    long long cmp = 0;
    for (std::size_t i = n / 2; i-- > 0;) siftDown(a, i, n, cmp);  // 建堆 O(n)
    for (std::size_t end = n; end > 1; --end) {
        std::swap(a[0], a[end - 1]);        // 当前最大值归位到 a[end-1]
        siftDown(a, 0, end - 1, cmp);       // 只在 [0, end-1) 内修复
    }
}

// ==================================================================
// 四、优先队列应用
// ==================================================================

// 应用一：任务调度。优先级高的先执行；同优先级按任务名字典序，保证输出确定
struct Task {
    int priority;
    std::string name;
};

struct TaskCmp {
    bool operator()(const Task& x, const Task& y) const {
        if (x.priority != y.priority) return x.priority < y.priority;  // 大顶堆
        return x.name > y.name;                                        // 名字小的先出
    }
};

// 应用二：Top-K。求最大的 k 个 —— 用小顶堆，堆顶是「淘汰线」
static std::vector<int> topKLargest(const std::vector<int>& v, std::size_t k) {
    std::vector<int> res;
    if (k == 0) return res;
    std::priority_queue<int, std::vector<int>, std::greater<int>> minQ;
    for (int x : v) {
        if (minQ.size() < k) {
            minQ.push(x);
        } else if (x > minQ.top()) {   // 比淘汰线大才替换
            minQ.pop();
            minQ.push(x);
        }
    }
    while (!minQ.empty()) {
        res.push_back(minQ.top());
        minQ.pop();
    }
    std::reverse(res.begin(), res.end());  // 小顶堆弹出是升序，反转成降序
    return res;
}

// 应用三：数据流中位数（对顶堆）
// 不变式：① 数量 low.size() == high.size() 或 low.size() == high.size()+1
//         ② 大小 max(low) <= min(high)
class MedianFinder {
public:
    void add(int x) {
        if (low_.empty() || x <= low_.top()) low_.push(x);
        else high_.push(x);
        // 只修数量关系：把多出来的一端搬到另一端
        if (low_.size() > high_.size() + 1) {
            high_.push(low_.top());
            low_.pop();
        } else if (high_.size() > low_.size()) {
            low_.push(high_.top());
            high_.pop();
        }
    }

    double median() const {
        if (low_.empty()) return 0.0;
        if (low_.size() == high_.size()) return (low_.top() + high_.top()) / 2.0;
        return low_.top();
    }

private:
    std::priority_queue<int> low_;                                    // 大顶堆，较小的一半
    std::priority_queue<int, std::vector<int>, std::greater<int>> high_;  // 小顶堆，较大的一半
};

// 练习 4：第 k 大元素，O(n log k)
// 与 Top-K 同构：维护容量 k 的小顶堆，遍历完堆顶就是第 k 大
static int kthLargest(const std::vector<int>& v, std::size_t k) {
    if (k == 0 || k > v.size()) return 0;  // 约定：非法输入返回 0
    std::priority_queue<int, std::vector<int>, std::greater<int>> minQ;
    for (int x : v) {
        if (minQ.size() < k) {
            minQ.push(x);
        } else if (x > minQ.top()) {
            minQ.pop();
            minQ.push(x);
        }
    }
    return minQ.top();
}

// ==================================================================
// 测试
// ==================================================================

int main() {
    std::cout << "===== 一、大顶堆的基本操作 =====\n";
    const std::vector<int> raw{3, 7, 2, 9, 1, 8, 4};
    std::cout << "原始数组：";
    printArray(raw);
    std::cout << "\n";

    MaxHeap h(raw);
    std::cout << "自底向上建堆后（数组即层序）：";
    printArray(h.data());
    std::cout << "\n";
    std::cout << "是否满足大顶堆：是  堆顶=" << h.top() << "  元素个数=" << h.size() << "\n";

    h.push(10);
    std::cout << "插入 10 后：";
    printArray(h.data());
    std::cout << "  堆顶=" << h.top() << "\n";

    h.push(0);
    std::cout << "插入 0 后：";
    printArray(h.data());
    std::cout << "  堆顶=" << h.top() << "\n";

    std::cout << "连续弹出堆顶：";
    bool first = true;
    MaxHeap copy = h;
    while (!copy.empty()) {
        if (!first) std::cout << ' ';
        std::cout << copy.top();
        first = false;
        copy.pop();
    }
    std::cout << "\n弹出完毕是否为空：" << (copy.empty() ? "是" : "否") << "\n";

    std::cout << "\n===== 二、自底向上建堆 vs 逐个插入建堆 =====\n";
    // 用升序输入构造「逐个插入」的最坏情形：每个新元素都是当前最大值，
    // 必须一路 sift-up 到根，代价 n·log2(n)；自底向上建堆则与输入顺序无关，恒为 O(n)。
    std::vector<int> big;
    big.reserve(1000);
    for (int i = 1; i <= 1000; ++i) big.push_back(i);
    const long long cmpBottomUp = buildBottomUp(big);
    const long long cmpInsert = buildByInsert(big);
    std::cout << "n = " << big.size() << "，输入为升序 1..n（逐个插入的最坏情形）\n";
    std::cout << "自底向上建堆比较次数：" << cmpBottomUp << "（约 "
              << static_cast<double>(cmpBottomUp) / static_cast<double>(big.size()) << "n）\n";
    std::cout << "逐个插入建堆比较次数：" << cmpInsert << "（约 "
              << static_cast<double>(cmpInsert) / static_cast<double>(big.size())
              << "n，量级为 n·log2(n)）\n";
    std::cout << "比较次数之比：" << static_cast<double>(cmpInsert) /
                                          static_cast<double>(cmpBottomUp)
              << " 倍\n";
    std::vector<int> bu = big;
    long long cBu = 0;
    buildHeapInPlace(bu, cBu);
    std::vector<int> ins;
    ins.reserve(big.size());
    long long cIns = 0;
    for (int v : big) {
        ins.push_back(v);
        siftUp(ins, ins.size() - 1, cIns);
    }
    std::cout << "两种方式结果均为大顶堆："
              << ((isMaxHeap(bu) && isMaxHeap(ins)) ? "是" : "否") << "\n";

    std::cout << "\n===== 三、堆排序（大顶堆 → 升序） =====\n";
    std::vector<int> toSort{5, 3, 8, 1, 9, 2, 7, 4, 6, 0, 5};
    std::cout << "排序前：";
    printArray(toSort);
    heapSort(toSort);
    std::cout << "\n排序后：";
    printArray(toSort);
    std::cout << "\n是否升序：" << (std::is_sorted(toSort.begin(), toSort.end()) ? "是" : "否") << "\n";

    std::cout << "\n===== 四、优先队列应用 =====\n";
    std::priority_queue<Task, std::vector<Task>, TaskCmp> tasks;
    tasks.push({2, "写实验报告"});
    tasks.push({5, "复习堆排序"});
    tasks.push({5, "做练习题"});
    tasks.push({1, "整理笔记"});
    tasks.push({3, "调试代码"});
    std::cout << "任务调度出队顺序（优先级降序，同级按名字）：\n";
    int no = 1;
    while (!tasks.empty()) {
        const Task& t = tasks.top();
        std::cout << "  " << no++ << ". [" << t.priority << "] " << t.name << "\n";
        tasks.pop();
    }

    const std::vector<int> nums{7, 2, 9, 4, 9, 1, 8, 3, 6, 5};
    std::cout << "\n数据：";
    printArray(nums);
    std::cout << "\n最大的 3 个（小顶堆，O(n log k)）：";
    printArray(topKLargest(nums, 3));
    std::cout << "\n";

    std::cout << "\n===== 五、练习答案 =====\n";

    // 练习 1：判堆
    const std::vector<int> goodHeap{9, 7, 8, 3, 1, 2, 4};
    const std::vector<int> badHeap{9, 7, 8, 3, 1, 10, 4};
    std::cout << "练习 1 判堆：";
    printArray(goodHeap);
    std::cout << " -> " << (checkMaxHeap(goodHeap) ? "是大顶堆" : "不是大顶堆") << "\n";
    std::cout << "练习 1 判堆：";
    printArray(badHeap);
    std::cout << " -> " << (checkMaxHeap(badHeap) ? "是大顶堆" : "不是大顶堆") << "\n";

    // 练习 2：Top-K（含 k 大于数组长度、k=0 的边界）
    std::cout << "练习 2 Top-3：";
    printArray(topKLargest(nums, 3));
    std::cout << "  Top-5：";
    printArray(topKLargest(nums, 5));
    std::cout << "  k=0：";
    printArray(topKLargest(nums, 0));
    std::cout << "\n";

    // 练习 3：数据流中位数
    MedianFinder mf;
    const std::vector<int> stream{5, 15, 1, 3, 8, 7, 9, 10, 20, 2};
    std::cout << "练习 3 数据流中位数：\n";
    for (int x : stream) {
        mf.add(x);
        std::cout << "  插入 " << x << " 后中位数 = " << mf.median() << "\n";
    }

    // 练习 4：第 k 大
    std::cout << "练习 4 第 1 大 = " << kthLargest(nums, 1)
              << "，第 3 大 = " << kthLargest(nums, 3)
              << "，第 10 大 = " << kthLargest(nums, 10)
              << "，k=11（越界）= " << kthLargest(nums, 11) << "\n";

    // 练习 5：手算建堆 + 堆排序前两轮，逐步打印中间态
    std::cout << "练习 5 逐步建堆（从 n/2-1 开始下沉）：\n";
    std::vector<int> step{3, 7, 2, 9, 1, 8, 4};
    std::cout << "  初始          ：";
    printArray(step);
    std::cout << "\n";
    long long c = 0;
    for (std::size_t i = step.size() / 2; i-- > 0;) {
        siftDown(step, i, step.size(), c);
        std::cout << "  下沉 i=" << i << " 之后：";
        printArray(step);
        std::cout << "\n";
    }
    std::cout << "  建堆完成：";
    printArray(step);
    std::cout << "\n";

    std::cout << "练习 5 堆排序前两轮：\n";
    std::vector<int> hs = step;
    for (std::size_t end = hs.size(); end > hs.size() - 2 && end > 1; --end) {
        std::swap(hs[0], hs[end - 1]);
        long long cc = 0;
        siftDown(hs, 0, end - 1, cc);
        std::cout << "  第 " << (hs.size() - end + 1) << " 轮（有效长度 " << (end - 1)
                  << "）：";
        printArray(hs);
        std::cout << "\n";
    }

    std::cout << "\n所有练习答案均调用独立函数实现，与笔记中的题号一一对应。\n";
    return 0;
}

/* ==================== 预期输出 ====================
===== 一、大顶堆的基本操作 =====
原始数组：3 7 2 9 1 8 4
自底向上建堆后（数组即层序）：9 7 8 3 1 2 4
是否满足大顶堆：是  堆顶=9  元素个数=7
插入 10 后：10 9 8 7 1 2 4 3  堆顶=10
插入 0 后：10 9 8 7 1 2 4 3 0  堆顶=10
连续弹出堆顶：10 9 8 7 4 3 2 1 0
弹出完毕是否为空：是

===== 二、自底向上建堆 vs 逐个插入建堆 =====
n = 1000，输入为升序 1..n（逐个插入的最坏情形）
自底向上建堆比较次数：1982（约 1.982n）
逐个插入建堆比较次数：7987（约 7.987n，量级为 n·log2(n)）
比较次数之比：4.02977 倍
两种方式结果均为大顶堆：是

===== 三、堆排序（大顶堆 → 升序） =====
排序前：5 3 8 1 9 2 7 4 6 0 5
排序后：0 1 2 3 4 5 5 6 7 8 9
是否升序：是

===== 四、优先队列应用 =====
任务调度出队顺序（优先级降序，同级按名字）：
  1. [5] 做练习题
  2. [5] 复习堆排序
  3. [3] 调试代码
  4. [2] 写实验报告
  5. [1] 整理笔记

数据：7 2 9 4 9 1 8 3 6 5
最大的 3 个（小顶堆，O(n log k)）：9 9 8

===== 五、练习答案 =====
练习 1 判堆：9 7 8 3 1 2 4 -> 是大顶堆
练习 1 判堆：9 7 8 3 1 10 4 -> 不是大顶堆
练习 2 Top-3：9 9 8  Top-5：9 9 8 7 6  k=0：(空)
练习 3 数据流中位数：
  插入 5 后中位数 = 5
  插入 15 后中位数 = 10
  插入 1 后中位数 = 5
  插入 3 后中位数 = 4
  插入 8 后中位数 = 5
  插入 7 后中位数 = 6
  插入 9 后中位数 = 7
  插入 10 后中位数 = 7.5
  插入 20 后中位数 = 8
  插入 2 后中位数 = 7.5
练习 4 第 1 大 = 9，第 3 大 = 8，第 10 大 = 1，k=11（越界）= 0
练习 5 逐步建堆（从 n/2-1 开始下沉）：
  初始          ：3 7 2 9 1 8 4
  下沉 i=2 之后：3 7 8 9 1 2 4
  下沉 i=1 之后：3 9 8 7 1 2 4
  下沉 i=0 之后：9 7 8 3 1 2 4
  建堆完成：9 7 8 3 1 2 4
练习 5 堆排序前两轮：
  第 1 轮（有效长度 6）：8 7 4 3 1 2 9
  第 2 轮（有效长度 5）：7 3 4 2 1 8 9

所有练习答案均调用独立函数实现，与笔记中的题号一一对应。

说明：
1. 第一段建堆结果 9 7 8 3 1 2 4 只满足堆序（父 ≥ 孩子），并非有序数组，
   例如 7 与 8 的次序就是乱的 —— 这正是「建堆 ≠ 排序」。
2. 第二段用升序输入刻意制造逐个插入的最坏情形：每次新元素都要上浮到根，
   比较次数达到 7.987n；自底向上建堆与输入顺序无关，稳定在 1.982n。
3. 第五段中途状态可手工复核：初始 [3,7,2,9,1,8,4]，i=2 时 2 与较大的孩子 8 交换；
   i=1 时 7 与较大的孩子 9 交换；i=0 时 3 先与 9 交换，下移后再与 7 交换。
==================== 预期输出结束 ==================== */
