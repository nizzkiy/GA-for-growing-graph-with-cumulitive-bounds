#include <mimalloc-override.h>
#include "GraphUtil.h"
#include "Graph.h"
#include "random.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <random>
#include <iomanip>
#include <atomic>
#include <set>
#include <utility>

using namespace math;

// --- Структуры данных для загрузки ---

struct SimulationParams {
    unsigned K = 1;             // Количество мутируемых ребер за раз
    unsigned POP = 50;          // Размер популяции
    unsigned MAX_BAD = 50;      // Критерий остановки (число неудачных поколений)
    double MAX_BUDGET = 1000.0; // Максимальная стоимость
    int THREADS = 4;            // Число потоков
    bool usePorog = true;
};

struct NetworkTask {
    Graph* baseGraph = nullptr; // Исходный неизменяемый граф
    unsigned originalVerts = 0; // Число старых вершин
    unsigned newVertsToAdd = 0; // Число новых вершин
    std::vector<PotentialEdge> allowedEdges; // Пул возможных ребер
};

// Структура для кэширования значений надежности
struct Individual {
    Graph* graph = nullptr;
    double exactRL = 0.0;
};

// --- Функции чтения файлов ---

bool loadSimulationParams(const std::string& filename, SimulationParams& params) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string varName;
    while (file >> varName) {
        if (varName == "mutated_edges_k") file >> params.K;
        else if (varName == "population_size") file >> params.POP;
        else if (varName == "max_bad_iters") file >> params.MAX_BAD;
        else if (varName == "max_budget") file >> params.MAX_BUDGET;
        else if (varName == "threads") file >> params.THREADS;
        else if (varName == "use_cumulitive_bounds") file >> std::boolalpha >> params.usePorog;
    }
    return true;
}

bool loadNetworkData(const std::string& filename, NetworkTask& task) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    // 1. Считываем базовый граф через ваш оператор >>
    task.baseGraph = new Graph();
    file >> *(task.baseGraph); // Оператор считает N, M, KAO, FO, Rel, Cost

  

    task.originalVerts = task.baseGraph->getVertNumb();

    // 2. Считываем оставшуюся часть файла вручную
    std::string marker;
    while (file >> marker) {
        if (marker == "new_vertices_count") {
            file >> task.newVertsToAdd;
        }
        else if (marker == "allowed_edges_pool") {
            unsigned poolSize;
            file >> poolSize;
            task.allowedEdges.resize(poolSize);
            for (unsigned i = 0; i < poolSize; ++i) {
                // Считываем параметры потенциальных ребер
                file >> task.allowedEdges[i].u
                    >> task.allowedEdges[i].v
                    >> task.allowedEdges[i].p
                    >> task.allowedEdges[i].cost;
            }
        }
    }
    file.close();
    return true;
}

// Функция возвращает пару {v1, v2}, если разрез ровно из двух вершин.
// Если условие не выполняется (вершин 1, 3 или 0), возвращает {0, 0}.
std::pair<int, int> check2VerCut(const NetworkTask& task) {
    std::set<int> contactVertices;

    for (const auto& edge : task.allowedEdges) {
        // Проверяем, принадлежит ли каждая вершина ребра начальному графу
        bool u_is_old = (edge.u <= task.originalVerts);
        bool v_is_old = (edge.v <= task.originalVerts);

        // Если ребро соединяет начальный граф и новые вершины
        if (u_is_old && !v_is_old) {
            contactVertices.insert(edge.u);
        }
        else if (!u_is_old && v_is_old) {
            contactVertices.insert(edge.v);
        }
    }

    // Если контактных вершин начального графа ровно две
    if (contactVertices.size() == 2) {
        auto it = contactVertices.begin();
        int v1 = *it;
        std::advance(it, 1);
        int v2 = *it;
        return { v1, v2 };
    }

    // В противном случае возвращаем нули как индикатор отсутствия 2-вершинного разреза
    return { 0, 0 };
}

// Функция возвращает пару {Надежность базового графа, Надежность базового со стянутым разрезом}
std::pair<double, double> calcBaseGraphReliabilities(const NetworkTask& task, std::pair<int, int> cutVertices) {
    // Базовая проверка на корректность входных данных
    if (cutVertices.first == 0 || cutVertices.second == 0 || task.baseGraph == nullptr) {
        return { 0.0, 0.0 }; // Разреза нет или граф не загружен
    }

    int u = cutVertices.first;
    int v = cutVertices.second;

    // 1. Считаем точную надежность начального графа
    double baseRL = calcBounds(*(task.baseGraph), 0.0).RL;

    // 2. Считаем надежность начального графа со стянутыми вершинами разреза
    Graph* tempGraph = nullptr;
    Graph* mergedGraph = nullptr;
    double mergedRL = 0.0;

    // Проверяем, существует ли уже ребро между контактными вершинами
    if (!task.baseGraph->isEdge(u, v)) {
        // Ребра нет -> добавляем фиктивное (Надежность 1.0, Стоимость 0.0)
        tempGraph = task.baseGraph->addEdge(u, v, 1.0, 0.0);
        if (tempGraph) {
            // Стягиваем граф по добавленному ребру
            mergedGraph = tempGraph->mergeEdge(u, v);
        }
    }
    else {
        // Ребро уже есть -> просто стягиваем вершины в исходном графе
        mergedGraph = task.baseGraph->mergeEdge(u, v);
    }

    // Если стягивание прошло успешно, высчитываем надежность
    if (mergedGraph) {
        mergedRL = calcBounds(*mergedGraph, 0.0).RL;
        delete mergedGraph; // Обязательно чистим память за mergedGraph
    }

    // Чистим память за временным графом (если мы его создавали через addEdge)
    if (tempGraph) {
        delete tempGraph;
    }

    return { baseRL, mergedRL };
}

// Вычисление надежности полного графа через декомпозицию 2-вершинного разреза
double calcFullReliabilityDecomp(const NetworkTask& task, const Graph& G,
    std::pair<int, int> cutVertices,
    double R_G1, double R_G1_prime) {
    int u = cutVertices.first;
    int v = cutVertices.second;

    // 1. Формируем базис для G2. 
    // В G2 войдут две контактные вершины (u и v) и все новые вершины.
    int V2 = 2 + task.newVertsToAdd;

    // Лямбда для маппинга номеров вершин из большого графа G в локальный G2 (от 1 до V2)
    // u -> 1, v -> 2, новые вершины -> 3, 4, ... V2
    auto mapV = [&](int oldV) -> int {
        if (oldV == u) return 1;
        if (oldV == v) return 2;
        if (oldV > task.originalVerts) {
            return oldV - task.originalVerts + 2;
        }
        return 0; // Защита от непредвиденных старых вершин
    };

    // 2. Ищем все добавленные рёбра, которые присутствуют в текущей особи (G)
    std::vector<PotentialEdge> g2_edges;
    for (const auto& edge : task.allowedEdges) {
        if (G.isEdge(edge.u, edge.v)) {
            int mu = mapV(edge.u);
            int mv = mapV(edge.v);
            // Берем только те ребра, которые лежат внутри G2
            if (mu != 0 && mv != 0) {
                g2_edges.push_back(edge);
            }
        }
    }

    int E2 = g2_edges.size();
    if (E2 == 0) return 0.0; // Если новых ребер нет, то новые вершины повисли в воздухе

    // 3. Собираем массивы для создания объекта Graph G2
    // Выделяем память так, чтобы деструктор Graph корректно её освободил (через delete[])
    u_int* newKAO = new u_int[V2 + 1];
    u_int allocFO = std::max(1, 2 * E2); // Защита от нулевого размера
    u_int* newFO = new u_int[allocFO];
    double* newFORel = new double[allocFO];
    double* newFOCost = new double[allocFO];

    // Группируем смежные вершины для формирования FO
    std::vector<std::vector<std::tuple<int, double, double>>> adj(V2 + 1);
    for (const auto& edge : g2_edges) {
        int mu = mapV(edge.u);
        int mv = mapV(edge.v);
        adj[mu].push_back({ mv, edge.p, edge.cost });
        adj[mv].push_back({ mu, edge.p, edge.cost });
    }

    newKAO[0] = 0;
    int jC = 0;
    for (int i = 1; i <= V2; ++i) {
        newKAO[i] = newKAO[i - 1] + adj[i].size();
        for (const auto& e : adj[i]) {
            newFO[jC] = std::get<0>(e);
            newFORel[jC] = std::get<1>(e);
            newFOCost[jC] = std::get<2>(e);
            jC++;
        }
    }

    // Создаем экземпляр G2
    Graph* G2 = new Graph(V2, E2, newKAO, newFO, newFORel, newFOCost);

    // 4. Считаем R(G2)
    double R_G2 = calcBounds(*G2, 0.0).RL;

    // 5. Создаем G2' (стягиваем вершины 1 и 2) и считаем R(G2')
    Graph* G2_merged = nullptr;
    Graph* tempG2 = nullptr;

    // Проверяем, есть ли уже ребро между контактными вершинами в G2
    if (!G2->isEdge(1, 2)) {
        // Если нет, создаем фиктивное ребро (надежность 1.0), чтобы mergeEdge сработал
        tempG2 = G2->addEdge(1, 2, 1.0, 0.0);
        if (tempG2) {
            G2_merged = tempG2->mergeEdge(1, 2);
        }
    }
    else {
        G2_merged = G2->mergeEdge(1, 2);
    }

    double R_G2_prime = 0.0;
    if (G2_merged) {
        R_G2_prime = calcBounds(*G2_merged, 0.0).RL;
    }

    // 6. Считаем итоговую вероятность G по формуле декомпозиции
    double R_total = R_G1 * (R_G2_prime - R_G2) +
        R_G2 * (R_G1_prime - R_G1) +
        R_G1 * R_G2;

    // 7. Очищаем память
    delete G2;
    if (tempG2) delete tempG2;
    if (G2_merged) delete G2_merged;

    return R_total;
}


// --- Генерация начальной популяции СРАЗУ СВЯЗНЫМ графом
Graph* generateConnectiveIndividual(const NetworkTask& task, double maxBudget)
{
    Graph* g = new Graph(*task.baseGraph); //Копия базового графа

    int unaddedVerts = task.newVertsToAdd;

    //Массив добавленных и ещё не добавленных вершин
    std::vector<bool> inGraph(task.newVertsToAdd+task.originalVerts+1,false);
    for (int i = 1; i <= task.originalVerts; i++) inGraph[i] = true;

    //Копия пула возможных рёбер к добавлению
    std::vector<PotentialEdge> AllowedEdges = task.allowedEdges; 
    //Перемешиваем чтобы начальная популяция была разнообразной
    std::shuffle(AllowedEdges.begin(), AllowedEdges.end(), math::get_random_generator());

    //Основной цикл поиска ребра для добавления
    while (unaddedVerts>0)
    {
        bool EdgeWasAdd = 0;
        for (int i = 0; i < AllowedEdges.size(); i++)
        {
            int u = AllowedEdges[i].u; //Берем первую вершину ребра
            int v = AllowedEdges[i].v; //берем вторую вершину ребра
            
            //Проверка что первая вершина ребра ещё не присоединена
            if (inGraph[u]==false)
            {
                //Теперь проверяем, что вторая вершина присоединена уже
                
                if (inGraph[v]==true)
                {
                    //Успех

                    //Проверим бюджет
                    if (g->getTotalCost() + AllowedEdges[i].cost > maxBudget) continue;

                    //Обновляем граф
                    Graph* temp = g->addEdge(u, v, AllowedEdges[i].p, AllowedEdges[i].cost);
                    if (temp)
                    {
                        delete g;
                        g = temp;
                        EdgeWasAdd = 1;
                        //говорим что вершина u присоединена
                        inGraph[u] = true;
                        unaddedVerts--;

                        //Удаляем ребро, которое мы использовали

                        AllowedEdges.erase(AllowedEdges.begin() + i);
                        break;
                    }

                }
                else continue;
            }

            //Если мы тут значит первая вершина уже добавлена
            //Тогда проверяем что вторая вершина ещё не добавлена
            else
            {
                
                if (inGraph[v]==false)
                {
                    //Успех
                    //Проверим бюджет
                    if (g->getTotalCost() + AllowedEdges[i].cost > maxBudget) continue;


                    //Обновляем граф
                    Graph* temp = g->addEdge(u, v, AllowedEdges[i].p, AllowedEdges[i].cost);
                    if (temp)
                    {
                        delete g;
                        g = temp;
                        EdgeWasAdd = 1;
                        //Говорим что вершину v мы присоединили
                        inGraph[v] = true;
                        unaddedVerts--;
                        //Удаляем ребро, которое мы использовали

                        AllowedEdges.erase(AllowedEdges.begin() + i);

                        break;
                    }
                }
            }

        }

        //Прошлись по всем рёбрам, или выкинуло сюда после успеха, делаем проверку что мы изменили граф
        //Если не смогли добавить ребро, удаляем
        if (EdgeWasAdd == 0)
        {
            delete g;
            return nullptr;
        }
    }

    // --- ДОБАВЛЕНИЕ (Жадное уплотнение графа) ---
    // Теперь, когда граф связный, накидываем случайные ребра из остатков пула, 
    // пока не упремся в бюджет.
    for (size_t i = 0; i < AllowedEdges.size(); i++) {
        int u = AllowedEdges[i].u;
        int v = AllowedEdges[i].v;

        if (g->getTotalCost() + AllowedEdges[i].cost <= maxBudget) {
            Graph* temp = g->addEdge(u, v, AllowedEdges[i].p, AllowedEdges[i].cost);
            if (temp) {
                delete g;
                g = temp;
                // Ребро добавлено, идем к следующему в пуле
            }
        }
    }
    // ---------------------------------------------
 
    return g;
}


// --- Генерация начальной популяции ---

Graph* generateFeasibleIndividual(const NetworkTask& task, double maxBudget) {
    // 1. Берем копию базового графа
    Graph* g = new Graph(*task.baseGraph); 

    // Если в базовом графе меньше вершин, чем total (old + new), 
    // нужно "раздуть" его. Проще всего добавить фиктивное ребро и удалить, 
    // или использовать addEdge для первой связи.

    // Стратегия: Случайно выбираем ребра из пула, пока:
    // а) Граф не станет связным (все новые вершины подключены)
    // б) Не превышен бюджет

    std::vector<int> poolIndices(task.allowedEdges.size());
    for (size_t i = 0; i < poolIndices.size(); ++i) poolIndices[i] = i;

    // Перемешиваем пул, чтобы выбор был случайным
    std::shuffle(poolIndices.begin(), poolIndices.end(), std::mt19937(std::random_device{}()));

    for (int idx : poolIndices) {
        const auto& edge = task.allowedEdges[idx];

        // Проверка бюджета ДО добавления
        if (g->getTotalCost() + edge.cost > maxBudget) continue;

        // Добавляем ребро
        Graph* temp = g->addEdge(edge.u, edge.v, edge.p, edge.cost);
        if (temp) {
            delete g; // Удаляем старую версию
            g = temp; // Обновляем указатель
        }

        // Проверяем, достаточно ли мы добавили?
        // Критерий: в графе должно быть (old + new) вершин и он должен быть связным
        // isConnective проверяет связность всех имеющихся вершин.
        // Но нам важно, чтобы "новые" вершины вообще появились в графе.
        if (g->getVertNumb() == task.originalVerts + task.newVertsToAdd) {
            if (g->isConnective()) {
                return g; // Успех!
            }
        }
    }

    // Если прошли весь пул и не собрали связный граф или превысили бюджет
    delete g;
    return nullptr;
}

// --- Основной цикл Мутации (с кумулятивной проверкой) ---

void mutationStep(std::vector<Individual>& pop, const NetworkTask& task, const SimulationParams& params, int &SumOfCancel,bool have_2_VertCut,double cachedBaseRL,
 double cachedMergedRL,std::pair<int,int> cutVertices, std::vector<std::mt19937>& rngs)
{
    int canceled_mutant = 0;
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < (int)pop.size(); ++i) {

        math::set_current_rng(&rngs[i]);
        double currentRL = pop[i].exactRL;

        // 2. Попытка мутации, удаляем ровно K ребер и пытаемся жадно добавить
       
        Graph* mutated = pop[i].graph->mutateKNetworkGrowth(task.originalVerts, task.allowedEdges, params.MAX_BUDGET,params.K);

        if (!mutated) continue; // Мутация не удалась (например, нет допустимых ходов)

        // --- ПРОВЕРКА 1: Бюджет (Мгновенно) ---
        if (mutated->getTotalCost() > params.MAX_BUDGET) {
            delete mutated;
            continue;
        }

        CumBounds bounds;
        // если нашли разрез считаем по формуле декомпозиции
        if (have_2_VertCut)
        {
            bounds.RL = calcFullReliabilityDecomp(task, *mutated, cutVertices, cachedBaseRL, cachedMergedRL);
        }
        //Если не было разреза в задаче считаем через кумулятивные оценки надежность 
        else
        {
            // --- ПРОВЕРКА 2: Быстрая оценка надежности (Feasibility) ---
        // Если верхняя граница надежности мутанта хуже, чем текущая надежность родителя,
        // то нет смысла считать точно.

        /*
        #pragma omp critical
        {
            std::cout << "\nStart calculating bounds of reability with POROG for " << i << " individual AFTER mutating...";
        }
        */



            if (params.usePorog == true)
            {
                bounds = calcBounds(*mutated, currentRL);
            }
            else { bounds = calcBounds(*mutated, 0.0); }




            /*
            #pragma omp critical
            {
                std::cout << "\nFinish calculating bounds of reability with POROG for " << i << " individual AFTER mutating...";
            }
            */

           /* if (bounds.exact == false)
            {
                canceled_mutant += 1;
                SumOfCancel += 1;
            }
            */
            // Если даже в лучшем случае (RU) мутант хуже, чем родитель,то удаляем

            if (bounds.RU < currentRL) {
                delete mutated;
                /*
                #pragma omp critical
                {
                    std::cout << "\nNOT PASS CHECK: MUTANT " << i << " not good enough";
                }
                */
                continue;
            }
        }

        
        
        
      



        if (bounds.RL > currentRL) {
            // Успех - заменяем родителя (или можно использовать турнир)
            delete pop[i].graph;
            pop[i].graph = mutated;
            pop[i].exactRL = bounds.RL; //Кэшируем новое значение надежности мутанта


            //std::cout << " \n[MUT] Improved ind " << i << ": " << currentRL << " -> " << bounds.RL << " (Cost: " << pop[i].graph->getTotalCost() << ")" << std::endl;
        }
        else {
            delete mutated;
        }
    }

    std::cout << "Canceled mutants = " << canceled_mutant<<"\n";
}

// --- Основной цикл Скрещивания ---

void crossoverStep(std::vector<Individual>& pop, const NetworkTask& task, const SimulationParams& params, bool have_2_VertCut, double cachedBaseRL,
    double cachedMergedRL, std::pair<int, int> cutVertices, std::vector<std::mt19937>& rngs) {
    

    int P = (int)pop.size();
    std::vector<int> idx(P);
    for (int i = 0; i < P; ++i) idx[i] = i;
    std::shuffle(idx.begin(), idx.end(), math::global_main_rng);
    int canceled_child = 0;
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < P / 2; ++i) {

        math::set_current_rng(&rngs[i]);
        Individual& p1 = pop[idx[2 * i]];
        Individual& p2 = pop[idx[2 * i + 1]];

        // Скрещиваем
        Graph* child = crossoverGraphsV2(p1.graph, p2.graph,params.MAX_BUDGET);
        if (!child) continue;

        // --- ПРОВЕРКА 1: Бюджет (Мгновенно) ---
        if (child->getTotalCost() > params.MAX_BUDGET) {
            delete child; // Сразу убиваем, слишком дорогой
            continue;
        }

        // --- ПРОВЕРКА 2: Связность ---
        if (!child->isConnective()) {
            delete child;
            continue;
        }

        
        

        


        double worstP = std::min(p1.exactRL, p2.exactRL); // находим худшего из родителей
        CumBounds childBounds;
        if (have_2_VertCut)
        {
            childBounds.RL= calcFullReliabilityDecomp(task, *child, cutVertices, cachedBaseRL, cachedMergedRL);
        }

        else
        {
            /*
#pragma omp critical
        {
            std::cout << "\nStart calculating bound reability with POROG for CHILD of " << i << " pair parents";
        }
        */

            if (params.usePorog == true)
            {
                childBounds = calcBounds(*child, worstP); // в порог передаем худшего родителя так как ребенок должен быть лучше худшего из родителей
            }
            else { childBounds = calcBounds(*child, 0.0); }


           // if (childBounds.exact == 0) canceled_child += 1;

            /*
    #pragma omp critical
            {
                std::cout << "\nFinish calculating bound reability with POROG for CHILD of " << i << " pair parents";
            }
            */

            // --- ПРОВЕРКА 3: если верхняя оценка ребенка хуже худшего родителя, то удаляем ребенка

            if (childBounds.RU < worstP)
            {
                delete child;
                /*
                #pragma omp critical
                {
                    std::cout << "\nNOT PASS CHECK: CHILD " << i << " not good enough";
                }
                */
                continue;
            }
        }

   
        



        if (childBounds.RL > worstP) {

            {
                if (p1.exactRL< p2.exactRL) {
                    *p1.graph = *child; // Заменяем первого
                    p1.exactRL = childBounds.RL; //Обновляем кэш первого родителя
                }
                else {
                    *p2.graph = *child; // Заменяем второго
                    p2.exactRL = childBounds.RL;
                }
                //std::cout << "\n [CROSS] Child born: " << childBounds.RL << " (Cost: " << child->getTotalCost() << ")" << std::endl;
            }
        }
        delete child; // Мы скопировали данные в родителя через operator=, объект child удаляем
    }
    //std::cout << "Canceled childs = " << canceled_child<<"\n";
}


int main() 
{
    // 1. Загрузка параметров
    SimulationParams params;
    if (!loadSimulationParams("sim_params4.txt", params)) {
        std::cerr << "Error loading sim_params.txt! Using defaults." << std::endl;
    }

    if (params.THREADS > 0) omp_set_num_threads(params.THREADS);

    NetworkTask task;
    if (!loadNetworkData("network_data3.txt", task)) {
        std::cerr << "Error loading network_data.txt!" << std::endl;
        return 1;
    }

    // Создаём пул генераторов на каждую особь, чтобы был
    std::vector<std::mt19937> pop_rngs(params.POP);
    for (int i = 0; i < (int)params.POP; ++i) {
        pop_rngs[i].seed(1000 + i); // Каждая позиция в массиве имеет свой жесткий сид
    }

    std::cout << "=== NETWORK GROWTH OPTIMIZATION ===" << std::endl;
    std::cout << "Base Graph Verts: " << task.originalVerts << std::endl;
    std::cout << "New Verts to Add: " << task.newVertsToAdd << std::endl;
    std::cout << "Allowed Edges Pool: " << task.allowedEdges.size() << std::endl;
    std::cout << "Max Budget: " << params.MAX_BUDGET << std::endl;

    //Проверка что в задаче новые вершины присоединяются через 2-вершинный разрез(тогда считаем везде в задаче надежность через формулы декомпозиции)
    std::pair<int, int> cutVertices = check2VerCut(task);
    bool have_2_VertCut = (cutVertices.first != 0 && cutVertices.second != 0);
    double cachedBaseRL = 0.0;
    double cachedMergedRL = 0.0;
    if (have_2_VertCut) // если в задаче есть разер, то кэшируем вероятность начального графа, потом все передаем в функции мутации и кроссовера
    {
        std::cout << "2-Verticise cut found! Calculating base and merged base reliabilities..." << std::endl;
        auto reliabilities = calcBaseGraphReliabilities(task, cutVertices);
        cachedBaseRL = reliabilities.first;
        cachedMergedRL = reliabilities.second;
    }


    long long total_time_ms = 0;
    int NUM_RUNS = 10;   // КОЛИЧЕСТВО ПРОГОНОВ
    int unsuccessful = 0;
    for (int run = 0; run < NUM_RUNS; run++)
    {
        // 2. Инициализация популяции
        std::vector<Individual> population;
        population.reserve(params.POP);

        std::cout << "Generating initial population..." << std::endl;
        int attempts = 0;
        
        while (population.size() < params.POP && attempts < 10000) {
            Graph* g = generateConnectiveIndividual(task, params.MAX_BUDGET);
            //Graph* g = generateFeasibleIndividual(task, params.MAX_BUDGET); //это старое создание нач. популяции, не гарантирует связность 
            if (g)
            {
                //g->printAdjMatrix(); 
                
                population.push_back({ g, 0.0 }); // Сохраняем в кэш с заглушкой надежности
                
            }
            else  unsuccessful++;
            attempts++;
        }

        if (population.size() < params.POP) {
            std::cerr << "Failed to generate full population! (Budget too low or graph impossible)" << std::endl;
            // Можно выйти или работать с тем что есть
            if (population.empty()) return 1;
        }
        std::cout << "Population created: " << population.size() << std::endl;
        
        std::cout << "Calculating initial population reliabilities in parallel..." << std::endl;

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < (int)population.size(); ++i) {
            // Вызываем calcBounds в параллельных потоках
            double initialRL = 0.0;
            if (have_2_VertCut)
            {
                initialRL= calcFullReliabilityDecomp(task, *(population[i].graph), cutVertices, cachedBaseRL, cachedMergedRL);
            }
            else initialRL = calcBounds(*(population[i].graph), 0.0).RL;
            population[i].exactRL = initialRL;
        }

        std::cout << "Initial population evaluation complete." << std::endl;
//#pragma omp critical
        //omp_set_num_threads(1); // ПОСЧИТАЛИ НАЧАЛЬНУЮ ПОПУЛЯЦИЮ ПАРАЛЛЕЛЬНО ДАЛЬШЕ НА ОДНОМ ПОТОКЕ ДЛЯ ЧИСТОТЫ ЗАМЕРА ВЛИЯНИЕ ПОРОГА

       
        // 3. Основной цикл ГА
        int badGenerations = 0;
        double globalBestRL = 0.0;
        int SumOfCancelMutant = 0;
        auto start = std::chrono::steady_clock::now();// Начало времени отсчета основного цилка ГА(проверяем насколько ускоряет porog)
        int pokoleniy = 10; // -------- УКАЗЫВАЕТ КОЛИЧЕСТВО ПОКОЛЕНИЙ -------------

        for (int iter = 0; iter < pokoleniy; ++iter) { 
            if (badGenerations >= params.MAX_BAD) {
                std::cout << "Stopping criteria reached (MaxBad)." << std::endl;
                break;
            }



            std::cout << "\n--- Generation " << iter << " ---" << std::endl;

            // ВЫВОД КЭШИРОВАННЫХ ЗНАЧЕНИЙ НАДЕЖНОСТИ (До мутации/скрещивания)
            std::cout << "Current Population Reliabilities: [ ";
            for (const auto& ind : population) {
                std::cout << std::fixed << std::setprecision(8) << ind.exactRL << " ";
            }
            std::cout << "]" << std::endl;

            // A. Мутация
            mutationStep(population, task, params,SumOfCancelMutant,have_2_VertCut,cachedBaseRL,cachedMergedRL,cutVertices,pop_rngs);

            // B. Скрещивание
            crossoverStep(population, task, params,have_2_VertCut,cachedBaseRL, cachedMergedRL,cutVertices,pop_rngs);
           
        }

        auto end = std::chrono::steady_clock::now(); // Конец времени отсчета работы основного цикла ГА
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        total_time_ms += elapsed_ms.count();
        // 4. Вывод результата
        std::cout << "\n=== FINAL RESULT ===" << std::endl;

        // Найдем лучшую особь в финальной популяции

        Individual bestIndividual = { nullptr, -1.0 };

        for (const auto& ind : population) {
            if (ind.exactRL > bestIndividual.exactRL) {
                bestIndividual = ind;
            }
        }

        if (bestIndividual.graph) {

            std::cout << "exact Reliability: " << bestIndividual.exactRL << std::endl;
            std::cout << "Total Cost: " << bestIndividual.graph->getTotalCost() << std::endl;
            std::cout << "\nBest Graph Structure :" << std::endl;
            unsigned n = bestIndividual.graph->getVertNumb();
            int addedEdgesCount = 0;

            for (unsigned u = 1; u <= n; ++u) {
                for (unsigned v = u + 1; v <= n; ++v) {
                    // Смотрим, есть ли ребро в финальном лучшем графе
                    if (bestIndividual.graph->isEdge(u, v)) {

                        bool wasInBaseGraph = false;
                        // Проверяем, было ли это ребро в базовом графе
                        // (только если обе вершины принадлежат старому графу)
                        if (u <= task.originalVerts && v <= task.originalVerts) {
                            wasInBaseGraph = task.baseGraph->isEdge(u, v);
                        }

                        // Если ребра не было в базе — значит, оно добавлено алгоритмом
                        if (!wasInBaseGraph) {
                            std::cout << "Added Edge: " << u << " -- " << v << std::endl;
                            addedEdgesCount++;
                        }
                    }
                }
            }
            std::cout << "Total added edges in solution: " << addedEdgesCount << std::endl;
        }
        std::cout << "\nWorking time of GA: " << elapsed_ms.count() << " ms";

        std::cout << "\nAmount of cancelled mutants = " << 100.0*SumOfCancelMutant/(population.size()*pokoleniy);


        // Очистка
        for (auto& ind : population) delete ind.graph;
        
    }

    // 6. Вывод среднего времени
    double average_ms = static_cast<double>(total_time_ms) / NUM_RUNS;
    std::cout << "\nAverage GA time: " << average_ms << " ms" << std::endl;

    //std::cout << "Unsuccessful atempts of creating start Population(individuals) = " << unsuccessful;

    


    delete task.baseGraph;

    
    return 0;
}