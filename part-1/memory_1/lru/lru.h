#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

template <typename K, typename V>
class LruCache {
public:
    LruCache(size_t max_size) : size_(max_size) {}

    void Put(const K& key, const V& value) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            cache_.erase(it->second);
            map_.erase(it);
        }

        cache_.push_front(make_pair(key, value));
        map_[key] = cache_.begin();

        if (cache_.size() > size_) {
            auto last = cache_.back();
            map_.erase(last.first);
            cache_.pop_back();
        }
    }

    bool Get(const K& key, V* value) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            auto pair = *(it->second);
            cache_.erase(it->second);
            map_.erase(it);
            cache_.push_front(pair);
            //it->second = cache_.begin(); //??
            map_[key] = cache_.begin();
            *value = pair.second;
            return true;
        } else {
            return false;
        }
    }

private:
    size_t size_;
    list<pair<K, V>> cache_;
    unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_;
};
