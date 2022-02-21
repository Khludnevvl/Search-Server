#pragma once

#include <map>
#include <mutex>
#include <set>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap
{
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct MiniMap
    {
        std::mutex map_mutex;
        std::map<Key, Value> map_store;
    };

    struct Access
    {

        Access() = delete;

        Access(const Key &key, MiniMap &map) : guard(map.map_mutex), ref_to_value(map.map_store[key]) {}
        std::lock_guard<std::mutex> guard;
        Value &ref_to_value;
    };

    explicit ConcurrentMap(size_t maps_count) : storage_(maps_count){};

    Access operator[](const Key &key)
    {
        uint64_t which_map = key % storage_.size();
        return Access(key, storage_[which_map]);
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
        std::map<Key, Value> result;
        for (auto &it : storage_)
        {
            std::lock_guard<std::mutex> g(it.map_mutex);
            result.merge(it.map_store);
        }

        return result;
    }

    void erase(const Key &key)
    {
        uint64_t which_map = key % storage_.size();
        std::lock_guard<std::mutex> g(storage_[which_map].map_mutex);
        storage_[which_map].map_store.erase(key);
    }

private:
    std::vector<MiniMap> storage_;
};

template <typename Value>
class ConcurrentSet
{
public:
    struct MiniSet
    {
        std::mutex set_mutex;
        std::set<Value> set_store;
    };

    struct Access
    {

        Access() = delete;

        Access(const Value &val, MiniSet &set) : guard(set.set_mutex), ref_to_value(set.set_store.at(val)) {}
        std::lock_guard<std::mutex> guard;
        Value &ref_to_value;
    };

    explicit ConcurrentSet(size_t sets_count) : storage_(sets_count){};

    std::set<Value> BuildOrdinaryset()
    {
        std::set<Value> result;
        for (auto &it : storage_)
        {
            std::lock_guard<std::mutex> g(it.set_mutex);
            result.merge(it.set_store);
        }

        return result;
    }

    void insert(const Value val)
    {
        uint64_t which_set = static_cast<int>(val[0]) % storage_.size();
        std::lock_guard<std::mutex> g(storage_[which_set].set_mutex);
        storage_[which_set].set_store.insert(val);
    }
    void erase(const Value &val)
    {
        uint64_t which_set = static_cast<int>(val[0]) % storage_.size();
        std::lock_guard<std::mutex> g(storage_[which_set].set_mutex);
        storage_[which_set].set_store.erase(val);
    }

private:
    std::vector<MiniSet> storage_;
};