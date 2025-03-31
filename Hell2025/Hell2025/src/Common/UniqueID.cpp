#include "UniqueID.h" 
#include <mutex>

namespace UniqueID {
    uint64_t g_uniqueId = 1;

    uint64_t GetNext() {
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        return g_uniqueId++;
    }
}