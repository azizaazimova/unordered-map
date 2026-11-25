#include "List.h"
#include "MemoryManager.h"
#include "Mem.h"
#include "Container.h"

// Деструктор
List:: ~List()
{
    clear();
}

// Добавляет элемент в начало контейнера.
// В случае успешного выполнения функция возвращает значение 0, в случае ошибки 1.
int List::push_front(void* elem, size_t elemSize)
{
    Node* newNode = (Node*)_memory.allocMem(sizeof(Node));
    if (newNode == nullptr) return 1;
    newNode->data = _memory.allocMem(elemSize);
    if (newNode->data == nullptr)
    {
        _memory.freeMem(newNode);   // не теряем уже выделенный узел
        return 1;
    }
    memcpy(newNode->data, elem, elemSize);
    newNode->size = elemSize;
    newNode->next = golova;
    golova = newNode;
    numelem++;
    return 0;
}

// Удаляет элемент из начала контейнера.
void List::pop_front()
{
    if (!golova)  return;
    Node* temp = golova;
    golova = golova->next;
    _memory.freeMem(temp->data);
    _memory.freeMem(temp);
    numelem--;
}

// Возвращает указатель на элемент, находящийся в начале контейнера,
// и его размер через size.
void* List::front(size_t& size)
{
    if (golova == nullptr)
    {
        size = 0;
        return nullptr;
    }
    size = golova->size;
    return golova->data;
}

// Добавляет элемент в позицию, на которую указывает итератор iter
// (перед текущим элементом). Возвращает 0 при успехе, 1 при ошибке.
int List::insert(Iterator* iter, void* elem, size_t elemSize)
{
    if (iter == nullptr) return push_front(elem, elemSize);
    Node* newNode = (Node*)_memory.allocMem(sizeof(Node));
    if (newNode == nullptr) return 1;
    newNode->data = _memory.allocMem(elemSize);
    if (newNode->data == nullptr)
    {
        _memory.freeMem(newNode);
        return 1;
    }
    memcpy(newNode->data, elem, elemSize);
    newNode->size = elemSize;
    ListIterator* it = (ListIterator*)iter;
    newNode->next = it->current;
    if (it->prev == nullptr) golova = newNode;   // вставка перед головой
    else it->prev->next = newNode;
    it->prev = newNode;                          // новый узел стал предыдущим для current
    numelem++;
    return 0;
}

// Возвращает указатель на элемент, на который указывает итератор, и его размер.
// Если итератор указывает на пустую позицию, возвращает NULL.
void* List::ListIterator::getElement(size_t& size)
{
    if (current == nullptr)
    {
        size = 0;
        return nullptr;
    }
    size = current->size;
    return current->data;
}

// Возвращает true, если есть следующий элемент, иначе false.
bool List::ListIterator::hasNext()
{
    if (current && current->next) return true;
    return false;
}

// Переход к следующему элементу.
void List::ListIterator::goToNext()
{
    if (current)
    {
        prev = current;
        current = current->next;
    }
}

// Проверка на равенство итераторов
bool List::ListIterator::equals(Iterator* right)
{
    size_t one, two;
    void* elone = getElement(one);
    void* eltwo = right->getElement(two);
    if ((one == two) && (!(memcmp(elone, eltwo, one)))) return true;
    return false;
}

int List::size() // число элементов в контейнере.
{
    return numelem;
}

size_t List::max_bytes() // максимально допустимое количество памяти.
{
    return _memory.maxBytes();
}

// Поиск первого элемента, равного elem. Возвращает итератор на него
// (его нужно удалить через delete) или nullptr, если не найден.
Container::Iterator* List::find(void* elem, size_t size)
{
    Iterator* iter = newIterator();
    if (iter == nullptr) return nullptr;

    ListIterator* it = (ListIterator*)iter;
    while (it->current)
    {
        if ((it->current->size == size) && (!memcmp(it->current->data, elem, size)))
            return iter;             
        it->goToNext();
    }
    delete iter;   // 
    return nullptr;
}

// Создаёт итератор на первый элемент. Если контейнер пуст, возвращает nullptr.
// Удалять итератор обязан вызывающий через delete.
Container::Iterator* List::newIterator()
{
    if (!golova) return nullptr;
    return new ListIterator(golova);
}

// Удаляет элемент, на который указывает iter, и сдвигает итератор
// на следующий элемент. Освобождает память узла и его данных.
void List::remove(Iterator* iter)
{
    if (iter == nullptr) return;
    ListIterator* it = (ListIterator*)iter;
    Node* node = it->current;
    if (node == nullptr) return;
    Node* nxt = node->next;
    if (it->prev == nullptr) golova = nxt;       // удаляем голову
    else it->prev->next = nxt;                   // prev по-прежнему предшествует nxt
    _memory.freeMem(node->data);
    _memory.freeMem(node);
    it->current = nxt;                           // итератор смотрит на следующий
    numelem--;
}

// Удаляет все элементы контейнера, освобождая память через менеджер.
void List::clear()
{
    while (golova)
    {
        Node* temp = golova;
        golova = golova->next;
        _memory.freeMem(temp->data);
        _memory.freeMem(temp);
    }
    numelem = 0;
}

bool List::empty() // true если контейнер пуст
{
    return numelem == 0;
}
