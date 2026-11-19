#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

using namespace std;

vector<shared_ptr<string>> DeDuplicate(vector<unique_ptr<string>>& in) {
    vector<shared_ptr<string>> out;
    unordered_map<string, shared_ptr<string>> unique_vals;
    for (auto& ptr : in) {
        if (!ptr) continue;
        auto it = unique_vals.find(*ptr);
        if (it != unique_vals.end()) { //elem found
            out.push_back(it->second);
        } else{
            auto shared = make_shared<string>(*ptr);
            out.push_back(shared);
            unique_vals[*ptr] = shared;
        }
    }
    return out;
}

vector<unique_ptr<string>> Duplicate(vector<shared_ptr<string>> in) {
    vector<unique_ptr<string>> out;
    for (const auto& it : in) {
        if (it){
            out.push_back(make_unique<string>(*it));
        }
    }
    return out;
}