# data-structure-lab

数据结构课程实验与学习笔记仓库（C/C++，Visual Studio 工程）。

## 目录结构

```
data-structure-lab
├── linear list/     线性表实验：顺序表、单链表、双向链表、循环链表、双向循环链表
├── stack.cpp/       栈实验：顺序栈 sqstack、链栈 listack
├── queue/           队列实验：环形队列 sqqueue、链队列 lqueue
├── code/            学习笔记配套的独立示例代码（g++ 可直接编译）
├── notes/           数据结构学习笔记（每日更新，索引见 notes/README.md）
└── daily-log/       每日学习日志
```

## 学习路线

- [x] 线性表（顺序表、单链表、双向链表、循环链表）
- [x] 栈（顺序栈、链栈）
- [x] 队列（环形队列、链队列）
- [x] 树与二叉树
- [x] 二叉搜索树 / 平衡二叉树
- [x] 堆与优先队列
- [x] 图（存储结构、DFS、BFS、最小生成树、最短路）
- [x] 查找（顺序、折半、散列）
- [x] 排序（插入、交换、选择、归并、基数）
- [x] 二叉树的线索化
- [x] 并查集（QuickFind / QuickUnion）

## 编译方式

各实验目录为独立的 Visual Studio 工程，直接打开对应 `.sln` 即可；
`code/` 下的示例代码用 g++ 单文件编译：

```bash
g++ -std=c++17 -Wall -Wextra -o demo code/2026-10-15-binary_tree.cpp
./demo
```

## 说明

- 仓库只跟踪源码与文档，`*.vs/`、`x64/` 等构建产物和 IDE 缓存不入库（见 `.gitignore`）。
- 笔记与配套代码按日期命名，最新进度见 [`notes/README.md`](notes/README.md)。
