#ifndef R_TREE_HPP
#define R_TREE_HPP

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>
#include <cstddef>

using namespace std;

// 定义高维空间的点
template <size_t N>
struct Point
{
    array<double, N> coordinates;
    Point(const array<double, N> &_coordinates) : coordinates(_coordinates) {}
};

// 使用两个点定义高维空间的矩形
template <size_t N> // 数据维数
struct Rectangle
{
    array<double, N> minCoordinates;
    array<double, N> maxCoordinates;

    Rectangle(const array<double, N> &_minCoordinates, const array<double, N> &_maxCoordinates)
        : minCoordinates(_minCoordinates), maxCoordinates(_maxCoordinates) {}
};

// R-tree节点
template <size_t N> // 数据维数
class RTreeNode
{
public:
    Rectangle<N> rect;               // 节点的范围
    vector<Rectangle<N>> data;       // 子节点的索引，子节点的范围vector
    vector<RTreeNode<N> *> children; // 指向子节点的指针vector
    RTreeNode<N> *parent;            // 指向父节点的指针，在节点分割时使用

    RTreeNode(const Rectangle<N> &_rect, RTreeNode<N> *_parent = nullptr);

    // 添加一个子结点，并将子节点的父指针设为自己
    void addChild(RTreeNode<N> *child);
    // 添删除一个子结点
    void removeChild(RTreeNode<N> *child);
};

// R-tree
template <size_t N> // 数据维数
class RTree
{
private:
    size_t maxNodeSize;
    RTreeNode<N> *root;

public:
    RTree(size_t _maxNodeSize);

    // 插入节点
    void insert(const Rectangle<N> &rect);
    void insertHelper(RTreeNode<N> *node, const Rectangle<N> &rect);

    // 分裂节点，待修改
    void splitNode(RTreeNode<N> *node);
    // 选择分裂轴
    size_t chooseSplitAxis(RTreeNode<N> *node);

    // 计算合并后的体积与合并前体积的差
    double calculateEnlargement(const Rectangle<N> &parentRect, const Rectangle<N> &rect);
    // 计算高维空间矩形的超体积
    double calculateVolume(const Rectangle<N> &rect);
    // 计算合并两矩形后的新矩形范围
    Rectangle<N> mergeRect(const Rectangle<N> &rect1, const Rectangle<N> &rect2);
    // 更新节点范围，只更新当前节点
    void updateNodeRect(RTreeNode<N> *node);

    // 搜索数据
    vector<Rectangle<N>> search(const Rectangle<N> &queryRect);
    void searchHelper(RTreeNode<N> *node, const Rectangle<N> &queryRect, vector<Rectangle<N>> &result, vector<RTreeNode<N> *> &nodeQueue);

    // 判断两个矩形是否有包含关系
    bool intersects(const Rectangle<N> &rect1, const Rectangle<N> &rect2);
};

#include "./R-Tree.cpp"
#endif // R_TREE_HPP