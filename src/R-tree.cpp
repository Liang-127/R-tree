#include "./R-tree.hpp"

using namespace std;

template <size_t N>
RTreeNode<N>::RTreeNode(const Rectangle<N> &_rect, RTreeNode<N> *_parent) : rect(_rect), parent(_parent)
{
}
template <size_t N>
void RTreeNode<N>::addChild(RTreeNode<N> *child)
{
    children.push_back(child);
    child->parent = this;
}
template <size_t N>
void RTreeNode<N>::removeChild(RTreeNode<N> *child)
{
    auto it = find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        children.erase(it);
        child->parent = nullptr;
    }
}

template <size_t N>
RTree<N>::RTree(size_t _maxNodeSize) : maxNodeSize(_maxNodeSize), root(nullptr)
{
}
template <size_t N>
void RTree<N>::insert(const Rectangle<N> &rect)
{
    if (!root) // 第一次插入时，根节点为空，先创建根节点再插入数据
        root = new RTreeNode<N>(rect);
    insertHelper(root, rect);
}
template <size_t N>
void RTree<N>::insertHelper(RTreeNode<N> *node, const Rectangle<N> &rect)
{
    // 在叶子节点插入的情况
    if (node->children.empty())
    {
        node->data.push_back(rect);
        if (node->data.size() > maxNodeSize)
        {
            // 分裂
            splitNode(node);
            return;
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
    updateNodeRect(node); // 插入数据后更新节点范围，这是不需要分裂的情况
}

template <size_t N>
void RTree<N>::splitNode(RTreeNode<N> *node)
{
    // 选择分裂轴，以中间位置作为分裂位置，创建新的节点
    size_t splitAxis = chooseSplitAxis(node);
    size_t splitIndex = node->data.size() / 2;
    RTreeNode<N> *newNode = new RTreeNode<N>(Rectangle<N>(node->data[0].minCoordinates, node->data[0].maxCoordinates), node->parent);
    // 由于叶子节点和非叶子节点的分裂不太一样，需要分别处理
    vector<pair<Rectangle<N>, RTreeNode<N> *>> data_children;
    if (!node->children.empty()) // 非叶子节点
    {
        for (size_t i = 0; i < node->data.size(); i++)
        {
            data_children.push_back(make_pair(node->data[i], node->children[i]));
        }
        // 根据分裂轴对数据集进行排序
        sort(data_children.begin(), data_children.end(), [splitAxis](const pair<Rectangle<N>, RTreeNode<N> *> &a, const pair<Rectangle<N>, RTreeNode<N> *> &b)
             { return a.first.minCoordinates[splitAxis] < b.first.minCoordinates[splitAxis]; });

        // 为新的节点分配数据项
        for (size_t i = 0; i < data_children.size(); i++)
        {
            newNode->data.push_back(data_children[i].first);
            newNode->children.push_back(data_children[i].second);
        }
        // 更新node节点的数据项列表
        node->data.clear();
        node->children.clear();
        for (size_t i = 0; i < splitIndex; i++)
        {
            node->data.push_back(newNode->data[i]);
            node->children.push_back(newNode->children[i]);
        }
        newNode->data.erase(newNode->data.begin(), newNode->data.begin() + splitIndex - 1);
        newNode->children.erase(newNode->children.begin(), newNode->children.begin() + splitIndex - 1);
    }
    else
    {
        // 根据分裂轴对数据集进行排序
        sort(node->data.begin(), node->data.end(), [splitAxis](const Rectangle<N> &a, const Rectangle<N> &b)
             { return a.minCoordinates[splitAxis] < b.minCoordinates[splitAxis]; });

        // 为新的节点分配数据项
        for (size_t i = 0; i < node->data.size(); i++)
        {
            newNode->data.push_back(node->data[i]);
        }
        // 更新node节点的数据项列表
        node->data.clear();
        for (size_t i = 0; i < splitIndex; i++)
        {
            node->data.push_back(newNode->data[i]);
        }
        newNode->data.erase(newNode->data.begin(), newNode->data.begin() + splitIndex - 1);
    }

    // 更新节点的范围
    updateNodeRect(node);
    updateNodeRect(newNode);
    // 如果node节点为root节点，则创建新的节点，并设为根节点
    if (!node->parent)
    {
        root = new RTreeNode<N>(
            *(new Rectangle<N>(*(new array<double, N>), *(new array<double, N>))));
        // 更新root节点的子节点列表
        root->addChild(node);
        root->addChild(newNode);
        // 更新root节点的范围
        updateNodeRect(root);
    }
    else // 更新父节点的子节点列表
    {
        // 此处要考虑父节点是否会分裂
        node->parent->addChild(newNode);
        // 叶子节点的分裂和非叶子节点不同？？？？？
        if (node->parent->children.size() > maxNodeSize)
        {
            splitNode(node->parent);
        }
    }
}

template <size_t N>
size_t RTree<N>::chooseSplitAxis(RTreeNode<N> *node)
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

template <size_t N>
double RTree<N>::calculateEnlargement(const Rectangle<N> &parentRect, const Rectangle<N> &rect)
{
    double volumeBefore = calculateVolume(parentRect);
    double volumeAfter = calculateVolume(mergeRect(parentRect, rect));
    return volumeAfter - volumeBefore;
}
template <size_t N>
double RTree<N>::calculateVolume(const Rectangle<N> &rect)
{
    double volume = 1.0;
    for (size_t i = 0; i < N; ++i)
    {
        volume *= (rect.maxCoordinates[i] - rect.minCoordinates[i]);
    }
    return volume;
}
template <size_t N>
Rectangle<N> RTree<N>::mergeRect(const Rectangle<N> &rect1, const Rectangle<N> &rect2)
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
template <size_t N>
void RTree<N>::updateNodeRect(RTreeNode<N> *node)
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
    array<double, N> minCoords;
    array<double, N> maxCoords;
    minCoords.fill(numeric_limits<double>::max()); // 初值应该为最大
    maxCoords.fill(numeric_limits<double>::min()); // 初值应该为最小
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

template <size_t N>
vector<Rectangle<N>> RTree<N>::search(const Rectangle<N> &queryRect)
{
    vector<Rectangle<N>> result;
    vector<RTreeNode<N> *> nodeQueue;
    searchHelper(root, queryRect, result, nodeQueue);
    return result;
}
template <size_t N>
void RTree<N>::searchHelper(RTreeNode<N> *node, const Rectangle<N> &queryRect, vector<Rectangle<N>> &result, vector<RTreeNode<N> *> &nodeQueue)
{
    if (!node)
        return;
    if (intersects(node->rect, queryRect))
    {
        for (size_t i = 0; i < node->data.size(); i++)
        {
            if (intersects(node->data[i], queryRect))
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
            searchHelper(temp, queryRect, result, nodeQueue);
        }
    }
    if (node->children.empty())
        return; // 当搜索到叶子节点时即可停止
}

template <size_t N>
bool RTree<N>::intersects(const Rectangle<N> &rect1, const Rectangle<N> &rect2)
{
    for (size_t i = 0; i < N; i++)
    {
        if (rect1.maxCoordinates[i] < rect2.minCoordinates[i] || rect1.maxCoordinates[i] < rect2.maxCoordinates[i] || rect1.minCoordinates[i] > rect2.maxCoordinates[i] || rect1.minCoordinates[i] > rect2.minCoordinates[i])
        {
            return false;
        }
    }
    return true;
}
