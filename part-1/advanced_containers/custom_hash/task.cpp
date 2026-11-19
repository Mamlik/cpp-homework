#include "task.h"

using namespace std;

void PopulateHashMap(
    unordered_map<SuperKey, string>& hashMap,
    const vector<pair<SuperKey, string>>& toAdd
) {
    for (const auto& pair : toAdd) {
        hashMap[pair.first] = pair.second;
    }
}
