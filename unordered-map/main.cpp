#include <iostream>
#include "Tests.h"
#include "Container.h"

// Запуск тестов контейнеров List и Table.
// Критерии: вставка (до 1 млн), обход итератором, удаление нечётных,
// поиск через find/findByKey, корректная работа с памятью через менеджер.
int main()
{
    try
    {
        TableTests::runAll();
        printf("ALL TESTS PASSED\n");
    }
    catch (Container::Error& e)
    {
        printf("TEST ERROR: %s\n", e.msg);
        return 1;
    }
    return 0;
}
