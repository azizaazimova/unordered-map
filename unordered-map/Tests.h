#pragma once
#include "Container.h"
#include "MemoryManager.h"

// =============================================================================
//  Считающий менеджер памяти.
//  Для критерия "оценка 4": проверяем, что вся память (и пользовательские
//  объекты, и внутренние структуры контейнера) выделяется/освобождается через
//  менеджер и что после очистки нет утечек (liveBlocks() == 0).
// =============================================================================
class CountingMem : public MemoryManager
{
private:
    long long _allocCalls;
    long long _freeCalls;
    long long _liveBlocks;
public:
    CountingMem(size_t sz);

    void* allocMem(size_t sz) override;
    void  freeMem(void* ptr) override;

    long long allocCalls() const { return _allocCalls; }
    long long freeCalls()  const { return _freeCalls; }
    long long liveBlocks() const { return _liveBlocks; }   // 0 == нет утечек
};


// =============================================================================
//  Тесты для Table (хеш-таблица ключ->значение).
//  Ключ — int, значение — char. Запуск: TableTests::runAll();
// =============================================================================
class TableTests
{
public:
    static void runAll();

    static const int N_BIG    = 1000000;   // вставка + обход итератором
    static const int N_REMOVE = 100000;     // удаление нечётных (у таблицы remove быстрый)

private:
    static void testEmpty();
    static void testInsertAndSize();
    static void testIterateAllPresent();
    static void testInsertIterateBig();
    static void testRemoveOddByKey();
    static void testFindByKeyHitMiss();
    static void testAt();
    static void testFindAfterRemoveViaFind();
    static void testDuplicateKey();
    static void testClearResetsState();
    static void testIteratorMemoryFreed();
    static void testNoLeaksAfterDestruct();
};
