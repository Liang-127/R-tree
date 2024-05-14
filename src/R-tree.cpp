#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>

using namespace std;

const size_t dimension = 3;   // 数据的维数
const size_t MaxNodeSize = 3; // 子节点的最大数

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

    RTreeNode(const Rectangle<N> &_rect, RTreeNode<N> *_parent = nullptr) : rect(_rect), parent(_parent) {}

    // 添加一个子结点，并将子节点的父指针设为自己
    void addChild(RTreeNode<N> *child)
    {
        children.push_back(child);
        child->parent = this;
    }
    // 添删除一个子结点
    void removeChild(RTreeNode<N> *child)
    {
        auto it = find(children.begin(), children.end(), child);
        if (it != children.end())
        {
            children.erase(it);
            child->parent = nullptr;
        }
    }
};

// R-tree
template <size_t N> // 数据维数
class RTree
{
private:
    size_t maxNodeSize;
    RTreeNode<N> *root;

public:
    RTree(size_t _maxNodeSize) : maxNodeSize(_maxNodeSize), root(nullptr) {}

    // 插入节点
    void insert(const Rectangle<N> &rect)
    {
        if (!root)
            root = new RTreeNode<N>(rect);
        else
            insertHelper(root, rect);
    }
    void insertHelper(RTreeNode<N> *node, const Rectangle<N> &rect)
    {
        // 在叶子节点插入的情况
        if (node->children.empty())
        {
            node->data.push_back(rect);
            if (node->data.size() > maxNodeSize)
            {
                // 分裂？？？应该需要改一下
                splitNode(node);
            }
        }
        else // 有两层的情况，第一次分裂发生后只会进入该分支
        {
            // 选择合适的子节点插入，选择的判断标准：合并后体积增量最小
            double minEnlargement = numeric_limits<double>::max();
            RTreeNode<N> *bestChild = nullptr;
            // 只会选择不为空的子节点，所以在有三层的情况下也不会导致不平衡
            for (RTreeNode<N> *child : node->children)
            {
                double enlargement = calculateEnlargement(child->rect, rect);
                if (enlargement < minEnlargement)
                {
                    minEnlargement = enlargement;
                    bestChild = child;
                }
            }
            insertHelper(bestChild, rect);
        }
    }

    // 分裂节点，待修改
    void splitNode(RTreeNode<N> *node)
    {
        // 选择分裂轴
        size_t splitAxis = chooseSplitAxis(node);
        // 根据分裂轴对数据集进行排序
        sort(node->data.begin(), node->data.end(), [splitAxis](const Rectangle<N> &a, const Rectangle<N> &b)
             { return a.minCoordinates[splitAxis] < b.minCoordinates[splitAxis]; });
        // 以中间位置作为分裂位置
        size_t splitIndex = node->data.size() / 2;
        // 创建新的节点并分配数据项
        RTreeNode<N> *newNode = new RTreeNode<N>(Rectangle<N>(node->data[splitIndex].minCoordinates, node->data[splitIndex].maxCoordinates), node->parent);
        for (size_t i = splitIndex; i < node->data.size(); i++)
        {
            newNode->data.push_back(node->data[i]);
        }
        // 更新node节点的数据项列表
        node->data.erase(node->data.begin(), node->data.end());
        for (size_t i = 0; i < splitIndex; i++)
        {
            node->data.push_back(node->data[i]); // 写错了，边写边改是什么鬼？？
        }
        // 更新node节点的范围
        updateNodeRect(node);
        // 更新新节点的范围
        updateNodeRect(newNode);

        // 如果node节点为root节点，则创建新的节点，并设为根节点
        if (!node->parent)
        {
            root = new RTreeNode<dimension>(
                *(new Rectangle<dimension>(*(new array<double, dimension>), *(new array<double, dimension>))));
            // 更新root节点的子节点列表
            root->addChild(node);
            root->addChild(newNode);
            // 更新root节点的范围
            updateNodeRect(root);
        }
        else // 更新父节点的子节点列表
        {
            // 此处要考虑父节点是否会分裂
            // 叶子节点的分裂和非叶子节点不同？？？？？
            if (node->parent->children.size() > maxNodeSize)
            {
                splitNode(node->parent);
            }
        }
    }

    // 选择分裂轴
    size_t chooseSplitAxis(RTreeNode<N> *node)
    {
        double minOverlap = numeric_limits<double>::max(); // 记录重合长度最小值
        size_t splitAxis = 0;                              // 选择分裂的维度

        for (size_t axis = 0; axis < N; ++axis)
        {
            // 对每个维度计算投影区域
            vector<pair<double, double>> projections; // 每个数据在该维度的两个坐标数据
            for (const Rectangle<N> &rect : node->data)
            {
                projections.push_back({rect.minCoordinates[axis], rect.maxCoordinates[axis]});
            }
            // 对投影区域进行排序，按第一个坐标值从小到大
            sort(projections.begin(), projections.end(), [](const pair<double, double> &a, const pair<double, double> &b)
                 { return a.first < b.first; });
            // 计算分裂位置
            double overlap = 0; // 记录每个维度重叠长度之和
            for (size_t i = 0; i < projections.size() - 1; i++)
            {
                for (size_t j = i + 1; j < projections.size(); j++)
                {
                    if (projections[i].second > projections[j].first)
                    {
                        overlap += (projections[i].second - projections[j].first);
                    }
                    else
                        break;
                }
            }
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                splitAxis = axis;
            }
        }
        return splitAxis;
    }

    // 计算合并后的体积与合并前体积的差
    double calculateEnlargement(const Rectangle<N> &parentRect, const Rectangle<N> &rect)
    {
        double volumeBefore = calculateVolume(parentRect);
        double volumeAfter = calculateVolume(mergeRect(parentRect, rect));
        return volumeAfter - volumeBefore;
    }
    // 计算高维空间矩形的超体积
    double calculateVolume(const Rectangle<N> &rect)
    {
        double volume = 1.0;
        for (size_t i = 0; i < N; ++i)
        {
            volume *= (rect.maxCoordinates[i] - rect.minCoordinates[i]);
        }
        return volume;
    }
    // 计算合并两矩形后的新矩形范围
    Rectangle<N> mergeRect(const Rectangle<N> &rect1, const Rectangle<N> &rect2)
    {
        array<double, N> minCoords;
        array<double, N> maxCoords;
        for (size_t i = 0; i < N; ++i)
        {
            minCoords[i] = min(rect1.minCoordinates[i], rect2.minCoordinates[i]);
            maxCoords[i] = max(rect1.maxCoordinates[i], rect2.maxCoordinates[i]);
        }
        return Rectangle<N>(minCoords, maxCoords);
    }
    // 更新节点范围，只更新当前节点
    void updateNodeRect(RTreeNode<N> *node)
    {
        // 需要修改的节点为非叶子节点时，需要先更新data
        if (!node->children.empty())
        {
            node->data.clear();
            for (size_t i = 0; i < node->children.size(); i++)
            {
                node->data.push_back(node->children[i]->rect);
            }
        }
        // 根据data计算新的范围
        // minCoords,maxCoords初值应该为最小值才行???????
        array<double, N> minCoords;
        array<double, N> maxCoords;
        for (Rectangle<N> rect : node->data)
        {
            for (size_t i = 0; i < N; i++)
            {
                minCoords[i] = min(minCoords[i], rect.minCoordinates[i]);
                maxCoords[i] = max(maxCoords[i], rect.maxCoordinates[i]);
            }
        }
        node->rect = Rectangle<N>(minCoords, maxCoords);
    }

    // 搜索数据
    vector<Rectangle<N>> search(const Rectangle<N> &queryRect)
    {
        vector<Rectangle<N>> result;
        searchHelper(root, queryRect, result);
        return result;
    }
    void searchHelper(RTreeNode<N> *node, const Rectangle<N> &queryRect, vector<Rectangle<N>> &result)
    {
        if (!node)
            return;
        if (intersects(node->rect, queryRect) && intersects(queryRect, node->rect))
        {
            vector<RTreeNode<N> *> nodeQueue;
            for (size_t i = 0; i < node->data.size(); i++)
            {
                if (intersects(queryRect, node->data[i]) && intersects(node->data[i], queryRect))
                {
                    // 如果是node是叶子节点，将结果存入result
                    if (node->children.empty())
                        result.push_back(node->data[i]);
                    // 如果是node不是叶子节点，将对应子节点存入队列中，等待递归查询
                    else
                        nodeQueue.push_back(node->children[i]);
                }
            }
            while (!nodeQueue.empty())
            {
                RTreeNode<N> *temp = nodeQueue[0];
                nodeQueue.erase(nodeQueue.begin());
                searchHelper(temp, queryRect, result);
            }
        }
        if (node->children.empty())
            return; // 当搜索到叶子节点时即可停止
    }

    // 判断两个矩形是否有包含关系
    bool intersects(const Rectangle<N> &rect1, const Rectangle<N> &rect2)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (rect1.maxCoordinates[i] < rect2.minCoordinates[i] || rect1.minCoordinates[i] > rect2.maxCoordinates[i])
            {
                return false;
            }
        }
        return true;
    }
};

int main()
{
    RTree<MaxNodeSize> rtree(MaxNodeSize); // 创建一个三维R树，每个节点最多存储4个数据项
    // 读取并插入高维数据对象
    freopen("data\\data.txt", "r", stdin);
    int data_num;
    cin >> data_num; // 读取数据个数
    for (int i = 0; i < data_num; i++)
    {
        array<double, dimension> temp_arr1;
        array<double, dimension> temp_arr2;
        for (int j = 0; j < dimension; j++)
            cin >> temp_arr1[j];
        for (int j = 0; j < dimension; j++)
            cin >> temp_arr2[j];
        rtree.insert(Rectangle<dimension>(temp_arr1, temp_arr2));
    }
    // 范围查询
    Rectangle<3> queryRect({8, 60, 80}, {8, 60, 80});
    vector<Rectangle<3>> result = rtree.search(queryRect);
    cout << "Search result:" << endl;
    for (const Rectangle<3> &rect : result)
    {
        cout << "(";
        for (size_t i = 0; i < 3; ++i)
        {
            cout << rect.minCoordinates[i];
            if (i < 2)
                cout << ",";
        }
        cout << ")" << endl
             << "(";
        for (size_t i = 0; i < 3; ++i)
        {
            cout << rect.maxCoordinates[i];
            if (i < 2)
                cout << ",";
        }
        cout << ")" << endl;
    }
    return 0;
}
