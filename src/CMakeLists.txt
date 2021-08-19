add_library(cuckoo_hash cuckoo_hash.hpp cuckoo_hash.cpp)

add_executable(main main.cpp)
target_link_libraries(main cuckoo_hash)
