#include<iostream>
using namespace std;
void QuickSort(int a[], int low, int high) {
   if(low >= high) return; // 如果区间不合法，直接返回
   int i = low, j = high;
   int pivot = a[low]; // 选择区间的第一个数作为基准
   while(i < j) {
       while(i < j && a[j] >= pivot) j--; // 从右向左找小于基准的数
       a[i] = a[j];
       while(i < j && a[i] <= pivot) i++; // 从左向右找大于基准的数
       a[j] = a[i];
   }
   a[i] = pivot; // 基准数归位
   QuickSort(a, low, i - 1); // 递归左子数组
   QuickSort(a, i + 1, high); // 递归右子数组
}
int main() {
   int a[] = {3, 5, 8, 2, 9, 1};
   int num = sizeof(a) / sizeof(a[0]);
   QuickSort(a, 0, num - 1);
   for(int i = 0; i < num; i++)
       cout << a[i] << " "; // 输出排序后的数组
   return 0;
}
