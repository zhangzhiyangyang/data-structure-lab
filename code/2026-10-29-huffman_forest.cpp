// ============================================================
// Huffman 树与 Huffman 编码、树与森林转换 —— 课程第 18~19 期配套代码
// 功能：
//   (1) 用"每次选两个最小的合并"构建哈夫曼树（顺序静态三叉链表存储）
//   (2) 两种方法求 WPL（根→叶 DFS 与 叶→根回溯），互为校验
//   (3) 生成哈夫曼编码（回溯法 / DFS 法互验），编码一段文本再解码还原
//   (4) 最优性实验：{1,2,4,8} 哈夫曼 vs 等深方案；字符频率的压缩比
//   (5) 树/森林 ↔ 二叉树（孩子兄弟法）转换，并做完整往返验证
// 编译：g++ -std=c++17 -Wall -Wextra -o huffman_forest.exe huffman_forest.cpp
// ============================================================
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// 第一部分：哈夫曼树（顺序存储 / 静态三叉链表）
// ------------------------------------------------------------
// 教材经典做法：开 2n 大小的数组（0 号弃用），HT[1..n] 放原始叶子，
// HT[n+1..2n-1] 放每次合并出来的新结点，HT[2n-1] 最后是根。
// 每个结点除了权值，还记 parent —— 有了 parent 就能"从叶往根走"，
// 这是顺序存储在这里最大的好处。
// ============================================================
struct HTNode {
    int weight;          // 权值
    int parent;          // 双亲下标，0 = 还没有双亲（还在"待合并"池子里）
    int lchild, rchild;  // 左右孩子下标，0 = 没有（叶子两个都是 0）
};

class HuffmanTree {
public:
    // 用一组权值建树：每轮在"还没有双亲"的结点里选最小的两个合并
    explicit HuffmanTree(const std::vector<int>& weights) : n_(static_cast<int>(weights.size())) {
        int total = 2 * n_;                 // 需要 2n-1 个结点，0 号弃用所以开 2n
        ht_.assign(total, HTNode{0, 0, 0, 0});
        for (int i = 1; i <= n_; ++i) {
            ht_[i].weight = weights[i - 1]; // 叶子：只有权值，指针全 0
        }
        // 一共合并 n-1 次，产生 HT[n+1] ~ HT[2n-1]
        for (int i = n_ + 1; i <= 2 * n_ - 1; ++i) {
            int s1 = 0, s2 = 0;
            selectTwo(i - 1, s1, s2);       // 在 1..i-1 里选权最小、且 parent==0 的两个
            ht_[s1].parent = i;             // 两个孩子认爹
            ht_[s2].parent = i;
            ht_[i].weight = ht_[s1].weight + ht_[s2].weight;  // 新结点权 = 两权之和
            ht_[i].lchild = s1;             // 约定：小的当左孩子（左 0 右 1）
            ht_[i].rchild = s2;
        }
    }

    int root() const { return 2 * n_ - 1; }
    int leafCount() const { return n_; }
    int weightOf(int leaf) const { return ht_[leaf].weight; }  // 叶子权值（打印用）

    // ---------- 方法 A：根→叶 DFS 求 WPL ----------
    // 走到叶子就累加 weight × depth。O(n)，只访问每个结点一次。
    int wplByDfs() const {
        int sum = 0;
        dfsWpl(root(), 0, sum);
        return sum;
    }

    // ---------- 方法 B：每个叶子沿 parent 链爬回根，数出深度 ----------
    // O(n × 树高)。两种方法结果必须一致，用来互为校验。
    int wplByParent() const {
        int sum = 0;
        for (int i = 1; i <= n_; ++i) {     // 遍历所有叶子
            int depth = 0;
            for (int p = ht_[i].parent; p != 0; p = ht_[p].parent) {
                ++depth;                    // 每认一次爹，深度 +1，直到走到根
            }
            sum += ht_[i].weight * depth;
        }
        return sum;
    }

    // ---------- 编码方法 A：叶→根回溯 ----------
    // 对每个叶子沿 parent 往上走，每步看"我是父结点的左孩子还是右孩子"，
    // 左记 '0' 右记 '1'。因为是从后往前收集的，最后要 reverse。
    std::vector<std::string> codesByUpward() const {
        std::vector<std::string> codes(n_ + 1);
        for (int i = 1; i <= n_; ++i) {
            std::string rev;                 // 逆序收集
            int cur = i;                     // cur 沿 parent 一路往上爬
            for (int p = ht_[cur].parent; p != 0; cur = p, p = ht_[p].parent) {
                rev += (ht_[p].lchild == cur) ? '0' : '1';
            }
            codes[i] = std::string(rev.rbegin(), rev.rend());  // reverse 得到正序
        }
        return codes;
    }

    // ---------- 编码方法 B：根→叶 DFS ----------
    // 路径上左走记 '0' 右走记 '1'，到叶子即得编码。两种方法应完全一致。
    std::vector<std::string> codesByDfs() const {
        std::vector<std::string> codes(n_ + 1);
        std::string path;                    // 当前路径（边走边拼）
        dfsCodes(root(), path, codes);
        return codes;
    }

    // ---------- 编码文本：symbols[i] 是叶子 i 对应的字符 ----------
    // 逐字符查表拼接。前缀码保证拼接结果可以无歧义地解码回来。
    std::string encode(const std::string& text, const std::string& symbols) const {
        std::vector<std::string> codes = codesByDfs();
        std::string bits;
        for (char c : text) {
            for (int i = 1; i <= n_; ++i) {
                if (symbols[i - 1] == c) {   // 找到该字符对应的叶子
                    bits += codes[i];
                    break;
                }
            }
        }
        return bits;
    }

    // ---------- 解码：从根出发读比特流，'0' 左 '1' 右，到叶子输出并回根 ----------
    // 前缀码的性质保证"到达叶子"那一刻恰好读完一个字符，不会有歧义。
    std::string decode(const std::string& bits) const {
        std::string text;
        int cur = root();
        for (char b : bits) {
            cur = (b == '0') ? ht_[cur].lchild : ht_[cur].rchild;
            if (ht_[cur].lchild == 0) {      // 叶子（哈夫曼树叶子左右孩子都是 0）
                text += symbolOf(cur);
                cur = root();                // 回到根，准备解码下一个字符
            }
        }
        return text;
    }

    // 打印静态三叉链表（HT 表），检查树建得对不对
    void printTable() const {
        std::cout << "  编号  权值  parent  lchild  rchild\n";
        for (int i = 1; i <= 2 * n_ - 1; ++i) {
            std::cout << "  " << i << "\t" << ht_[i].weight << "\t" << ht_[i].parent
                      << "\t" << ht_[i].lchild << "\t" << ht_[i].rchild << '\n';
        }
    }

private:
    std::vector<HTNode> ht_;  // ht_[0] 弃用
    int n_;                   // 叶子个数

    // 在 1..end 里选出 parent==0 且权最小的两个下标 s1 < s2（并列时任取靠前的）
    // 每轮 O(n) 扫描、共 n-1 轮 → 建树 O(n^2)；用小根堆可优化到 O(n log n)
    void selectTwo(int end, int& s1, int& s2) const {
        s1 = s2 = 0;
        for (int i = 1; i <= end; ++i) {
            if (ht_[i].parent != 0) continue;        // 关键：已合并的不能再来！
            if (s1 == 0 || ht_[i].weight < ht_[s1].weight) {
                s2 = s1;                             // 原来的最小退居第二
                s1 = i;
            } else if (s2 == 0 || ht_[i].weight < ht_[s2].weight) {
                s2 = i;                              // 只比第二小还小，顶掉第二小
            }
        }
    }

    void dfsWpl(int idx, int depth, int& sum) const {
        if (ht_[idx].lchild == 0) {          // 叶子：哈夫曼树里没有度为 1 的结点，
            sum += ht_[idx].weight * depth;  // 所以 lchild==0 就能断定是叶子
            return;
        }
        dfsWpl(ht_[idx].lchild, depth + 1, sum);  // 左边下去，深度 +1
        dfsWpl(ht_[idx].rchild, depth + 1, sum);
    }

    void dfsCodes(int idx, std::string& path, std::vector<std::string>& codes) const {
        if (ht_[idx].lchild == 0) {
            codes[idx] = path;               // 到叶子：当前路径就是它的编码
            return;
        }
        path += '0';                         // 左走记 0
        dfsCodes(ht_[idx].lchild, path, codes);
        path.pop_back();
        path += '1';                         // 右走记 1
        dfsCodes(ht_[idx].rchild, path, codes);
        path.pop_back();
    }

    // 叶子的"字符"就用它的下标映射，便于演示解码（A 对应叶子 1，以此类推）
    char symbolOf(int leaf) const { return static_cast<char>('A' + leaf - 1); }
};

// ============================================================
// 第二部分：树/森林 ↔ 二叉树（孩子兄弟表示法）
// ------------------------------------------------------------
// 一句话规则："左孩子、右兄弟"——
//   结点的第一个孩子 → 二叉树里的左孩子
//   结点的下一个兄弟 → 二叉树里的右孩子
// 森林转二叉树时，各树根互为兄弟，所以根的右链上依次挂着第 2、3…棵树。
// ============================================================
struct CSNode {
    char    val;
    CSNode* firstChild;   // 第一个孩子（转二叉树后 = 左孩子）
    CSNode* nextSibling;  // 下一个兄弟（转二叉树后 = 右孩子）
};

// 造一个新结点，孩子和兄弟先置空
CSNode* csNew(char val) {
    return new CSNode{val, nullptr, nullptr};
}

// 缩进打印一棵树（按"孩子兄弟"两条链遍历，depth 控制缩进）
void csPrint(CSNode* node, int depth) {
    if (node == nullptr) return;
    for (int i = 0; i < depth; ++i) std::cout << "    ";
    std::cout << node->val << '\n';
    csPrint(node->firstChild, depth + 1);   // 孩子们：缩进一层
    csPrint(node->nextSibling, depth);      // 兄弟：与自己在同一层
}

// 森林 → 二叉树：把各树根用 nextSibling 串成一条右链，返回第一棵树的根
// （每棵树内部本来就是孩子兄弟结构，它天然就是二叉树）
CSNode* forestToBinary(const std::vector<CSNode*>& roots) {
    for (int i = 0; i + 1 < static_cast<int>(roots.size()); ++i) {
        roots[i]->nextSibling = roots[i + 1];  // T1 根的"兄弟" = T2 根
    }
    return roots.empty() ? nullptr : roots[0];
}

// 二叉树 → 森林（逆变换）：沿根的右链切开，每个结点各成一棵树的根
std::vector<CSNode*> binaryToForest(CSNode* bt) {
    std::vector<CSNode*> roots;
    CSNode* cur = bt;
    while (cur != nullptr) {
        CSNode* next = cur->nextSibling;  // 先记住下一个树根
        cur->nextSibling = nullptr;       // 切断与后面兄弟的连接
        roots.push_back(cur);
        cur = next;
    }
    return roots;
}

// 二叉树先序遍历（根-左-右），用来核对转换结果
void csPreorder(CSNode* node, std::string& out) {
    if (node == nullptr) return;
    out += node->val;
    csPreorder(node->firstChild, out);
    csPreorder(node->nextSibling, out);
}

// ============================================================
// 测试
// ============================================================
int main() {
    std::cout << "==================== (1) 构建哈夫曼树 ====================\n";
    // 权值 {3,5,8,11,14}，n=5，共 2*5-1=9 个结点
    HuffmanTree hf({3, 5, 8, 11, 14});
    std::cout << "权值 {3,5,8,11,14}，n=5，共 " << 2 * hf.leafCount() - 1
              << " 个结点（性质：n 个叶子的哈夫曼树 = 2n-1 个结点）\n";
    hf.printTable();

    std::cout << "\n==================== (2) 两种方法求 WPL ====================\n";
    std::cout << "DFS 法  WPL = " << hf.wplByDfs() << "\n";
    std::cout << "回溯法  WPL = " << hf.wplByParent() << "（两种方法应一致）\n";

    std::cout << "\n==================== (3) 编码与解码 ====================\n";
    std::vector<std::string> up = hf.codesByUpward();
    std::vector<std::string> dfs = hf.codesByDfs();
    bool same = true;
    for (int i = 1; i <= hf.leafCount(); ++i) {
        if (up[i] != dfs[i]) same = false;  // 两种方法互为校验
        std::cout << "叶子" << i << "(权" << hf.weightOf(i) << ") 回溯法: " << up[i]
                  << "  DFS法: " << dfs[i] << "\n";
    }
    std::cout << "两种编码方法结果" << (same ? "一致" : "不一致!") << "\n";

    // 编码一段文本再解码还原，验证前缀码无歧义
    std::string text = "ABCDEEDCBA";
    std::string symbols = "ABCDE";           // 叶子 1~5 对应 A~E
    std::string bits = hf.encode(text, symbols);
    std::string restored = hf.decode(bits);
    std::cout << "原文   : " << text << "\n";
    std::cout << "编码   : " << bits << "（" << bits.size() << " 位）\n";
    std::cout << "解码   : " << restored << "\n";
    std::cout << "解码与原文" << (restored == text ? "一致，前缀码无歧义" : "不一致!!") << "\n";

    std::cout << "\n==================== (4) 哈夫曼最优性实验 ====================\n";
    // 实验 A：权值 {1,2,4,8}。等深方案（都放深度 2）WPL = 2*(1+2+4+8) = 30，
    // 哈夫曼树会把大权 8 提到深度 1，验证 WPL 更小。
    HuffmanTree hf2({1, 2, 4, 8});
    int equalDepthWpl = 2 * (1 + 2 + 4 + 8);
    std::cout << "权值 {1,2,4,8}：等深方案 WPL = " << equalDepthWpl
              << "，哈夫曼 WPL = " << hf2.wplByDfs() << "（省 "
              << equalDepthWpl - hf2.wplByDfs() << "）\n";

    // 实验 B：字符 {a,b,c,d} 频率 {8,2,2,1}，共 13 个字符。
    // 等长编码 2 位/字符 → 26 位；哈夫曼把高频的 a 放最近 → 总位数更少。
    HuffmanTree hf3({8, 2, 2, 1});
    int total = 8 + 2 + 2 + 1;
    int fixedBits = total * 2;
    int hufBits = hf3.wplByDfs();            // WPL 恰好就是"总编码位数"！
    std::cout << "频率 {8,2,2,1}（共 " << total << " 字符）：等长 " << fixedBits
              << " 位，哈夫曼 " << hufBits << " 位，压缩率 "
              << (100.0 * hufBits / fixedBits) << "%\n";

    std::cout << "\n==================== (5) 森林 <-> 二叉树 ====================\n";
    // 手工搭一个森林：T1=A(B,C)  T2=D(E(G),F)  T3=H
    CSNode* b = csNew('B');
    CSNode* c = csNew('C');
    CSNode* a = csNew('A');
    a->firstChild = b;  b->nextSibling = c;  // A 的孩子 B、C
    CSNode* g = csNew('G');
    CSNode* f = csNew('F');
    CSNode* e = csNew('E');
    CSNode* d = csNew('D');
    e->firstChild = g;  e->nextSibling = f;  // E 的孩子 G、兄弟 F
    d->firstChild = e;                        // D 的孩子 E
    CSNode* h = csNew('H');

    std::vector<CSNode*> forest = {a, d, h};
    std::cout << "原森林（缩进表示）：\n";
    for (CSNode* root : forest) {
        csPrint(root, 0);                     // 逐棵打印，每棵之间空一行
        std::cout << "\n";
    }

    CSNode* bt = forestToBinary(forest);      // 森林 → 二叉树（根串右链）
    std::string pre;
    csPreorder(bt, pre);
    std::cout << "转成二叉树后的先序遍历：" << pre << "\n";
    std::cout << "二叉树形态（缩进，左=孩子 右=兄弟）：\n";
    csPrint(bt, 0);

    std::vector<CSNode*> back = binaryToForest(bt);  // 二叉树 → 森林（沿右链切开）
    std::cout << "还原后的森林：\n";
    for (CSNode* root : back) {
        csPrint(root, 0);
        std::cout << "\n";
    }
    // 往返验证：再转一次二叉树，先序应与第一次完全相同
    CSNode* bt2 = forestToBinary(back);
    std::string pre2;
    csPreorder(bt2, pre2);
    std::cout << "往返验证：还原森林再转回二叉树，先序 = " << pre2
              << (pre2 == pre ? "（与第一次一致，往返无损）" : "（不一致!!）") << "\n";

    return 0;  // 演示程序，结点不手动释放，交给操作系统回收
}

/* ==================== 预期输出（2026-10-29 实际运行核对） ====================
==================== (1) 构建哈夫曼树 ====================
权值 {3,5,8,11,14}，n=5，共 9 个结点（性质：n 个叶子的哈夫曼树 = 2n-1 个结点）
  编号  权值  parent  lchild  rchild
  1	3	6	0	0
  2	5	6	0	0
  3	8	7	0	0
  4	11	8	0	0
  5	14	8	0	0
  6	8	7	1	2
  7	16	9	3	6
  8	25	9	4	5
  9	41	0	7	8

==================== (2) 两种方法求 WPL ====================
DFS 法  WPL = 90
回溯法  WPL = 90（两种方法应一致）

==================== (3) 编码与解码 ====================
叶子1(权3) 回溯法: 010  DFS法: 010
叶子2(权5) 回溯法: 011  DFS法: 011
叶子3(权8) 回溯法: 00  DFS法: 00
叶子4(权11) 回溯法: 10  DFS法: 10
叶子5(权14) 回溯法: 11  DFS法: 11
两种编码方法结果一致
原文   : ABCDEEDCBA
编码   : 010011001011111000011010（24 位）
解码   : ABCDEEDCBA
解码与原文一致，前缀码无歧义

==================== (4) 哈夫曼最优性实验 ====================
权值 {1,2,4,8}：等深方案 WPL = 30，哈夫曼 WPL = 25（省 5）
频率 {8,2,2,1}（共 13 字符）：等长 26 位，哈夫曼 21 位，压缩率 80.7692%

==================== (5) 森林 <-> 二叉树 ====================
原森林（缩进表示）：
A
    B
    C

D
    E
        G
    F

H

转成二叉树后的先序遍历：ABCDEGFH
二叉树形态（缩进，左=孩子 右=兄弟）：
A
    B
    C
D
    E
        G
    F
H
还原后的森林：
A
    B
    C

D
    E
        G
    F

H

往返验证：还原森林再转回二叉树，先序 = ABCDEGFH（与第一次一致，往返无损）
================================================================ */
