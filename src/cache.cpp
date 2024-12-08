#include "cache.h"

bool CacheEntry::operator==(const CacheEntry& other) const {
    return fd == other.fd && offset == other.offset;
}

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

void LRUCache::put(const CacheEntry& entry) {
    auto key = make_pair(entry.fd, entry.offset);

    if (cacheMap.find(key) != cacheMap.end()) {
        cacheList.erase(cacheMap[key]);
    } else if (cacheList.size() == capacity) {
        CacheEntry last = cacheList.back();
        auto lastKey = make_pair(last.fd, last.offset);
        cacheMap.erase(lastKey);
        cacheList.pop_back();
    }

    cacheList.push_front(entry);
    cacheMap[key] = cacheList.begin();
}

bool LRUCache::get(HANDLE fd, size_t offset, vector<char>& data) {
    auto key = make_pair(fd, offset);

    if (cacheMap.find(key) == cacheMap.end()) {
        return false;
    }

    auto it = cacheMap[key];
    data = it->data;
    cacheList.erase(it);
    cacheList.push_front(*it);
    cacheMap[key] = cacheList.begin();
    return true;
}