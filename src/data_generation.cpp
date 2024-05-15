#include <iostream>

using namespace std;
//
const int data_num = 10000;
const int data_dimension = 3;
const int count = 10;
const int max_range = 47;
const int max_increment = 97;
int main()
{
    freopen("data/data.txt", "w", stdout);
    cout << data_num * count << endl;
    srand((unsigned)time(NULL));
    for (int n = 0; n < count; n++)
    {
        // 先随机该轮次每个维度的范围和增量
        int *range = (int *)malloc(sizeof(int) * data_dimension);
        int *increment = (int *)malloc(sizeof(int) * data_dimension);
        for (int i = 0; i < data_dimension; i++)
        {
            range[i] = rand() % max_range+1;
            increment[i] = rand() % max_increment;
        }

        // 随机生成该轮次的数据
        for (int i = 0; i < data_num; i++)
        {
            int *temp = (int *)malloc(sizeof(int) * data_dimension * 2);
            for (int k = 0; k < data_dimension; k++) // 两个点表示一个高维空间区域
            {
                temp[k] = rand() % range[k] + increment[k];
                temp[k + data_dimension] = rand() % range[k] + increment[k];
                // 判断第二个点的每个维度坐标值都大于第一个点
                if (temp[k] > temp[k + data_dimension])
                    k--;
            }
            for (int j = 0; j < data_dimension * 2; j++)
            {
                cout << temp[j] << ' ';
            }
            cout << endl;
        }
    }
    return 0;
}