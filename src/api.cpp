#include "api.h"
#include "cache.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

constexpr size_t BLOCK_SIZE = 4096;

LRUCache globalCache(100);

HANDLE lab2_open(const char *filename) {
    HANDLE fd = CreateFileA(
        filename, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );
    return fd == INVALID_HANDLE_VALUE ? NULL : fd;
}

int lab2_close(HANDLE fd) {
    if (fd == NULL || fd == INVALID_HANDLE_VALUE) {
        return -1;
    }
    return CloseHandle(fd) ? 0 : -1;
}

ssize_t lab2_read(HANDLE fd, void *buffer, size_t count) {
    size_t offset = SetFilePointer(fd, 0, NULL, FILE_CURRENT);

    vector<char> data(count);
    if (globalCache.get(fd, offset, data)) {
        memcpy(buffer, data.data(), min(count, data.size()));
        SetFilePointer(fd, count, NULL, FILE_CURRENT);
        return min(count, data.size());
    }

    DWORD bytesRead;
    vector<char> temp(count);
    if (!ReadFile(fd, temp.data(), count, &bytesRead, NULL)) {
        return -1;
    }

    globalCache.put({fd, offset, temp});

    memcpy(buffer, temp.data(), bytesRead);
    return bytesRead;
}

ssize_t lab2_write(HANDLE fd, const void *buffer, size_t count) {
    size_t offset = SetFilePointer(fd, 0, NULL, FILE_CURRENT);

    DWORD bytesWritten;
    if (!WriteFile(fd, buffer, count, &bytesWritten, NULL)) {
        return -1;
    }

    // Обновление кэша
    vector<char> temp((const char*)buffer, (const char*)buffer + bytesWritten);
    globalCache.put({fd, offset, temp});

    return bytesWritten;
}

// Перемещение указателя в файле
off_t lab2_lseek(HANDLE fd, off_t offset, int whence) {
    DWORD moveMethod;
    switch (whence) {
        case SEEK_SET: moveMethod = FILE_BEGIN; break;
        case SEEK_CUR: moveMethod = FILE_CURRENT; break;
        case SEEK_END: moveMethod = FILE_END; break;
        default: return -1;
    }
    return SetFilePointer(fd, offset, NULL, moveMethod);
}

// Синхронизация данных с диском
int lab2_fsync(HANDLE fd) {
    return FlushFileBuffers(fd) ? 0 : -1;
}
