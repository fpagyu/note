// 阿里巴巴与四十大盗--背包问题
#include <iostream>
#include <algorithm>

using namespace std;
const int M = 1000005;

struct three {
    double w; // 每个宝物的重量
    double v; // 每个宝物的价值
    double p; // 性价比
} s[M];


bool cmp(three a, three b) {
    return a.p > b.p; // 根据宝物的单位价值从大到小排序 
}

int main() {
    int n; // 表示有n个宝物
    double m; // 表示毛驴的承受能力
    cout << "请输入宝物数量n以及毛驴的承载能力m: " << endl;
    cin >> n >> m;
    cout << "请输入每个宝物的重量和价值， 用空格分开: " << endl;
    for (int i=0; i < n; i++) {
        cin >> s[i].w >> s[i].v;
        s[i].p = s[i].v / s[i].w;
    }

    sort(s, s+n, cmp);
    double sum = 0.0; // sum表示贪心记录运走宝物的价值之和
    for (int i = 0; i < n; i++) {
        // 按照排好的顺序贪心
        if (m > s[i].w) {
            m -= s[i].w;
            sum += s[i].v;
        } else {
            // 如果宝物重量大于毛驴剩下的承载能力
            sum += m*s[i].p; // 部分装入
            break;
        }
    }
    cout << "装入宝物的最大价值Maximun value=" << sum << endl;
    return 0;
}
