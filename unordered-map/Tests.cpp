#include "Tests.h"
#include "List.h"
#include "Table.h"
#include "Mem.h"
#include <cstdio>
#include <cstring>
#include <ctime>

// -----------------------------------------------------------------------------
//  CountingMem
// -----------------------------------------------------------------------------
CountingMem::CountingMem(size_t sz)
    : MemoryManager(sz), _allocCalls(0), _freeCalls(0), _liveBlocks(0) {}

void* CountingMem::allocMem(size_t sz)
{
    void* p = new char[sz];
    if (p) { _allocCalls++; _liveBlocks++; }
    return p;
}

void CountingMem::freeMem(void* ptr)
{
    if (ptr) { _freeCalls++; _liveBlocks--; }
    delete[] (char*)ptr;
}

// -----------------------------------------------------------------------------
//  Общий помощник: проверка условия с понятным сообщением.
// -----------------------------------------------------------------------------
static void check(bool cond, const char* what)
{
    if (!cond) { printf("  FAILED: %s\n", what); throw Container::Error(what); }
}

// читает int из текущей позиции итератора
static int valueOfInt(Container::Iterator* it)
{
    size_t sz = 0;
    void* d = it->getElement(sz);
    check(d != nullptr && sz == sizeof(int), "getElement: ожидался int");
    return *(int*)d;
}
// =============================================================================
//  TableTests
//  Ключ — int (i), значение — char ('A' + i%26).
// =============================================================================

static char tableValueFor(int i) { return (char)('A' + (i % 26)); }

static void tableFill(Table& t, int count)
{
    for (int i = 0; i < count; i++)
    {
        char v = tableValueFor(i);
        check(t.insertByKey(&i, sizeof(i), &v, sizeof(v)) == 0, "insertByKey вернул ошибку");
    }
}

void TableTests::runAll()
{
    printf("=== TableTests ===\n");
    testEmpty();
    testInsertAndSize();
    testIterateAllPresent();
    testInsertIterateBig();
    testRemoveOddByKey();
    //
    testFindByKeyHitMiss();
    testAt();
    testFindAfterRemoveViaFind();
    testDuplicateKey();
    testClearResetsState();
    testIteratorMemoryFreed();
    testNoLeaksAfterDestruct();
    printf("=== ALL TABLE TESTS PASSED ===\n\n");
}

void TableTests::testEmpty()
{
    Mem mem(1u << 30);
    Table t(mem, 16);
    check(t.empty(), "новая таблица не пустая");
    check(t.size() == 0, "size() != 0 у пустой");
    check(t.newIterator() == nullptr, "newIterator() на пустой не nullptr");
    int key = 5;
    check(t.findByKey(&key, sizeof(key)) == nullptr, "findByKey на пустой не nullptr");
    size_t vs = 0;
    check(t.at(&key, sizeof(key), vs) == nullptr, "at на пустой не nullptr");
    printf("  ok: testEmpty\n");
}

void TableTests::testInsertAndSize()
{
    Mem mem(1u << 30);
    Table t(mem, 101);
    const int n = 1000;
    tableFill(t, n);
    check(!t.empty(), "таблица пуста после вставки");
    check(t.size() == n, "size() != числу вставленных");
    printf("  ok: testInsertAndSize\n");
}

void TableTests::testIterateAllPresent()
{
    Mem mem(1u << 30);
    Table t(mem, 1009);
    const int n = 5000;
    tableFill(t, n);
    // итератор таблицы возвращает значения (char). Проверяем число элементов
    // обходом и что каждое значение соответствует какому-то корректному ключу.
    long long counted = 0;
    Container::Iterator* it = t.newIterator();
    check(it != nullptr, "newIterator на непустой == nullptr");
    while (it)
    {
        size_t sz = 0;
        void* d = it->getElement(sz);
        check(d != nullptr && sz == sizeof(char), "getElement таблицы: ожидался char");
        counted++;
        if (it->hasNext()) it->goToNext();
        else { delete it; it = nullptr; }
    }
    check(counted == n, "обход дал не n элементов");
    // полноту проверяем через findByKey по всем ключам
    for (int i = 0; i < n; i++)
    {
        Container::Iterator* f = t.findByKey(&i, sizeof(i));
        check(f != nullptr, "ключ потерялся");
        delete f;
    }
    printf("  ok: testIterateAllPresent\n");
}

void TableTests::testInsertIterateBig()
{
    Mem mem((size_t)1 << 31);
    Table t(mem, 2000003);   // с запасом, чтобы не упираться в rehash
    clock_t t0 = clock();
    tableFill(t, N_BIG);
    check(t.size() == N_BIG, "size() != N_BIG");
    long long counted = 0;
    Container::Iterator* it = t.newIterator();
    while (it)
    {
        size_t sz = 0;
        check(it->getElement(sz) != nullptr, "пустой элемент при обходе");
        counted++;
        if (it->hasNext()) it->goToNext();
        else { delete it; it = nullptr; }
    }
    check(counted == N_BIG, "обход 1 млн дал не N_BIG");
    double sec = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("  ok: testInsertIterateBig (1e6, %.2fs)\n", sec);
}

void TableTests::testRemoveOddByKey()
{
    Mem mem(1u << 30);
    Table t(mem, 200003);
    const int n = N_REMOVE;
    tableFill(t, n);
    for (int i = 1; i < n; i += 2) t.removeByKey(&i, sizeof(i));
    check(t.size() == n - n / 2, "после удаления нечётных неверный size()");
    for (int i = 0; i < n; i++)
    {
        Container::Iterator* f = t.findByKey(&i, sizeof(i));
        if (i % 2 == 0) { check(f != nullptr, "чётный ключ пропал"); delete f; }
        else            check(f == nullptr, "нечётный ключ не удалён");
    }
    printf("  ok: testRemoveOddByKey (%d)\n", n);
}

void TableTests::testFindByKeyHitMiss()
{
    Mem mem(1u << 30);
    Table t(mem, 1009);
    tableFill(t, 1000);
    int key = 333;
    Container::Iterator* it = t.findByKey(&key, sizeof(key));
    check(it != nullptr, "findByKey не нашёл существующий ключ");
    size_t sz = 0;
    void* v = it->getElement(sz);
    check(v && *(char*)v == tableValueFor(key), "findByKey вернул не то значение");
    delete it;

    int absent = 999999;
    check(t.findByKey(&absent, sizeof(absent)) == nullptr, "findByKey нашёл отсутствующий ключ");
    printf("  ok: testFindByKeyHitMiss\n");
}

void TableTests::testAt()
{
    Mem mem(1u << 30);
    Table t(mem, 1009);
    tableFill(t, 1000);

    for (int i = 0; i < 1000; i += 137)
    {
        size_t vs = 0;
        void* v = t.at(&i, sizeof(i), vs);
        check(v != nullptr && vs == sizeof(char), "at вернул мусор");
        check(*(char*)v == tableValueFor(i), "at вернул не то значение");
    }
    printf("  ok: testAt\n");
}

void TableTests::testFindAfterRemoveViaFind()
{
    Mem mem(1u << 30);
    Table t(mem, 20011);
    const int n = 10000;
    tableFill(t, n);
    for (int i = 1; i < n; i += 2) t.removeByKey(&i, sizeof(i));
    // второй цикл — через findByKey
    for (int i = 0; i < n; i++)
    {
        Container::Iterator* f = t.findByKey(&i, sizeof(i));
        if (i % 2 == 0)
        {
            check(f != nullptr, "чётный ключ потерян");
            size_t sz = 0;
            check(*(char*)f->getElement(sz) == tableValueFor(i), "значение чётного изменилось");
            delete f;
        }
        else check(f == nullptr, "нечётный ключ не удалён");
    }
    printf("  ok: testFindAfterRemoveViaFind\n");
}

void TableTests::testDuplicateKey()
{
    Mem mem(1u << 30);
    Table t(mem, 101);
    int key = 7;
    char a = 'X', b = 'Y';
    check(t.insertByKey(&key, sizeof(key), &a, sizeof(a)) == 0, "первая вставка не удалась");
    // повторная вставка того же ключа: по спецификации ничего не меняем, возвращаем 1
    check(t.insertByKey(&key, sizeof(key), &b, sizeof(b)) == 1, "дубликат ключа должен вернуть 1");
    check(t.size() == 1, "дубликат увеличил размер");

    size_t vs = 0;
    void* v = t.at(&key, sizeof(key), vs);
    check(v && *(char*)v == 'X', "значение по ключу изменилось при дубликате");
    printf("  ok: testDuplicateKey\n");
}

void TableTests::testClearResetsState()
{
    Mem mem(1u << 30);
    Table t(mem, 257);
    tableFill(t, 1000);
    check(t.size() == 1000, "size до clear неверный");

    t.clear();
    check(t.empty(), "после clear таблица не пуста");
    check(t.size() == 0, "после clear size() != 0");
    check(t.newIterator() == nullptr, "после clear newIterator не nullptr");
    int key = 1;
    check(t.findByKey(&key, sizeof(key)) == nullptr, "после clear ключ ещё находится");

    // таблица остаётся пригодной к работе после clear
    tableFill(t, 10);
    check(t.size() == 10, "таблица не работает после clear");
    printf("  ok: testClearResetsState\n");
}

void TableTests::testNoLeaksAfterDestruct()
{
    CountingMem mem(1u << 30);
    {
        Table t(mem, 101);
        tableFill(t, 5000);          // вызовет и rehash по пути
        check(mem.liveBlocks() > 0, "менеджер не использовался");
        for (int i = 1; i < 5000; i += 2) t.removeByKey(&i, sizeof(i));
    }   // здесь ~Table()
    check(mem.liveBlocks() == 0, "утечка: liveBlocks != 0 после ~Table");
    check(mem.allocCalls() == mem.freeCalls(), "alloc/free таблицы несбалансированы");
    printf("  ok: testNoLeaksAfterDestruct (alloc=%lld free=%lld)\n",
           mem.allocCalls(), mem.freeCalls());
}

void TableTests::testIteratorMemoryFreed()
{
    CountingMem mem(1u << 30);
    Table t(mem, 257);
    tableFill(t, 200);

    // итераторы таблицы создаются через operator new/delete и не трогают
    // менеджер, поэтому число живых блоков менеджера не должно меняться.
    long long before = mem.liveBlocks();
    for (int rep = 0; rep < 10; rep++)
    {
        Container::Iterator* it = t.newIterator();
        check(it != nullptr, "newIterator вернул nullptr");
        while (it->hasNext()) it->goToNext();
        delete it;
    }
    check(mem.liveBlocks() == before, "создание/удаление итераторов изменило память менеджера");
    printf("  ok: testIteratorMemoryFreed (live blocks stable = %lld)\n", before);
}
