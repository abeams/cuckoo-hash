#pragma once

#include <cstdint>
#include <memory>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <random>
#include <chrono>

#define NUM_HASHES 3
#define MAX_REBUILD_ATTEMPTS 100


class CuckooHashTable{
    public:
        CuckooHashTable(uint64_t elements_per_table, uint64_t key_size, uint64_t value_size, uint32_t seed);
        bool Add(const uint8_t key[], const uint8_t value[]);
        bool Get(const uint8_t key[], uint8_t out_value[]);
        bool Remove(const uint8_t key[], uint8_t out_value[]);
        uint64_t Hash(uint8_t hash_id, const uint8_t key[]);
        static constexpr uint8_t num_hashes = NUM_HASHES;
        static constexpr uint8_t max_rebuilds = MAX_REBUILD_ATTEMPTS;
        void Print();

    private:
        std::unique_ptr<uint8_t[]> _internal_table;
        uint64_t _table_size;
        uint64_t _key_size;
        uint64_t _value_size;
        std::mt19937 _gen;

        bool Add(const uint8_t key[], const uint8_t value[], uint8_t db[]);
        bool Rebuild(const uint8_t key[], const uint8_t value[]);
        bool GetExists(uint8_t table, uint64_t index, const uint8_t db[]);
        void SetExists(uint8_t table, uint64_t index, uint8_t val, uint8_t db[]);
        uint64_t GetIndexForKey(uint8_t table, uint64_t index);
        uint64_t GetIndexForValue(uint8_t table, uint64_t index);

        std::unique_ptr<uint8_t[]> GenerateShuffleTables();
        std::unique_ptr<uint8_t[]> _byte_table;



};


