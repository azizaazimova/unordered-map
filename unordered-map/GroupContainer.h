#pragma once
#include "Container.h"

// Базовый класс для некоторой группы абстрактных контейнеров
class GroupContainer : public Container
{  
public:
	size_t numelems; //сколько элементов
    GroupContainer(MemoryManager& mem) : Container(mem) {}

    //хеш функция Set
    //int setHashFunc(void* value, size_t size);
    static unsigned int setHashFunc(void* value, size_t size) {
        char* val = (char*)value;
        unsigned long long int result = 0;
        for (unsigned int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j++) {
                int a = 1 << j;
                if ((*((int*)(val)) & a) >= 1) {
                    unsigned long long int pow = 1 << (i * 8 + j);
                    result += 2 * pow;
                }
            }
            val++;
        }
        return result;
    }
    static size_t tableHash(void* key, size_t keySize, size_t num)
    {
        size_t hash = 5381;
        unsigned char* data = (unsigned char*)key;

        for (size_t i = 0; i < keySize; i++) {
            hash = ((hash << 5) + hash) + data[i];
        }

        return hash % num;
    }

    // Функция возвращает значение, равное количеству элементов в контейнере.
    int size() { return numelems; }

    // Функция возвращает значение, равное максимальной вместимости контейнера в байтах.
    size_t max_bytes() { return _memory.size(); }

    // Если контейнер пуст возвращает true, иначе false
	bool empty() { return numelems == 0; }

};
