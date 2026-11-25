#pragma once
#include "ListAbstract.h"
#include "MemoryManager.h"
#include "Mem.h"
#include "GroupList.h"

class List : public AbstractList
{
private:
	Node* golova;
	int numelem;
public:
	Node* gethead()
	{
		return this->golova;
	}
	void sethead(Node* current)
	{
		golova = current;
	}
	class ListIterator : public Iterator
	{
	public:
		Node* current;
		Node* prev;
		ListIterator(Node* node) : prev(nullptr), current(node) {}
		void* getElement(size_t& size);
		// Возврашает явно указатель на элемент, на который указывает итератор в данный момент.
		// Неявно возвращает размер данных.
		// Если итератор показывает за пределы контейнера (например, удален последний элемент), возвращает NULL.
		bool hasNext(); // Возвращает true, если есть следующий элемент, иначе false.
		void goToNext(); // Переход к следующему элементу.
		bool equals(Iterator* right); // проверка на равенство итераторов
	};
	List(MemoryManager& mem) : AbstractList(mem), golova(nullptr), numelem(0) {}
	~List()override;	// деструктор
	int push_front(void* elem, size_t elemSize);
	// Добавление элемента в начало контейнера.
// В случае успешного добавления функция возвращает значение 0, в случае неудачи 1.

	void pop_front();    // Удаление элемента из начала контейнера.

	void* front(size_t& size);
	// Функция явно возвращает указатель на элемент, находящийся в начале контейнера.
// и не явно возвращает размер данных

	int insert(Iterator* iter, void* elem, size_t elemSize);
	// Добавление элемента в позицию, на которую указывает итератор iter.
// В случае успешного добавления функция возвращает значение 0, в случае неудачи 1.


	int size(); // Функция возвращает значение, равное количеству элементов в контейнере.

	size_t max_bytes(); // Функция возвращает значение, равное максимальной вместимости контейнера в байтах. 

	Iterator* find(void* elem, size_t size);
	// Функция создает в динамической памяти итератор, указывающий на первый найденный
   // в контейнере элемент. Если элемент не найден, возвращается пустой указатель.
// Удаление этого итератора должно делаться пользователем с помощью оператора delete

	Iterator* newIterator();
	// Функция создает в динамической памяти итератор, указывающий на первый элемент
  // контейнера. Если контейнер пустой, возвращается нулевой указатель.
// Удаление этого итератора должно делаться пользователем с помощью оператора delete.

	void remove(Iterator* iter);
	// Удаление элемента из позиции, на которую указывает итератор iter.
// После удаления итератор указывает на следующий за удаленным элемент.

	void clear(); // Удаление всех элементов из контейнера.

	bool empty(); // Если контейнер пуст возвращает true, иначе false

};