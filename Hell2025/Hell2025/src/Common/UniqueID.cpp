#include "UniqueID.h" 

namespace UniqueID {
    uint64_t g_uniqueId = 1;

    uint64_t GetNext() {
        return g_uniqueId++;
    }
}