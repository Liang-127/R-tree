#include "./R-tree.hpp"
#include <iostream>
#include <array>

using namespace std;

const size_t dimension = 3;   // 数据的维数
const size_t MaxNodeSize = 110; // 子节点的最大数

int main()
{
    RTree<dimension> rtree(MaxNodeSize); // 创建一个dimension维R树,每个节点最大容量为MaxNodeSize
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
    Rectangle<3> queryRect({80, 49, 45}, {86, 50, 71});
    vector<Rectangle<3>> result = rtree.search(queryRect);
    //输出结果
    cout << "Search result:" << endl;
    for (const Rectangle<3> &rect : result)
    {
        cout << "(";
        for (size_t i = 0; i < dimension; ++i)
        {
            cout << rect.minCoordinates[i];
            if (i < dimension-1)
                cout << ",";
        }
        cout << ")" << ' ' << "(";
        for (size_t i = 0; i < dimension; ++i)
        {
            cout << rect.maxCoordinates[i];
            if (i < dimension-1)
                cout << ",";
        }
        cout << ")" << endl;
    }
    return 0;
}
