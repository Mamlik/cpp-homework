#include "task.h"

using namespace std;

vector<int> HeapSort(const vector<int>& v) {
    std::priority_queue<int> data;
    for (int elem: v) {
        data.push(elem);
    }
    vector<int> ans(v.size(), 0);
    for (int i = v.size()-1; i >=0; --i){
        ans[i] = data.top();
        data.pop();
    }
    return ans;
}
