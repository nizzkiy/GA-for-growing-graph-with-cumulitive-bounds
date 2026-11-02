#include "Graph.h"
#include "GraphUtil.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace math;

// Глобальные переменные внутри DLL для хранения базового состояния
// Это позволяет не считывать файл при каждой оценке особи в ГА
static Graph* baseGraph = nullptr;
static std::vector<PotentialEdge> e_pool;
static unsigned originalVerts = 0;
static unsigned newVertsToAdd = 0;
static bool is_initialized = false;

// Оборачиваем в extern "C" ТОЛЬКО API функции для Python
extern "C" {

    // 1. Функция инициализации: вызывается из Питона один раз перед запуском ГА
#ifdef _WIN32
    __declspec(dllexport)
#endif
        void init_network_task(const char* filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "DLL ERROR: Cannot open " << filepath << std::endl;
            return;
        }

        // Очищаем предыдущий граф, если функцию вызвали повторно
        if (baseGraph) delete baseGraph;
        baseGraph = new Graph();

        // Используем ваш перегруженный оператор из Graph.cpp для базового графа
        file >> *baseGraph;
        originalVerts = baseGraph->getVertNumb();

        std::string marker;
        while (file >> marker) {
            if (marker == "new_vertices_count") {
                file >> newVertsToAdd;
            }
            else if (marker == "allowed_edges_pool") {
                unsigned poolSize;
                file >> poolSize;
                e_pool.resize(poolSize);
                for (unsigned i = 0; i < poolSize; ++i) {
                    file >> e_pool[i].u >> e_pool[i].v >> e_pool[i].p >> e_pool[i].cost;
                }
            }
        }
        file.close();
        is_initialized = true;
        std::cout << "C++ DLL Initialized successfully!" << std::endl;
        std::cout << "Base Verts: " << originalVerts << " | New Verts: " << newVertsToAdd
            << " | E_pool: " << e_pool.size() << std::endl;
    }

    // 2. Основная вычислительная функция: вызывается тысячи раз
#ifdef _WIN32
    __declspec(dllexport)
#endif
        double calculate_reliability(int* individual_array, int array_len) {
        if (!is_initialized || !baseGraph) {
            std::cerr << "DLL ERROR: Graph is not initialized!" << std::endl;
            return 0.0;
        }

        // 2.1. Создаем копию базового графа
        Graph* currentGraph = new Graph(*baseGraph);

        // 2.2. Наращиваем граф на основе хромосомы (individual_array)
        for (int i = 0; i < array_len; ++i) {
            if (individual_array[i] == 1 && i < (int)e_pool.size()) {
                // Пытаемся добавить выбранное ребро
                Graph* temp = currentGraph->addEdge(e_pool[i].u, e_pool[i].v, e_pool[i].p, e_pool[i].cost);
                if (temp) {
                    delete currentGraph; // Ваш addEdge возвращает новый граф, старый удаляем
                    currentGraph = temp;
                }
            }
        }

        // 2.3. Проверка: собрался ли полный связный граф?
        // Если вершин меньше, чем должно быть (т.е. какие-то новые вершины висят в воздухе)
        // или граф банально не связен - особь нежизнеспособна.
        if (currentGraph->getVertNumb() < (originalVerts + newVertsToAdd) || !currentGraph->isConnective()) {
            delete currentGraph;
            return 0.0;
        }

        // 2.4. Вызов вашего быстрого алгоритма оценки
        // Передаем porog = 0.0, чтобы факторизация дошла до точного значения
        CumBounds bounds = calcBounds(*currentGraph, 0.0);

        double result = bounds.RL;
        delete currentGraph; // Чистим память

        return result;
    }

    // 3. Очистка памяти по завершению работы скрипта
#ifdef _WIN32
    __declspec(dllexport)
#endif
        void free_network_task() {
        if (baseGraph) {
            delete baseGraph;
            baseGraph = nullptr;
        }
        e_pool.clear();
        is_initialized = false;
        std::cout << "C++ DLL Memory freed." << std::endl;
    }

} // конец extern "C"