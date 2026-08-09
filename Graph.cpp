#include <mimalloc-override.h>
#include <mimalloc.h>
#include "Graph.h"
#include "random.h"
#include <limits>
#include "list.h"
#include <cstdlib>
#include <queue>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <random>
using namespace containers;


void* operator new(std::size_t sz) {
    return mi_malloc_aligned(sz, alignof(std::max_align_t));
}
void* operator new[](std::size_t sz) {
    return mi_malloc_aligned(sz, alignof(std::max_align_t));
}
void operator delete(void* ptr) noexcept {
    mi_free(ptr);
}
void operator delete[](void* ptr) noexcept {
    mi_free(ptr);
}
void operator delete(void* ptr, std::size_t) noexcept {
    mi_free(ptr);
}
void operator delete[](void* ptr, std::size_t) noexcept {
    mi_free(ptr);
}

namespace math {

void Graph::memclear() {
    delete [] KAO;
    delete [] FO;
    delete [] FORel;
    delete[] FOCost;

    KAO = nullptr;
    FO = nullptr;
    FORel = nullptr;
    FOCost = nullptr;
    vertNumb = 0;
    edgNumb = 0;
}

void Graph::printAdjMatrix() const {
        // Iterate over all pairs of vertices from 1 to vertNumb
        for (u_int i = 1; i <= vertNumb; ++i) {
            for (u_int j = 1; j <= vertNumb; ++j) {
                // isEdge(i, j) returns true if edge (i, j) exists in the graph
                if (isEdge(i, j)) {
                    std::cout << "1 ";
                } else {
                    std::cout << "0 ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }



void Graph::memcopy(const Graph &graph) {
    vertNumb = graph.vertNumb;
    edgNumb = graph.edgNumb;

    KAO = new u_int[vertNumb + 1];
    FO = new u_int[2 * edgNumb];
    FORel = new double[2 * edgNumb];
    FOCost = new double[2 * edgNumb];

    for (u_int i = 0; i <= vertNumb; i++) {
        KAO[i] = graph.KAO[i];
    }
    for (u_int i = 0; i < (2 * edgNumb); i++) {
        FO[i] = graph.FO[i];
        FORel[i] = graph.FORel[i];
        FOCost[i] = graph.FOCost[i];
    }
}

#if __cplusplus >= 201103L
void Graph::memmove(Graph &&other) {
    KAO = other.KAO;
    FO = other.FO;
    FORel = other.FORel;
    FOCost = other.FOCost;
    vertNumb = other.vertNumb;
    edgNumb = other.edgNumb;

    other.KAO = nullptr;
    other.FO = nullptr;
    other.FORel = nullptr;
    other.FOCost = nullptr;
    other.vertNumb = 0;
    other.edgNumb = 0;
}
#endif

void Graph::memallocNewGraph(u_int _vertNumb, u_int _edgNumb, u_int *&newKAO, u_int *&newFO, double *&newFORel, double *&newFOCost) const {
    newKAO = new u_int[_vertNumb + 1];
    newFO = new u_int[2 * _edgNumb];
    newFORel = new double[2 * _edgNumb];
    newFOCost = new double[2 * _edgNumb];
}



void Graph::memupdate(u_int vertLim, u_int edgLim, u_int *&_KAO, u_int *&_FO, double *&_FORel,double *&_FOCost) {
    // Освобождаем текущую память объекта
    delete[] KAO;
    delete[] FO;
    delete[] FORel;
    delete[] FOCost;

    // Подменяем указатели на новые
    KAO = _KAO;
    FO = _FO;
    FORel = _FORel;
    FOCost = _FOCost;

    // Обновляем размеры
    vertNumb = vertLim;
    edgNumb = edgLim;

    // Зануляем входящие указатели, чтобы внешняя функция случайно не удалила их
    _KAO = nullptr; _FO = nullptr; _FORel = nullptr; _FOCost = nullptr;
}

void Graph::memrebase(u_int vertLim, u_int edgLim, u_int *&_KAO, u_int *&_FO, double *&_FORel,double *&_FOCost) {
    memclear();
    vertNumb = vertLim;
    edgNumb = edgLim;
    KAO = _KAO;
    FO = _FO;
    FORel = _FORel;
    FOCost = _FOCost;

}

bool Graph::meminited() const {
    return (vertNumb != 0) && (edgNumb != 0);
}

bool Graph::isEdge(u_int u, u_int v) const {
    // Если вершина выходит за пределы текущего массива KAO
    if (u == 0 || v == 0 || u > vertNumb || v > vertNumb || u == v) {
        return false;
    }

    // Теперь это безопасно
    for (u_int i = KAO[u - 1]; i < KAO[u]; i++) {
        if (FO[i] == v) {
            return true;
        }
    }
    return false;
}

bool Graph::isEdgeWithPos(u_int u, u_int v, u_int &pos) const {
    if (u == v) {
        return false;
    }

    for (u_int i = KAO[u - 1]; i < KAO[u]; i++) {
        if (FO[i] == v) {
            pos = i;
            return true;
        }
    }
    return false;
}

#if __cplusplus >= 201103L
Graph::Graph(): KAO(nullptr), FO(nullptr), FORel(nullptr),FOCost(nullptr), vertNumb(0), edgNumb(0) {}

Graph::Graph(Graph &&other) {
    memmove(std::forward<Graph>(other));
}
#else
    Graph::Graph(): KAO(NULL), FO(NULL), FORel(NULL),FOCost(NULL), vertNumb(0), edgNumb(0){}
#endif

Graph::Graph(const Graph &graph) {
   
    memcopy(graph);
}

Graph::Graph(u_int _vertNumb, u_int _edgNumb, u_int *&_KAO, u_int *&_FO, double *&_FORel, double *&_FOCost): vertNumb(_vertNumb), edgNumb(_edgNumb) {
    KAO = _KAO;
    FO = _FO;
    FORel = _FORel;
    FOCost = _FOCost;
}

Graph::~Graph() {
    memclear();
} 

std::ifstream &operator >>(std::ifstream &input, Graph &graph) {
    input >> graph.vertNumb;
    input >> graph.edgNumb;

    // Проверка, что заголовок считался
    if (input.fail()) {
        std::cerr << "Error: Failed to read graph header (N M)!" << std::endl;
        return input;
    }

    graph.KAO = new u_int[graph.vertNumb + 1];
    graph.FO = new u_int[2 * graph.edgNumb];
    graph.FORel = new double[2 * graph.edgNumb];
    graph.FOCost = new double[2 * graph.edgNumb]; // 1. Выделяем память

    for (u_int i = 0; i <= graph.vertNumb; i++) {
        input >> graph.KAO[i];
    }

    for (u_int i = 0; i < (graph.edgNumb * 2); i++) {
        input >> graph.FO[i];
        //graph.FORel[i] = 0.75;
        //graph.FOCost[i] = 1.0; 
        input >> graph.FORel[i];
        input >> graph.FOCost[i];

        // ВАЖНО: Если чтение не удалось (конец файла или мусор вместо числа)
        if (input.fail()) {
            std::cerr << "CRITICAL ERROR reading edge #" << i
                << "! Expected " << (graph.edgNumb * 2) << " lines." << std::endl;
            // Можно даже бросить исключение или просто выйти, но мы увидим лог
            break;
        }

        
    }

    return input;
}

Graph &Graph::operator= (const Graph &graph) {
    if (this == &graph) {
        return *this;
    }

    if (meminited()) {
        memclear();
    }
    memcopy(graph);

    return *this;
}

Graph &Graph::operator= (const Graph *graph) {
    if (this == graph) {
        return *this;
    }

    if (meminited()) {
        memclear();
    }
    memcopy(*graph);

    return *this;
}



#if __cplusplus >= 201103L
Graph &Graph::operator= (Graph &&other) {
    if (meminited()) {
        memclear();
    }

    memmove(std::forward<Graph>(other));

    return *this;
}
#endif
// -----    МЕТОДЫ МОДИФИКАЦИИ ГРАФА ------ //



bool Graph::isStructureValid(const std::string& context) const {
    std::cerr << "=== Validating Graph " << context << " ===" << std::endl;
    std::cerr << "vertNumb=" << vertNumb << ", edgNumb=" << edgNumb << std::endl;

    // Проверка 1: KAO монотонно возрастает
    for (u_int i = 0; i < vertNumb; ++i) {
        if (KAO[i] > KAO[i + 1]) {
            std::cerr << "ERROR: KAO not monotonic! KAO[" << i << "]=" << KAO[i]
                << " > KAO[" << i + 1 << "]=" << KAO[i + 1] << std::endl;
            return false;
        }
    }

    // Проверка 2: KAO в пределах границ
    for (u_int i = 0; i <= vertNumb; ++i) {
        if (KAO[i] > 2 * edgNumb) {
            std::cerr << "ERROR: KAO[" << i << "]=" << KAO[i]
                << " exceeds 2*edgNumb=" << (2 * edgNumb) << std::endl;
            return false;
        }
    }

    // Проверка 3: FO содержит валидные индексы
    for (u_int i = 0; i < 2 * edgNumb; ++i) {
        if (FO[i] == 0 || FO[i] > vertNumb) {
            std::cerr << "ERROR: FO[" << i << "]=" << FO[i]
                << " (valid: 1.." << vertNumb << ")" << std::endl;
            return false;
        }
    }

    // Проверка 4: Каждое ребро упоминается дважды (в обе стороны)
    for (u_int u = 1; u <= vertNumb; ++u) {
        for (u_int j = KAO[u - 1]; j < KAO[u]; ++j) {
            u_int v = FO[j];
            bool found = false;
            for (u_int k = KAO[v - 1]; k < KAO[v]; ++k) {
                if (FO[k] == u) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cerr << "ERROR: Edge (" << u << ", " << v
                    << ") is not symmetric!" << std::endl;
                return false;
            }
        }
    }

    std::cerr << "=== Graph structure VALID ===" << std::endl;
    return true;
}

Graph *Graph::mergeEdge(u_int u, u_int v) const {
    if ((u == 0) || (u > vertNumb) || (v == 0) || (v > vertNumb) || (!isEdge(u, v))) {
        return nullptr;
    }
    u_int newVertNumb = vertNumb - 1;
    u_int newEdgNumb = edgNumb - 1;
    u_int *newKAO;
    u_int *newFO;
    double *newFORel;
    double* newFOCost;
    u_int newVert;      //number of vertex to which contraction is performed (the smallest number from the pair (u, v))
    u_int delVert;      //number of vertex that will be removed from the graph

    if (u < v) {
        newVert = u;
        delVert = v;
    }
    else if (v < u) {
        newVert = v;
        delVert = u;
    }
    else {
        return nullptr;         //raise exception here
    }

    for (u_int i = KAO[newVert - 1]; i < KAO[newVert]; i++) {
        if (FO[i] == v) {
            continue;
        }
        if (isEdge(FO[i], delVert)) {
            newEdgNumb--;
        }
    }

    memallocNewGraph(newVertNumb, newEdgNumb, newKAO, newFO, newFORel, newFOCost);

    newKAO[0] = 0;
    u_int iC = 1;
    u_int jC = 0;
    u_int pos = 0;
    for (u_int i = 1; i <= vertNumb; i++) {
        if (i == delVert) {
            continue;
        }
        else if (i == newVert) {
            newKAO[iC] = newKAO[iC - 1] + (KAO[i] - KAO[i - 1]) + (KAO[delVert] - KAO[delVert - 1]) - 2;
            for (u_int j = KAO[delVert - 1]; j < KAO[delVert]; j++) {
                if (FO[j] == newVert) {
                    continue;
                }
                if (!isEdge(FO[j], newVert)) {
                    newFO[jC] = FO[j];
                    newFORel[jC] = FORel[j];
                    newFOCost[jC] = FOCost[j];
                    if (FO[j] > delVert) {
                        newFO[jC]--;
                    }
                    jC++;
                }
                else {
                    continue;
                }
            }
            for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
                if (FO[j] == delVert) {
                    continue;
                }
                if (isEdgeWithPos(FO[j], delVert, pos)) {
                    newKAO[iC]--;
                    newFORel[jC] = FORel[j] + FORel[pos] - FORel[j]*FORel[pos];
                    newFOCost[jC] = (FOCost[j] < FOCost[pos]) ? FOCost[j] : FOCost[pos];
                }
                else {
                    newFORel[jC] = FORel[j];
                    newFOCost[jC] = FOCost[j];
                }
                newFO[jC] = FO[j];
                if (FO[j] > delVert) {
                    newFO[jC]--;
                }
                jC++; 
            }
        }
        else {
            u_int posNew = 0;
            u_int posDel = 0;
            newKAO[iC] = newKAO[iC - 1] + (KAO[i] - KAO[i - 1]);
            bool newVertConnected = isEdgeWithPos(i, newVert, posNew);
            bool delVertConnected = isEdgeWithPos(i, delVert, posDel);

            if (!delVertConnected) {
                for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
                    newFO[jC] = FO[j];
                    newFORel[jC] = FORel[j];
                    newFOCost[jC] = FOCost[j];
                    if (FO[j] > delVert) {
                        newFO[jC]--;
                    }
                    jC++;
                }
            }
            else if (!newVertConnected && delVertConnected) {
                for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
                    if (FO[j] == delVert) {
                        newFO[jC] = newVert;
                    }
                    else {
                        newFO[jC] = FO[j];
                        if (FO[j] > delVert) {
                            newFO[jC]--;
                        }
                    }
                    newFORel[jC] = FORel[j];
                    newFOCost[jC] = FOCost[j];
                    jC++;
                }
            }
            else if (newVertConnected && delVertConnected) {
                newKAO[iC]--;
                for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
                    if (FO[j] == delVert) {
                        continue;
                    }
                    newFO[jC] = FO[j];
                    if (FO[j] == newVert) {
                        newFORel[jC] = FORel[j] + FORel[posDel] - FORel[j]*FORel[posDel];
                        newFOCost[jC] = (FOCost[j] < FOCost[posDel]) ? FOCost[j] : FOCost[posDel];
                    }
                    else {
                        newFORel[jC] = FORel[j];
                        newFOCost[jC] = FOCost[j];
                    }
                    if (FO[j] > delVert) {
                            newFO[jC]--;
                        }
                    jC++;
                }
            }
        }

        iC++;
    }
    
    Graph *result = new Graph(newVertNumb, newEdgNumb, newKAO, newFO, newFORel,newFOCost);
    return result;
}

    Graph *Graph::cutEdge(u_int u, u_int v) const {
    if (u == 0 || v == 0 || u > vertNumb || v > vertNumb || !isEdge(u, v)) {
        return nullptr;
    }

    u_int newVertNumb = vertNumb;
    u_int newEdgNumb = edgNumb - 1;

    u_int *newKAO;
    u_int *newFO;
    double *newFORel;
    double* newFOCost;

    memallocNewGraph(newVertNumb, newEdgNumb, newKAO, newFO, newFORel,newFOCost);

    u_int jC = 0;
    for (u_int i = 0; i < newVertNumb; i++) {
        newKAO[i] = jC;
        for (u_int j = KAO[i]; j < KAO[i + 1]; j++) {
            if ((i + 1 == u && FO[j] == v) || (i + 1 == v && FO[j] == u)) {
                continue;
            }
            newFO[jC] = FO[j];
            newFORel[jC] = FORel[j];
            newFOCost[jC] = FOCost[j];
            jC++;
        }
    }
    newKAO[newVertNumb] = jC;

    return new Graph(newVertNumb, newEdgNumb, newKAO, newFO, newFORel, newFOCost);
}

void Graph::shootsDeletion(double& prob) {
        // Используем thread_local для предотвращения выделений памяти на каждой рекурсии
        thread_local std::vector<int> deg;
        deg.assign(vertNumb + 1, 0); // assign перезапишет данные без выделения новой памяти

        for (u_int i = 1; i <= vertNumb; i++) {
            deg[i] = KAO[i] - KAO[i - 1];
        }

        // Заменяем медленный std::queue на быстрый вектор
        thread_local std::vector<u_int> leaves;
        leaves.clear();

        for (u_int i = 1; i <= vertNumb; i++) {
            if (deg[i] == 1) leaves.push_back(i);
        }

        thread_local std::vector<bool> removed;
        removed.assign(vertNumb + 1, false);

        size_t head = 0; // индекс вместо leaves.pop()

        while (head < leaves.size()) {
            u_int leaf = leaves[head++]; // Достаем верхний элемент и двигаем "начало"

            if (removed[leaf] || deg[leaf] != 1) continue;

            u_int neighbor = 0;
            u_int edgePos = KAO[leaf - 1];
            for (u_int pos = KAO[leaf - 1]; pos < KAO[leaf]; pos++) {
                if (!removed[FO[pos]]) {
                    neighbor = FO[pos];
                    edgePos = pos;
                    break;
                }
            }

            prob *= FORel[edgePos];
            removed[leaf] = true;
            deg[leaf] = 0;
            deg[neighbor]--;

            if (!removed[neighbor] && deg[neighbor] == 1) leaves.push_back(neighbor);
        }

        u_int removedCount = 0;
        for (u_int i = 1; i <= vertNumb; i++) {
            if (removed[i]) removedCount++;
        }

        if (removedCount == 0) return;

        u_int newVN = vertNumb - removedCount;
        u_int newE = 0;
        for (u_int i = 1; i <= vertNumb; i++) {
            if (!removed[i]) newE += deg[i];
        }
        newE /= 2;

        u_int* newKAO, * newFO; double* newFORel, * newFOCost;
        memallocNewGraph(newVN, newE, newKAO, newFO, newFORel, newFOCost);

        // Избегаем создания std::vector внутри локальной области
        thread_local std::vector<u_int> newIndex;
        newIndex.assign(vertNumb + 1, 0);

        u_int curId = 1;
        for (u_int i = 1; i <= vertNumb; i++) {
            if (!removed[i]) newIndex[i] = curId++;
        }

        newKAO[0] = 0;
        u_int foPos = 0;
        for (u_int i = 1; i <= vertNumb; i++) {
            if (removed[i]) continue;
            u_int iNew = newIndex[i];
            newKAO[iNew] = newKAO[iNew - 1] + deg[i];

            for (u_int pos = KAO[i - 1]; pos < KAO[i]; pos++) {
                u_int nb = FO[pos];
                if (!removed[nb]) {
                    newFO[foPos] = newIndex[nb];
                    newFORel[foPos] = FORel[pos];
                    newFOCost[foPos] = FOCost[pos];
                    foPos++;
                }
            }
        }

        memupdate(newVN, newE, newKAO, newFO, newFORel, newFOCost);
    }

bool Graph::parallelEdgesTransformation() {
    bool foundParallel = false;
    std::vector<u_int> seen(vertNumb + 1, 0);
    u_int currentMarker = 1;

    // 1. Быстрая проверка: есть ли вообще дубликаты? (чтобы зря не перевыделять память)
    for (u_int i = 1; i <= vertNumb; i++) {
        for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
            u_int v = FO[j];
            if (seen[v] == currentMarker) {
                foundParallel = true;
                break;
            }
            seen[v] = currentMarker;
        }
        if (foundParallel) break;
        currentMarker++;
    }

    if (!foundParallel) return false; // Граф без параллельных ребер, выходим

    // 2. Слияние параллельных ребер
    std::vector<u_int> tempKAO(vertNumb + 1, 0);
    std::vector<u_int> tempFO;
    std::vector<double> tempFORel;
    std::vector<double> tempFOCost;

    // Массив для хранения индекса ребра в tempFO. 
    u_int NONE = (u_int)-1;
    std::vector<u_int> edgePos(vertNumb + 1, NONE);

    for (u_int i = 1; i <= vertNumb; i++) {
        tempKAO[i] = tempKAO[i - 1];

        for (u_int j = KAO[i - 1]; j < KAO[i]; j++) {
            u_int v = FO[j];
            double p = FORel[j];
            double c = FOCost[j];

            if (edgePos[v] != NONE) {
                // Дубликат найден! Производим параллельное слияние
                u_int pos = edgePos[v];

                // Эквивалентная надежность: P = p1 + p2 - p1 * p2
                tempFORel[pos] = tempFORel[pos] + p - tempFORel[pos] * p;

                // Эквивалентная стоимость: берем минимальную (как в вашем PST)
                if (c < tempFOCost[pos]) {
                    tempFOCost[pos] = c;
                }
            }
            else {
                // Новое уникальное ребро для текущей вершины i
                edgePos[v] = tempFO.size();
                tempFO.push_back(v);
                tempFORel.push_back(p);
                tempFOCost.push_back(c);
                tempKAO[i]++;
            }
        }

        // Обязательно очищаем массив позиций только для тех соседей, которых затронули
        for (u_int j = tempKAO[i - 1]; j < tempKAO[i]; j++) {
            edgePos[tempFO[j]] = NONE;
        }
    }

    // 3. Выделение памяти под новый граф
    u_int nextVertNumb = vertNumb;
    // Так как граф неориентированный, каждое ребро мы записали 2 раза (для u и для v)
    u_int nextEdgNumb = tempFO.size() / 2;

    u_int* nKAO;
    u_int* nFO;
    double* nFORel;
    double* nFOCost;
    memallocNewGraph(nextVertNumb, nextEdgNumb, nKAO, nFO, nFORel, nFOCost);

    // 4. Перенос данных в C-массивы
    for (u_int i = 0; i <= vertNumb; i++) {
        nKAO[i] = tempKAO[i];
    }
    for (size_t j = 0; j < tempFO.size(); j++) {
        nFO[j] = tempFO[j];
        nFORel[j] = tempFORel[j];
        nFOCost[j] = tempFOCost[j];
    }

    // 5. Безопасное обновление текущего объекта
    // memupdate автоматически сделает delete[] старым массивам
    memupdate(nextVertNumb, nextEdgNumb, nKAO, nFO, nFORel, nFOCost);

    return true; // Граф был изменен
}


void Graph::parallelSeriesTransformation(double& prob) {
    prob = 1.0;

    // Используем thread_local векторы как рабочие буферы.
    // Память под них выделится один раз для каждого потока ОС и будет переиспользоваться,
    // что полностью исключает блокировки аллокатора (Heap Lock Contention).
    thread_local std::vector<u_int> curKAO, nextKAO;
    thread_local std::vector<u_int> curFO, nextFO;
    thread_local std::vector<double> curFORel, nextFORel;
    thread_local std::vector<double> curFOCost, nextFOCost;

    u_int vNumb = vertNumb;
    u_int eNumb = edgNumb;

    // Загружаем начальное состояние в "текущие" буферы
    curKAO.assign(KAO, KAO + vNumb + 1);
    curFO.assign(FO, FO + 2 * eNumb);
    curFORel.assign(FORel, FORel + 2 * eNumb);
    curFOCost.assign(FOCost, FOCost + 2 * eNumb);

    bool anyChanges = false;

    while (true) {
        u_int u = 0, v = 0, currentDelVert = 0;
        double p1, p2, c1, c2, newProb, newCost;
        bool found2Deg = false;
        bool duplicate = false;

        // 1. Поиск вершины степени 2
        for (u_int i = 1; i <= vNumb; i++) {
            if ((curKAO[i] - curKAO[i - 1]) == 2) {
                currentDelVert = i;
                u = curFO[curKAO[i - 1]];
                v = curFO[curKAO[i] - 1];
                p1 = curFORel[curKAO[i - 1]];
                p2 = curFORel[curKAO[i] - 1];
                c1 = curFOCost[curKAO[i - 1]];
                c2 = curFOCost[curKAO[i] - 1];

                // Расчет эквивалентных характеристик
                prob *= (p1 + p2 - p1 * p2);
                newProb = (p1 * p2) / (p1 + p2 - p1 * p2);
                newCost = c1 + c2; // Последовательное: сумма цен

                // Проверка на наличие параллельного ребра
                for (u_int j = curKAO[u - 1]; j < curKAO[u]; j++) {
                    if (curFO[j] == v) {
                        duplicate = true;
                        newProb = 1.0 - (1.0 - curFORel[j]) * (1.0 - newProb);
                        if (curFOCost[j] < newCost) newCost = curFOCost[j]; // Параллельное
                        break;
                    }
                }
                found2Deg = true;
                break;
            }
        }

        if (!found2Deg) break; // Больше нечего упрощать — выходим
        anyChanges = true;

        // 2. Подготовка новых размеров массивов
        u_int nextVertNumb = vNumb - 1;
        u_int nextEdgNumb = duplicate ? (eNumb - 2) : (eNumb - 1);

        // resize не будет делать медленный realloc, так как размер только уменьшается
        nextKAO.resize(nextVertNumb + 1);
        nextFO.resize(2 * nextEdgNumb);
        nextFORel.resize(2 * nextEdgNumb);
        nextFOCost.resize(2 * nextEdgNumb);

        // 3. Пересборка графа
        nextKAO[0] = 0;
        u_int iC = 1; // Индекс новой вершины
        u_int jC = 0; // Индекс нового ребра

        for (u_int i = 1; i <= vNumb; i++) {
            if (i == currentDelVert) continue;

            nextKAO[iC] = nextKAO[iC - 1];

            for (u_int j = curKAO[i - 1]; j < curKAO[i]; j++) {
                u_int target = curFO[j];
                double targetRel = curFORel[j];
                double targetCost = curFOCost[j];

                // Если ребро ведет в удаляемую вершину
                if (target == currentDelVert) {
                    if (duplicate) continue; // Это ребро поглощено параллельным
                    target = (i == u) ? v : u;
                    targetRel = newProb;
                    targetCost = newCost;
                }
                // Если это существующее параллельное ребро между u и v
                else if (((i == u) && (target == v)) || ((i == v) && (target == u))) {
                    targetRel = newProb;
                    targetCost = newCost;
                }

                // Корректировка индекса цели (все вершины выше удаленной сдвигаются вниз)
                if (target > currentDelVert) target--;

                nextFO[jC] = target;
                nextFORel[jC] = targetRel;
                nextFOCost[jC] = targetCost;
                jC++;
                nextKAO[iC]++;
            }
            iC++;
        }

        // 4. Меняем местами указатели буферов для следующей итерации (очень быстрая операция)
        curKAO.swap(nextKAO);
        curFO.swap(nextFO);
        curFORel.swap(nextFORel);
        curFOCost.swap(nextFOCost);

        vNumb = nextVertNumb;
        eNumb = nextEdgNumb;
    }

    // 5. Если граф был упрощен, ОДИН РАЗ выделяем реальную память и применяем к объекту
    if (anyChanges) {
        u_int* nKAO; u_int* nFO; double* nFORel; double* nFOCost;
        memallocNewGraph(vNumb, eNumb, nKAO, nFO, nFORel, nFOCost);

        std::copy(curKAO.begin(), curKAO.begin() + vNumb + 1, nKAO);
        std::copy(curFO.begin(), curFO.begin() + 2 * eNumb, nFO);
        std::copy(curFORel.begin(), curFORel.begin() + 2 * eNumb, nFORel);
        std::copy(curFOCost.begin(), curFOCost.begin() + 2 * eNumb, nFOCost);

        memupdate(vNumb, eNumb, nKAO, nFO, nFORel, nFOCost);
    }
}


bool Graph::isConnective() const {
    if (vertNumb <= 1) {
        return true;
    }

    // thread_local создаст память только ОДИН раз для каждого потока
    // В следующих вызовах функции память просто переиспользуется без new/delete
    thread_local std::vector<bool> visited;
    visited.assign(vertNumb + 1, false);

    thread_local std::vector<u_int> q;
    q.clear(); // Быстрая очистка без освобождения памяти

    q.push_back(1);
    visited[1] = true;

    u_int visitedSum = 1;
    size_t head = 0; // Индекс начала очереди (замена pop_front для вектора)

    while (head < q.size()) {
        if (visitedSum == vertNumb) {
            return true;
        }
        u_int currentEdge = q[head++]; // Достаем элемент

        for (u_int i = KAO[currentEdge - 1]; i < KAO[currentEdge]; i++) {
            u_int neighbor = FO[i];
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                visitedSum++;
                q.push_back(neighbor);
            }
        }
    }

    return (visitedSum == vertNumb);
}

double Graph::baseProbabilities() const {
    u_int SE;
    double result = 0;
    if (vertNumb == 1) {
        result = 1.0;
    }
    else if (vertNumb == 2) {
        result = FORel[0];
    }
    else if (vertNumb == 3) {
        double R[2][2];
        for (u_int u = 1; u < 3; u++) {
            for (u_int v = u + 1; v < 4; v++) {
                bool notEdge = true;
                for (u_int k = KAO[u - 1]; k < KAO[u]; k++) {
                    if (FO[k] == v) {   
                        notEdge = false;
                        SE = k;
                    }
                }
                if (notEdge) {
                    SE = edgNumb * 2;
                }
                if (SE < edgNumb * 2) {
                    R[u-1][v-2] = 1 - FORel[SE];
                }
                else { 
                    R[u-1][v-2] = 1;
                }  
            }
        }
        result = (R[0][0]*R[0][1]*R[1][1]+(1-R[0][0])*R[0][1]*R[1][1]+R[0][0]*(1-R[0][1])*R[1][1]+R[0][0]*R[0][1]*(1-R[1][1]));
    }
    else if (vertNumb == 4) {
        double a, b, c, d, e, f;
        double R[3][3];
        for (u_int u = 1; u < 4; u++) {
            for (u_int v = u + 1; v < 5; v++) {
                bool notEdge = true;
                for (u_int k = KAO[u - 1]; k < KAO[u]; k++) {
                    if (FO[k] == v) {   
                        notEdge = false;
                        SE = k;
                    }
                }
                if (notEdge) {
                    SE = edgNumb * 2;
                }
                if (SE < edgNumb * 2) {
                    R[u - 1][v - 2] = 1 - FORel[SE];
                }
                else { 
                    R[u - 1][v - 2] = 1;
                }  
            }
        }
        a = R[0][0]; b = R[1][1]; c = R[2][2]; d = R[0][2]; e = R[1][2]; f = R[0][1];
        result = 1 - (6*a*b*c*d*e*f - 2*(b*d*e*f*(a + c - 0.5) + a*c*e*f*(b + d - 0.5) + a*b*c*d*(e + f - 0.5)) + a*b*e + a*d*f + b*c*f + c*d*e);
    }
    else if (vertNumb == 5) {
        double a , b, c, d, e, f, q, h, k, l, 
                k1, k2, k3, k4, k5, k6, k7, k8, 
                k9, k10, k11, k12, k13, k14, k15;
        double R[4][4];
        for (u_int u = 1; u < 5; u++) {
            for (u_int v = u + 1; v < 6; v++) {
                bool notEdge = true;
                for (u_int k = KAO[u-1]; k < KAO[u]; k++) {
                    if (FO[k] == v) {   
                        notEdge = false;
                        SE = k;
                    }
                }
                if (notEdge) {
                    SE = edgNumb * 2;
                }
                if (SE < (edgNumb * 2)) {
                    R[u-1][v-2] = 1 - FORel[SE];
                }
                else { 
                    R[u-1][v-2] = 1;
                }  
            }
        }
        a = R[0][0]; b = R[0][1]; c = R[0][2]; d = R[0][3]; e = R[1][1]; 
        f = R[1][2]; q = R[1][3]; h = R[2][2]; k = R[2][3]; l = R[3][3];
        k1 = 1 - e*(f*q + h*k);
        k2 = 1 - h*(b*k + c*l);
        k3 = 1 - l*(c*f + d*q);
        k4 = 1 - d*(a*b + q*k);
        k5 = 1 - a*(b*c + e*f);
        k6 = (1-a)*(1-h)*(1-k) + (1-a)*(1-l)*((1-h)*k+h*(1-k)) + a*h*k*(1-4*l);
        k7 = (1-c)*(1-d)*(1-e) + (1-e)*(1-l)*((1-c)*d+c*(1-d)) + d*e*l;
        k8 = (1-a)*(1-d)*(1-h) + (1-q)*(1-h)*((1-a)*d+a*(1-d)) + a*h*q;
        k9 = (1-a)*(1-b)*(1-l) + (1-e)*(1-l)*((1-a)*b+a*(1-b)) + a*e*l;
        k10 = (1-e)*(1-d)*(1-f) + (1-d)*(1-h)*((1-e)*f+e*(1-f)) + d*f*h;
        k11 = (1-b)*(1-f)*(1-q) + (1-b)*(1-l)*((1-f)*q+f*(1-q)) + b*q*l;
        k12 = (1-c)*(1-e)*(1-q) + (1-c)*(1-k)*((1-e)*q+e*(1-q)) + c*e*k;
        k13 = (1-b)*(1-d)*(1-f) + (1-f)*(1-k)*((1-b)*d+b*(1-d)) + b*d*f;
        k14 = (1-b)*(1-c)*(1-q) + (1-q)*(1-h)*((1-b)*c+b*(1-c)) + b*c*q;
        k15 = (1-a)*(1-c)*(1-k) + (1-f)*(1-k)*((1-a)*c+a*(1-c)) + c*f*k;

        result =  1 - (b*c*(a*d*k1+f*e*(d*q*k6+k*l*k8)) +
                    f*q*(a*e*k2+h*k*(a*b*k7+c*d*k9)) +
                    b*h*(e*k*k3+d*l*(a*f*k12+e*q*k15)) +
                    c*l*(f*h*k4+a*q*(b*k*k10+e*h*k13)) +
                    d*k*(q*l*k5+a*e*(c*h*k11+f*l*k14)));
    }

    return result;
}

u_int Graph::getVertNumb() const {
    return vertNumb;
}

u_int Graph::getEdgNumb() const {
    return edgNumb;
}

u_int Graph::deg(u_int i) const {
    if ((i == 0) || (i > edgNumb)) {
        return 0;       //raise exception here
    }
    return (KAO[i] - KAO[i - 1]);
}

std::tuple<u_int, u_int, double,double> Graph::selectEdge() const {
        u_int u = 0, v = 0;        // Вершины ребра
        double pe = 0.0;           // Вероятность существования ребра
        double ce = 0.0;           // Стоимость ребра (добавлено)
        u_int minDegreeSum = std::numeric_limits<u_int>::max(); // Минимальная сумма степеней

        // Итерируемся по всем вершинам графа
        for (u_int i = 1; i <= vertNumb; ++i) {
            for (u_int j = KAO[i - 1]; j < KAO[i]; ++j) {
                u_int neighbor = FO[j]; // Смежная вершина

                // Рассматриваем каждое ребро только один раз (u < v)
                if (i < neighbor) {
                    // Сумма степеней двух вершин
                    u_int degreeSum = (KAO[i] - KAO[i - 1]) + (KAO[neighbor] - KAO[neighbor-1]);

                    // Если нашли новую минимальную сумму степеней
                    if (degreeSum < minDegreeSum) {
                        minDegreeSum = degreeSum;
                        u = i;
                        v = neighbor;
                        pe = FORel[j];   // Запоминаем вероятность
                        ce = FOCost[j];  // Запоминаем стоимость (добавлено)
                    }
                }
            }
        }

        // Если ребро не найдено
        if (u == 0 && v == 0) {
            std::cerr << "Warning: No suitable edge found!" << std::endl;
        }


        return { u, v, pe, ce }; // Возвращаем кортеж из 4-х элементов
}


Graph* Graph::addEdge(u_int u, u_int v, double rel, double cost) const {
    // 1. Валидация
    if (u == 0 || v == 0 || u == v || isEdge(u, v)) {
        return nullptr;
    }

    u_int maxV = (u > v) ? u : v;
    u_int newVertNumb = (maxV > vertNumb) ? maxV : vertNumb;
    u_int newEdgNumb = edgNumb + 1;

    // 2. Выделяем новые массивы
    u_int* newKAO = new u_int[newVertNumb + 1];
    u_int* newFO = new u_int[2 * newEdgNumb];
    double* newFORel = new double[2 * newEdgNumb];
    double* newFOCost = new double[2 * newEdgNumb];

    // 3. Считаем новые степени вершин
    newKAO[0] = 0;
    for (u_int i = 1; i <= newVertNumb; ++i) {
        u_int oldDeg = (i <= vertNumb) ? (KAO[i] - KAO[i - 1]) : 0;
        u_int newDeg = oldDeg;

        // Добавляем по 1 ребру для u и v
        if (i == u) newDeg++;
        if (i == v) newDeg++;  // Используем if, а не else if!

        newKAO[i] = newKAO[i - 1] + newDeg;
    }

    // 4. Копируем и добавляем ребра
    u_int jC = 0;

    for (u_int i = 1; i <= newVertNumb; ++i) {
        // Копируем старые ребра
        if (i <= vertNumb) {
            for (u_int j = KAO[i - 1]; j < KAO[i]; ++j) {
                newFO[jC] = FO[j];
                newFORel[jC] = FORel[j];
                newFOCost[jC] = FOCost[j];
                jC++;
            }
        }

        // Добавляем новое ребро если это u
        if (i == u) {
            newFO[jC] = v;
            newFORel[jC] = rel;
            newFOCost[jC] = cost;
            jC++;
        }

        // Добавляем новое ребро если это v (отдельный if!)
        if (i == v) {
            newFO[jC] = u;
            newFORel[jC] = rel;
            newFOCost[jC] = cost;
            jC++;
        }
    }

    // Финальная проверка целостности
    Graph* result = new Graph(newVertNumb, newEdgNumb, newKAO, newFO, newFORel, newFOCost);
    return result;
}

    Graph* Graph::createEmptyGraph(u_int vertNumb, u_int edgNumb) {
    u_int *newKAO = new u_int[vertNumb + 1];
    u_int *newFO = new u_int[2 * edgNumb];
    double *newFORel = new double[2 * edgNumb];
    double* newFOCost = new double[2 * edgNumb];
    return new Graph(vertNumb, edgNumb, newKAO, newFO, newFORel,newFOCost);
}

Graph* Graph::createFromEdges(u_int vertNumb, const std::vector<PotentialEdge>& edges) {
        u_int edgNumb = edges.size();

        // 1. Выделяем память ОДИН РАЗ
        u_int* newKAO = new u_int[vertNumb + 1];
        u_int* newFO = new u_int[2 * edgNumb];
        double* newFORel = new double[2 * edgNumb];
        double* newFOCost = new double[2 * edgNumb];

        // 2. Считаем степени вершин (сколько ребер исходит из каждой)
        std::vector<u_int> degrees(vertNumb + 1, 0);
        for (const auto& edge : edges) {
            degrees[edge.u]++;
            degrees[edge.v]++;
        }

        // 3. Заполняем массив KAO (префиксные суммы)
        newKAO[0] = 0;
        for (u_int i = 1; i <= vertNumb; ++i) {
            newKAO[i] = newKAO[i - 1] + degrees[i];
        }

        // 4. Расставляем ребра в FO
        // Создаем копию KAO, чтобы отслеживать текущую свободную позицию для каждой вершины
        std::vector<u_int> currentPos(vertNumb + 1);
        for (u_int i = 1; i <= vertNumb; ++i) {
            currentPos[i] = newKAO[i-1];
        }

        for (const auto& edge : edges) {
            u_int u = edge.u;
            u_int v = edge.v;

            // Добавляем связь (u -> v)
            u_int posU = currentPos[u]++;
            newFO[posU] = v;
            newFORel[posU] = edge.p;
            newFOCost[posU] = edge.cost;

            // Добавляем связь (v -> u)
            u_int posV = currentPos[v]++;
            newFO[posV] = u;
            newFORel[posV] = edge.p;
            newFOCost[posV] = edge.cost;
        }

        return new Graph(vertNumb, edgNumb, newKAO, newFO, newFORel, newFOCost);
}


Graph *Graph::mutateOneEdge() const
{
    // If there are no edges at all — nothing to mutate
    if (edgNumb < 1) {
        std::cerr << "Graph::mutateOneEdge(): no edges to mutate!\n";
        return nullptr;
    }

    // Limit the number of random selection attempts
    const int MAX_TRIES = 1000;
    for (int tryCount = 0; tryCount < MAX_TRIES; ++tryCount)
    {
        // 1) Randomly take an index in FO
        //    (from 0 to 2*edgNumb - 1)
        u_int randomIndex = rand() % (2 * edgNumb);

        // Determine which vertex u owns this segment of FO
        // Insurance against going out of bounds (if KAO[u] == randomIndex, etc.)
        u_int u = 1;
//        std::cout << "Before while " << randomIndex << std::endl;
        while (u <= vertNumb && KAO[u] <= randomIndex) {
            u++;
        }
//        std::cout << "After while " << randomIndex << std::endl;
        // Find v
//        std::cout << "check randIdx " << randomIndex << std::endl;
        u_int v = FO[randomIndex];
//        std::cout << "check randIdx " << randomIndex << std::endl;

        // 2) Remove edge (u, v)
//        std::cout << "cut " << randomIndex << std::endl;
        Graph *cutedGraph = this->cutEdge(u, v);
//        std::cout << "cut " << randomIndex << std::endl;
        if (!cutedGraph) {
            // If nullptr — unlucky, try again
            continue;
        }

        // 2a) Check if the graph became disconnected **after removal**
        //     If disconnected, discard and try the next attempt
//        if (!cutedGraph->isConnective()) {
//            delete cutedGraph;
//            continue;
//        }

        // 3) Try to insert a new edge (x,y) that doesn't exist, and see if the graph remains connected
        for (int addTry = 0; addTry < 50; addTry++)
        {
            u_int x = 1 + rand() % cutedGraph->vertNumb;
            u_int y = 1 + rand() % cutedGraph->vertNumb;
            if (x == y)
                continue;
            if (cutedGraph->isEdge(x, y))
                continue;

            // Try to insert (x,y)
//            std::cout << "add " << randomIndex << std::endl;
            Graph *addedGraph = cutedGraph->addEdge(x, y);
//            std::cout << "add " << randomIndex << std::endl;
            if (!addedGraph) {
                // If returned nullptr, addEdge didn't work (happens if the edge already exists)
                continue;
            }

            // Check connectivity of the final variant
            // Check connectivity of the final variant
            if (addedGraph->isConnective()) {
                // Success! Found a variant where "removed one edge, inserted another" — and the graph is still connected.
                delete cutedGraph;
                return addedGraph;
            }
            // Otherwise, delete and keep searching
            delete addedGraph;
        }

        // If after 50 attempts to add an edge we didn't find a connected result — delete cutedGraph and go to the next global try
        delete cutedGraph;
    }

    // If we couldn't find a connected result after MAX_TRIES, return nullptr.
    // In the calling code, you can check this nullptr and decide what to do next.
    std::cerr << "mutateOneEdge(): failed to find connected mutation after many tries.\n";
    return nullptr;
}


Graph* Graph::mutateNetworkGrowth(u_int originalVertCount, const std::vector<PotentialEdge>& allowedEdges, double maxCost) const
{
    // 1. Собираем ребра, которые можно удалять (связанные с новыми вершинами)
    std::vector<std::pair<u_int, u_int>> removableEdges;
    for (u_int i = 1; i <= vertNumb; ++i) {
        for (u_int j = KAO[i - 1]; j < KAO[i]; ++j) {
            u_int neighbor = FO[j];
            if (i < neighbor) {
                if (i > originalVertCount || neighbor > originalVertCount) {
                    removableEdges.push_back({ i, neighbor });
                }
            }
        }
    }

    if (removableEdges.empty()) return nullptr;

    const int MAX_TRIES = 50;
    for (int tryCount = 0; tryCount < MAX_TRIES; ++tryCount) {

        // --- ШАГ 1: Удаление одного случайного ребра ---
        int randIdx = rand() % removableEdges.size();
        auto [u_del, v_del] = removableEdges[randIdx];

        Graph* currentGraph = this->cutEdge(u_del, v_del);
        if (!currentGraph) continue;

        // --- ШАГ 2: Жадное добавление нескольких ребер ---
        double currentTotalCost = currentGraph->getTotalCost();
        double budgetLeft = maxCost - currentTotalCost;

        bool changed = false;

        // Перемешиваем пул кандидатов, чтобы не выбирать всегда одни и те же
        std::vector<size_t> indices(allowedEdges.size());
        std::iota(indices.begin(), indices.end(), 0);
        // Создаем качественный генератор случайных чисел
        // static нужен, чтобы не инициализировать его при каждом вызове мутации
        static std::random_device rd;
        static std::mt19937 g(rd());

        std::shuffle(indices.begin(), indices.end(),g);

        for (size_t k : indices) {
            const auto& edge = allowedEdges[k];

            // Проверки:
            // 1. Подходит ли по бюджету
            if (edge.cost > budgetLeft) continue;
            // 2. Не существует ли уже
            if (currentGraph->isEdge(edge.u, edge.v)) continue;
            // 3. Не то ли это ребро, которое мы только что удалили
            if ((edge.u == u_del && edge.v == v_del) || (edge.u == v_del && edge.v == u_del)) continue;

            // Добавляем ребро
            Graph* nextStepGraph = currentGraph->addEdge(edge.u, edge.v, edge.p, edge.cost);
            if (nextStepGraph) {
                delete currentGraph;
                currentGraph = nextStepGraph;
                budgetLeft -= edge.cost;
                changed = true;

                // Опционально: можно добавить вероятность выхода, 
                // чтобы не всегда забивать бюджет "под завязку"
                if (rand() % 100 < 20) break;
            }
        }

        // --- ШАГ 3: Финальная проверка ---
        // Если мы что-то изменили и граф в итоге остался связным
        if (changed && currentGraph->isConnective()) {
            return currentGraph;
        }

        delete currentGraph;
    }

    return nullptr;
}

Graph* Graph::mutateKNetworkGrowth(u_int originalVertCount, const std::vector<PotentialEdge>& allowedEdges, double maxCost, unsigned K) const
{
    // 1. Собираем ребра, которые можно удалять (связанные с новыми вершинами)
    std::vector<std::pair<u_int, u_int>> removableEdges;
    for (u_int i = 1; i <= vertNumb; ++i) {
        for (u_int j = KAO[i - 1]; j < KAO[i]; ++j) {
            u_int neighbor = FO[j];
            if (i < neighbor) {
                if (i > originalVertCount || neighbor > originalVertCount) {
                    removableEdges.push_back({ i, neighbor });
                }
            }
        }
    }

    if (removableEdges.empty()) return nullptr;

    const int MAX_TRIES = 50;
   

    for (int tryCount = 0; tryCount < MAX_TRIES; ++tryCount) {

        // --- ШАГ 1: Удаление K случайных ребер ---

        // Берем K, но защищаемся от случая, когда K больше, чем всего новых ребер
        int edgesToDrop = std::min((int)removableEdges.size(), (int)K);

        Graph* mutatedGraph = nullptr;
        std::vector<std::pair<u_int, u_int>> recentlyDeleted;
        std::vector<std::pair<u_int, u_int>> availableToRemove = removableEdges;

        for (int d = 0; d < edgesToDrop; ++d) {
            if (availableToRemove.empty()) break;

            int randIdx = math::random_int(0, availableToRemove.size() - 1);
            auto [u_del, v_del] = availableToRemove[randIdx];

            recentlyDeleted.push_back({ u_del, v_del });

            // Удаляем из пула доступных, чтобы не выбрать дважды
            availableToRemove.erase(availableToRemove.begin() + randIdx);

            const Graph* sourceGraph = mutatedGraph ? mutatedGraph : this;

            Graph* nextStepGraph = sourceGraph->cutEdge(u_del, v_del);
            if (!nextStepGraph) continue;

            if (mutatedGraph) delete mutatedGraph;
            mutatedGraph = nextStepGraph;
        }

        if (!mutatedGraph) continue;

        // --- ШАГ 2: Жадное добавление нескольких ребер ---
        double currentTotalCost = mutatedGraph->getTotalCost();
        double budgetLeft = maxCost - currentTotalCost;
        bool changed = false;

        std::vector<size_t> indices(allowedEdges.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), math::get_random_generator());

        for (size_t k : indices) {
            const auto& edge = allowedEdges[k];

            if (edge.cost > budgetLeft) continue;
            if (mutatedGraph->isEdge(edge.u, edge.v)) continue;

            // Защита от возврата только что удаленных ребер
            bool isDeleted = false;
            for (const auto& del : recentlyDeleted) {
                if ((edge.u == del.first && edge.v == del.second) ||
                    (edge.u == del.second && edge.v == del.first)) {
                    isDeleted = true;
                    break;
                }
            }
            if (isDeleted) continue;

            Graph* nextStepGraph = mutatedGraph->addEdge(edge.u, edge.v, edge.p, edge.cost);
            if (nextStepGraph) {
                delete mutatedGraph;
                mutatedGraph = nextStepGraph;
                budgetLeft -= edge.cost;
                changed = true;

                // Вероятность досрочного выхода, чтобы не всегда добивать бюджет под 100%
                if (math::random_int(0,99) < 20) break;
            }
        }

        // --- ШАГ 3: Финальная проверка ---
        if (changed && mutatedGraph->isConnective()) {
            return mutatedGraph;
        }

        delete mutatedGraph;
    }

    return nullptr;
}


double Graph::getTotalCost() const {
    double sum = 0.0;
    // В вашем коде FO и FOCost имеют размер 2 * edgNumb
    for (u_int i = 0; i < 2 * edgNumb; i++) {
        sum += FOCost[i];
    }
    return sum / 2.0; // Делим на 2, так как каждое ребро учтено для обеих вершин
}








    Graph *Graph::mutateKEdges(u_int K) const
{
    if (edgNumb < 1) {
        std::cerr << "Graph::mutateKEdges(): no edges to mutate!\n";
        return nullptr;
    }

    Graph *newGraph = new Graph(*this);

    for (u_int i = 0; i < K; i++)
    {
        Graph *tmp = newGraph->mutateOneEdge();
        if (!tmp) {
            std::cerr << "mutateKEdges(): failed on step " << i << " of " << K << "\n";
            delete newGraph;
            return nullptr;
        }

        // In place of copying the object, change the pointer
        delete newGraph;
        newGraph = tmp;
    }

    return newGraph;
}
   

}