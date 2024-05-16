#include "./R-tree.hpp"
#include <iostream>
#include <array>
#include <chrono>

using namespace std;

const size_t dimension = 2;     // 数据的维数
const size_t MaxNodeSize = 100; // 子节点的最大数

int main()
{
    RTree<dimension> rtree(MaxNodeSize); // 创建一个dimension维R树,每个节点最大容量为MaxNodeSize
    // 读取并插入高维数据对象
    FILE *file = freopen("data\\data.txt", "r", stdin);
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
    // fclose(file);

    // 范围查询
    freopen("data\\searchfor.txt", "r", stdin);
    int search_data_number;
    cin >> search_data_number;
    cout << "The number of search test data is " << search_data_number << endl;
    int64_t *result_time = (int64_t *)malloc(sizeof(int64_t) * search_data_number);
    for (size_t count = 0; count < search_data_number; count++)
    {
        array<double, dimension> minCoordinates;
        array<double, dimension> maxCoordinates;
        cout << "input a rect for search:";
        for (size_t j = 0; j < dimension; j++)
            cin >> minCoordinates[j];
        for (size_t j = 0; j < dimension; j++)
            cin >> maxCoordinates[j];
        cout << endl;
        for (size_t j = 0; j < dimension; j++)
            cout << minCoordinates[j] << ' ';
        for (size_t j = 0; j < dimension; j++)
            cout << maxCoordinates[j] << ' ';
        cout << endl;
        Rectangle<dimension> queryRect(minCoordinates, maxCoordinates);
        // 获取当前时间点
        auto start = std::chrono::steady_clock::now();
        vector<Rectangle<dimension>> result = rtree.search(queryRect);
        // 获取当前时间点
        auto end = std::chrono::steady_clock::now();
        // 计算时间差 输出计时结果
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        result_time[count] = duration.count();
        std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;
        // 输出结果
        cout << "result number: " << result.size() << endl;
        // cout << "Search result:" << endl;
        // for (const Rectangle<3> &rect : result)
        // {
        //     cout << "(";
        //     for (size_t i = 0; i < dimension; ++i)
        //     {
        //         cout << rect.minCoordinates[i];
        //         if (i < dimension - 1)
        //             cout << ",";
        //     }
        //     cout << ")" << ' ' << "(";
        //     for (size_t i = 0; i < dimension; ++i)
        //     {
        //         cout << rect.maxCoordinates[i];
        //         if (i < dimension - 1)
        //             cout << ",";
        //     }
        //     cout << ")" << endl;
        // }
        cout << endl;
    }
    // 计算平均查找时长
    int64_t ave_time = 0;
    for (size_t i = 0; i < search_data_number; i++)
    {
        ave_time += result_time[i];
    }
    ave_time /= search_data_number;
    cout << ave_time << endl;
    return 0;
}
