#include <iostream>
#include <algorithm>

const int N = 1000005;

using namespace std;

double w[N];  // 古董的重量数组

int main() {
    double c;
    int n;

    cout << "请输入载重量C及古董个数n: " << endl;
    cin >> c >> n;
    cout << "请输入每个古董的重量， 用空格分开: " << endl;

    for (int i = 0; i < n; i++) {
        cin >> w[i]; // 输入每个物品的重量
    }

    // 按照古董数量升序排序
    sort(w, w + n);
    double tmp = 0.0;
    int ans = 0; // tmp 为已装载到船上的古董数量， ans 为已装载的古董个数
    for (int i = 0; i < n; i++) {
        tmp += w[i];
        if (tmp < c) {
            ans++;
        } else {
            break;
        }
    }
    cout << "能装入的古董最大数量为Ans=";
    cout << ans << endl;
    return 0;
}
