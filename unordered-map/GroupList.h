#pragma once
#include "Container.h"

// Базовый класс для однонаправленного и двунаправленного списков
class GroupList : public Container
{
public:
    struct Node {
        size_t size;
        void* data;
        Node* next;
    };
    GroupList(MemoryManager& mem) : Container(mem) {}
};
