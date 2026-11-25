#include "Table.h"
#include "Mem.h"

/////////////////////////////////////////about bucket's functions/////////////////////////////////////////////////////////////

void Table::Bucket::remove(Node* node)
{
    List::Node* temp = gethead();
    if (temp == node) sethead(node->next);//if the golova spiska is being removed
    else
    {
        while (temp->next)
        {
            if (temp->next == node)
            {
                temp->next = node->next;
                break;
            }
            temp = temp->next;
        }
    }
    _memory.freeMem(((TableNode*)node)->elem);
    _memory.freeMem(node->data);
    _memory.freeMem(node);                
}
void Table::Bucket::clear()
{
    Node* pop = gethead();
    while (pop)
    {
        Node* next = pop->next;
        _memory.freeMem(((TableNode*)pop)->elem);
        _memory.freeMem(pop->data);
        _memory.freeMem(pop);             
        pop = next;
    }
    sethead(nullptr);
}

/////////////////////////////////////////////about iterator/////////////////////////////////////////////////////////

void* Table::TableIterator::getElement(size_t& size)
{
    if (current)
    {
        size = ((TableNode*)current)->elemsize;
        return ((TableNode*)current)->elem;
    }
    size = 0;
    return nullptr;
}
bool Table::TableIterator::hasNext()
{
    if (current && current->next) return true;
	if (index == -1) return false;
    size_t j = table->numbuckets;
    for (size_t i = index + 1; i < j; i++)
    {
        if (table->buckets[i] && table->buckets[i]->gethead()) return true;
    }
    return false;
}
void  Table::TableIterator::goToNext()
{
    if (current && current->next)
    {
        current = current->next;
        return;
    }
    if (index == -1) return;
    size_t j = table->numbuckets;
    for (size_t i = index + 1; i < j; i++)
    {
        if (table->buckets[i] && table->buckets[i]->gethead())
        {
            index = i;
            current = table->buckets[i]->gethead();
            return;
        }
    }
    current = nullptr;
    index = -1;
}
bool  Table::TableIterator::equals(Iterator* right) 
{
    size_t one, two;
	void* elone = getElement(one);
	void* eltwo = right->getElement(two);
    if ((one == two) && (!(memcmp(elone, eltwo, one)))) return true;
    return false;
}

//////////////////////////////////////////////about table//////////////////////////////////////////////////////////

Table::Iterator* Table::newIterator()
{
    if (numelems)
    {
        TableIterator* iter = new TableIterator(this);
        if (iter)
        {
            if (this->buckets[0]) iter->current = this->buckets[0]->gethead(); //znachit element est tam
            else iter->goToNext();
            return iter;
        }
    }
    return nullptr;
}
void Table::rehash()
{
    size_t newnumbuckets; 
    if (numbuckets) newnumbuckets = numbuckets * 2;
    else newnumbuckets = 5381;
    Bucket** newbuckets = (Bucket**)_memory.allocMem(newnumbuckets * sizeof(Bucket*));
    if (!newbuckets) return;
    for (size_t i = 0; i < newnumbuckets; i++)
    {
        newbuckets[i] = nullptr;
    }
    for (size_t i = 0; i < numbuckets; i++)
    {
        Bucket* oldbucket = buckets[i];
        if (!oldbucket) continue;
        List::Node* node = oldbucket->gethead();
        while (node)
        {
            List::Node* oldnext = node->next; //sohranyu brata
            size_t j = tableHash(node->data, node->size, newnumbuckets); //kuda kladu v novom newbuckets
            if (!newbuckets[j])
            {
                void* tmp = _memory.allocMem(sizeof(Bucket));
                if (!tmp) return;
                newbuckets[j] = new (tmp) Bucket(_memory);
            }
            List::Node* nb_head = newbuckets[j]->gethead();
            node->next = nb_head;
            newbuckets[j]->sethead(node);
            node = oldnext; //perehod k bratu
        }
    }
    for (size_t i = 0; i < numbuckets; ++i)
    {
        if (buckets[i]) _memory.freeMem(buckets[i]);
    }
    _memory.freeMem(buckets);
    buckets = newbuckets;
    numbuckets = newnumbuckets;
}


Table::Iterator* Table::find(void* elem, size_t size)
{
    Iterator* iter = newIterator();
    if (iter)
    {
        size_t elemsize = 0;
        while (((TableIterator*)iter)->current)
        {
            void* temp = ((TableIterator*)iter)->getElement(elemsize);
            if ((size == elemsize) && (!memcmp(elem, temp, size))) return iter;
            ((TableIterator*)iter)->goToNext();
        }
        delete(iter); //
    }
    return nullptr;
}

void Table::remove(Iterator* iter)
{
    if (iter && ((TableIterator*)iter)->current)
    {
        size_t i = ((TableIterator*)iter)->index;
        List::Node* node = ((TableIterator*)iter)->current;
        iter->goToNext();
        buckets[i]->remove(node);
        numelems--;
    }
}

void Table::clear()
{
    for (size_t i = 0; i < numbuckets; i++)
    {
        if (buckets[i]) buckets[i]->clear();
    }
    numelems = 0;   
}

Table::~Table()
{
    if (buckets)
    {
        for (size_t i = 0; i < numbuckets; i++)
        {
            if (buckets[i])
            {
                buckets[i]->clear();         
                buckets[i]->~Bucket();       //new
                _memory.freeMem(buckets[i]); 
            }
        }
        _memory.freeMem(buckets);            
        buckets = nullptr;
    }
    numelems = 0;
    numbuckets = 0;
}

bool Table::empty() { return numelems == 0; }

int Table::insertByKey(void* key, size_t keySize, void* elem, size_t elemSize)
{
    if ( numbuckets==0 || ( numelems / numbuckets) > 0.8 ) rehash();
    Iterator* iter = findByKey(key, keySize);
    if (!iter)
    {
        size_t i = tableHash(key, keySize, numbuckets);
        if (!buckets[i])
        {
            void* tmp = _memory.allocMem(sizeof(Bucket));
            buckets[i] = new (tmp) Bucket(_memory);
        }
        if (buckets[i])
        {
            TableNode* node = (TableNode*)_memory.allocMem(sizeof(TableNode));
            if (node)
            {
                node->data = _memory.allocMem(keySize);
                if (node->data)
                {
                    memcpy(node->data, key, keySize);
                    node->size = keySize;
                    node->elem = _memory.allocMem(elemSize);
                    if (node->elem)
                    {
                        memcpy(node->elem, elem, elemSize);
                        node->elemsize = elemSize;
                        node->next = buckets[i]->gethead();  
                        buckets[i]->sethead(node);            
                        numelems++;
                        return 0;
                    }
                    _memory.freeMem(node->data);
                }
                _memory.freeMem(node);
            }
        }
    }
    delete iter; //
    return 1;
}

void Table::removeByKey(void* key, size_t keySize)
{
    Iterator* iter = findByKey(key, keySize);
    remove(iter);
    delete iter; //
}

Table::Iterator* Table::findByKey(void* key, size_t keySize)
{
    Iterator* iter = Table::newIterator();
    if (iter)
    {
        size_t i = tableHash(key, keySize, numbuckets);

        if (buckets[i] && buckets[i]->gethead())
        {
            ((TableIterator*)iter)->index = i;
            ((TableIterator*)iter)->current = buckets[i]->gethead();
            while (((TableIterator*)iter)->index == i) //чтобы не выйти из бакета
            {
                if (((((TableIterator*)iter)->current)->size == keySize) && (!memcmp(key, (((TableIterator*)iter)->current)->data, keySize)))
                {
                    return iter;
                }
                ((TableIterator*)iter)->goToNext();
            }
        }
        delete iter; //
    }
    return nullptr;
}

void* Table::at(void* key, size_t keySize, size_t& valueSize)
{
    Table::Iterator* iter = findByKey(key, keySize);
    if (iter)
    {
        void* temp = iter->getElement(valueSize);
        delete iter; //
        return temp;
    }
    return nullptr;
}

