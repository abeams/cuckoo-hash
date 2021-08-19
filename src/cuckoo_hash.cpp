#include "cuckoo_hash.hpp"


CuckooHashTable::CuckooHashTable(uint64_t elements_per_table, uint64_t key_size, uint64_t value_size, uint32_t seed):
_key_size(key_size),
_value_size(value_size),
_table_size(elements_per_table){
    _gen = std::mt19937(seed);
    _byte_table = GenerateShuffleTables();
    //Initialize hash tables
    _internal_table = std::make_unique<uint8_t[]>
        (CuckooHashTable::num_hashes * elements_per_table * (1 + key_size + value_size));

}

std::unique_ptr<uint8_t[]> CuckooHashTable::GenerateShuffleTables(){
    //Generate random tables
    std::unique_ptr<uint8_t[]> byte_table = std::make_unique<uint8_t[]>(CuckooHashTable::num_hashes * 256);
    for(uint32_t t = 0; t < CuckooHashTable::num_hashes; t++){
        for(uint32_t b = 0; b < 256; b++){
            byte_table.get()[t * 256 + b] = (uint8_t)b;
        }
        std::shuffle(byte_table.get() + t * 256, byte_table.get() + (t+1) * 256, _gen);
    }
    return byte_table;
}

uint64_t CuckooHashTable::Hash(uint8_t hash_id, const uint8_t key[]){
    uint8_t h;
    uint64_t retval;

    assert(hash_id < CuckooHashTable::num_hashes);
    for (size_t j = 0; j < sizeof(retval); j++) {
        // Change the first byte
        h = _byte_table.get()[(hash_id * 256) + (key[0] + j % 256)];
        for (size_t i = 1; i < _key_size; i++) {
            h = _byte_table.get()[(hash_id * 256) + (h ^ key[i])];
        }
    retval = ((retval << 8) | h);
    }

  return retval % _table_size;
} 

bool CuckooHashTable::Get(const uint8_t key[], uint8_t out_value[]){
    uint64_t hash;
    uint64_t index;
    for (uint32_t h = 0; h < CuckooHashTable::num_hashes; h++){
        hash = Hash(h, key);
        if(GetExists(h, hash, _internal_table.get())){
            if(std::equal(key, key + _key_size, _internal_table.get() + GetIndexForKey(h, hash))){
                index = GetIndexForValue(h, hash);
                std::copy(_internal_table.get() + index, _internal_table.get() + index + _value_size, out_value);
                return true;
            }
        }
    }
    return false;
}

bool CuckooHashTable::Remove(const uint8_t key[], uint8_t out_value[]){
    uint64_t hash;
    uint64_t index;
    for (uint32_t h = 0; h < CuckooHashTable::num_hashes; h++){
        hash = Hash(h, key);
        if(GetExists(h, hash, _internal_table.get())){
            if(std::equal(key, key + _key_size, _internal_table.get() + GetIndexForKey(h, hash))){
                index = GetIndexForValue(h, hash);
                std::copy(_internal_table.get() + index, _internal_table.get() + index + _value_size, out_value);
                SetExists(h, hash, 0, _internal_table.get());
                return true;
            }
        }
    }
    return false;
}

bool CuckooHashTable::Add(const uint8_t key[], const uint8_t value[]){
    if(Add(key, value, _internal_table.get())){
        return true;
    }
    //Rebuild
    #if DEBUG == 1
    std::cout << "Could not insert, table is full. Trying to rebuild..." << std::endl;
    #endif
    for(uint8_t att = 0; att < CuckooHashTable::max_rebuilds; att++){
        if(Rebuild(key, value)){
            return true;
        }
    }
    #if DEBUG == 1
    std::cout << "Rebuilding failed." << std::endl;
    #endif
    return false;
}

bool CuckooHashTable::Rebuild(const uint8_t key[], const uint8_t value[]){
    #if DEBUG == 1
        std::cout << "Attempting a rebuild." << std::endl;
    #endif
    //Generate new shuffle tables
    auto byte_table = GenerateShuffleTables();
    _byte_table.swap(byte_table);
    //Initialize an empty hash table
    std::unique_ptr<uint8_t[]> provisional_db = std::make_unique<uint8_t[]>
        (CuckooHashTable::num_hashes * _table_size * (1 + _key_size + _value_size));

    //Try to add all of the elements already in the table
    for(uint8_t t = 0; t < CuckooHashTable::num_hashes; t++){
        for(uint64_t i = 0; i < _table_size; i++){
            if(GetExists(t, i, _internal_table.get())){
                if(!Add(_internal_table.get() + GetIndexForKey(t, i),
                    _internal_table.get() + GetIndexForValue(t, i), provisional_db.get())){
                    
                    return false;
                }
            }
        }
    }
    //As well as the new element
    if(!Add(key, value, provisional_db.get())){
        return false;
    }

    //Rebuild was a success
    _internal_table.swap(provisional_db);
    //And the old tables will be deallocated when the function returns
    return true;
}

bool CuckooHashTable::Add(const uint8_t key[], const uint8_t value[], uint8_t db[]){
    uint8_t cur_h = 0;
    uint64_t hash;
    uint64_t key_index;
    uint64_t val_index;
    std::unique_ptr<uint8_t[]> tmp_key(std::make_unique<uint8_t[]>(_key_size));
    std::unique_ptr<uint8_t[]> tmp_value(std::make_unique<uint8_t[]>(_value_size));
    std::copy(key, key + _key_size, tmp_key.get());
    std::copy(value, value + _value_size, tmp_value.get());

    std::unique_ptr<uint8_t[]> tmp_key2(std::make_unique<uint8_t[]>(_key_size));
    std::unique_ptr<uint8_t[]> tmp_value2(std::make_unique<uint8_t[]>(_value_size));

    //Condition for detecting cycles. Can be optimized.
    for(uint64_t iter = 0; iter < _table_size * CuckooHashTable::num_hashes; iter++){
        hash = Hash(cur_h, key);
        key_index = GetIndexForKey(cur_h, hash);
        val_index = GetIndexForValue(cur_h, hash);
        if(!GetExists(cur_h, hash, db)){
            SetExists(cur_h, hash, 1, db);
            std::copy(tmp_key.get(), tmp_key.get() + _key_size, db + key_index);
            std::copy(tmp_value.get(), tmp_value.get() + _value_size, db + val_index);
            return true;
        }
        else{
            std::copy(db + key_index, db + key_index + _key_size, tmp_key2.get());
            std::copy(db + val_index, db + val_index + _value_size, tmp_value2.get());
            std::copy(tmp_key.get(), tmp_key.get() + _key_size, db + key_index);
            std::copy(tmp_value.get(), tmp_value.get() + _value_size, db + val_index);
            tmp_key.swap(tmp_key2);
            tmp_value.swap(tmp_value2);
            cur_h = (cur_h + 1) % CuckooHashTable::num_hashes;
        }
    }
    return false;
}


bool CuckooHashTable::GetExists(uint8_t table, uint64_t index, const uint8_t db[]){
    return db[table * (_table_size * (1 + _key_size + _value_size))
    + index * (1 + _key_size + _value_size)] != 0;
}

void CuckooHashTable::SetExists(uint8_t table, uint64_t index, uint8_t val, uint8_t db[]){
    db[table * (_table_size * (1 + _key_size + _value_size)) + 
        index * (1 + _key_size + _value_size)] = val;

}

uint64_t CuckooHashTable::GetIndexForKey(uint8_t table, uint64_t index){
    return table * (_table_size * (1 + _key_size + _value_size))
    + index * (1 + _key_size + _value_size) + 1;
}

uint64_t CuckooHashTable::GetIndexForValue(uint8_t table, uint64_t index){
    return table * (_table_size * (1 + _key_size + _value_size))
    + index * (1 + _key_size + _value_size) + 1 + _key_size;
}

void CuckooHashTable::Print(){
    //Only allow reasonably small tables to be printed
    assert(_table_size <= 100);
    assert(_key_size < 10);
    std::cout << "=============================" << std::endl;
    for(uint8_t t = 0; t < CuckooHashTable::num_hashes; t++){
        std::cout << "Table " << (int)t << std::endl;
        for(uint64_t i = 0; i < _table_size; i++){
            std::cout << "[" << (int)i << "]: ";
            if(GetExists(t, i, _internal_table.get())){
                for(uint64_t j = 0; j < _key_size; j++){
                    std::cout << (int)_internal_table.get()[GetIndexForKey(t, i) + j];
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << "=============================" << std::endl;

}




