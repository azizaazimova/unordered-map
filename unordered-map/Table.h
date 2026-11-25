#pragma once
#include "TableAbstract.h"
#include "List.h"

class Table : public AbstractTable
{
    struct TableNode : public List::Node
    {
        size_t elemsize;
        void* elem;
    };
    struct Bucket : public List
    {
        Bucket(MemoryManager& mem) : List(mem) {}
        void remove(Node*);
        void clear();
    };
    Bucket** buckets;
    size_t numbuckets; 
    void rehash();
public:
    class TableIterator : public Iterator
    {
    public:
        size_t index; 
        List::Node* current;
        Table* table;
        TableIterator(Table* mama) : index(0), current(nullptr), table(mama) {};
        // Возврашает явно указатель на элемент, на который указывает итератор в данный момент.
        // Неявно возвращает размер данных.
        // Если итератор показывает за пределы контейнера (например, удален последний элемент), возвращает NULL.
        void* getElement(size_t& size);
        // Возвращает true, если есть следующий элемент, иначе false.
        bool hasNext();
        // Переход к следующему элементу.
        void goToNext();
        // проверка на равенство итераторов
        bool equals(Iterator* right);
    };
    Table(MemoryManager& mem) : AbstractTable(mem), buckets(nullptr), numbuckets(0) { numelems = 0; }
    Table(MemoryManager& mem, size_t num) : AbstractTable(mem), buckets(nullptr), numbuckets(num)
    {
        buckets = (Bucket**)_memory.allocMem(numbuckets * sizeof(Bucket*));
        for (size_t i = 0; i < numbuckets; i++)
        {
            buckets[i] = nullptr;
        }
        numelems = 0;
    }
	~Table() override;
    // Функция возвращает значение, равное количеству элементов в контейнере.
    // Функция создает в динамической памяти итератор, указывающий на первый найденный
    // в контейнере элемент. Если элемент не найден, возвращается пустой указатель.
    // Удаление этого итератора должно делаться пользователем с помощью оператора delete.
    Iterator* find(void* elem, size_t size) override;
    // Функция создает в динамической памяти итератор, указывающий на первый элемент
    // контейнера. Если контейнер пустой, возвращается нулевой указатель.
    // Удаление этого итератора должно делаться пользователем с помощью оператора delete.
    Iterator* newIterator() override;
    // Удаление элемента из позиции, на которую указывает итератор iter.
    // После удаления итератор указывает на следующий за удаленным элемент.
    void remove(Iterator* iter) override;
    // Удаление всех элементов из контейнера.
    void clear() override;
    // Если контейнер пуст возвращает true, иначе false
    bool empty() override;
    // Добавление элемента в контейнер, с сответствующим ключом.
    // Если такой ключ уже есть, функция ничего не делает и возвращает 1.
    // В случае успешного добавления функция возвращает значение 0, в случае неудачи 1.
    int insertByKey(void* key, size_t keySize, void* elem, size_t elemSize) override;
    // Удаление элемента с сответствующим ключом из контейнера.
    void removeByKey(void* key, size_t keySize) override;
    // Функция возвращает указатель на итератор, указывающий на найденный в контейнере элемент с сответствующим ключом.
    // Если элемент не найден, возвращается нулевой указатель.
    Iterator* findByKey(void* key, size_t keySize) override;
    // доступ к элементу с ключом key
    void* at(void* key, size_t keySize, size_t& valueSize) override;
    //void print();
};