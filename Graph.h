#ifndef SRC_GRAPH_H
#define SRC_GRAPH_H

#include <fstream>
#include <vector> // ОБЯЗАТЕЛЬНО: нужно для std::vector
#include <tuple>  // ОБЯЗАТЕЛЬНО: нужно для std::tuple

#include "base_def.h"

namespace math {

    // Структура вынесена вверх, чтобы быть видимой классу
    struct PotentialEdge {
        u_int u;
        u_int v;
        double p;    // Вероятность (reliability)
        double cost; // Стоимость
    };

    class Graph {
        friend class GraphFactory;

        u_int* KAO;     // Массив сумм степеней вершин
        u_int* FO;      // Массив смежности
        double* FORel;  // Массив вероятностей ребер
        double* FOCost; // Массив стоимостей ребер

        u_int vertNumb; // Количество вершин
        u_int edgNumb;  // Количество ребер

        void memclear();
        void memcopy(const Graph& other);

#if __cplusplus >= 201103L
        void memmove(Graph&& other);
#endif

        void memallocNewGraph(u_int _vertNumb, u_int _edgNumb, u_int*& newKAO, u_int*& newFO, double*& newFORel, double*& newFOCost) const;
        void memupdate(u_int vertLim, u_int edgLim, u_int*& _KAO, u_int*& _FO, double*& _FORel, double*& _FOCost);
        void memrebase(u_int vertLim, u_int edgLim, u_int*& _KAO, u_int*& _FO, double*& _FORel, double*& _FOCost);

        bool meminited() const;
    public:
        bool isEdge(u_int u, u_int v) const;
        bool isEdgeWithPos(u_int u, u_int v, u_int& pos) const;

    public:
        Graph();

#if __cplusplus >= 201103L
        Graph(Graph&& other);
#endif

        Graph(const Graph& graph);
        Graph(u_int _vertNumb, u_int _edgNumb, u_int*& _KAO, u_int*& _FO, double*& _FORel, double*& _FOCost);
        ~Graph();

        void printAdjMatrix() const;
        static Graph* createEmptyGraph(u_int vertNumb, u_int edgNumb);
        static Graph* createFromEdges(u_int vertNumb, const std::vector<PotentialEdge>& edges);
        // Getters
        const u_int* getKAOPtr() const { return KAO; }
        const u_int* getFOPtr() const { return FO; }
        const double* getFORelPtr() const { return FORel; }
        const double* getFOCostPtr() const { return FOCost; }
        double* getFOCostPtr() { return FOCost; } // Non-const version

        double getEdgeCost(u_int pos) const { return FOCost[pos]; }

        // Подсчет полной стоимости графа (сумма всех FOCost / 2)
        double getTotalCost() const;

        u_int* getKAOPtr() { return KAO; }
        u_int* getFOPtr() { return FO; }
        double* getFORelPtr() { return FORel; }

        friend std::ifstream& operator >>(std::ifstream& input, Graph& graph);
        Graph& operator =(const Graph& graph);
        Graph& operator =(const Graph* graph);

#if __cplusplus >= 201103L
        Graph& operator =(Graph&& other);
#endif

       
        bool Graph::isStructureValid(const std::string& context = "") const;
        Graph* mergeEdge(u_int u, u_int v) const;
        Graph* cutEdge(u_int u, u_int v) const;
        void shootsDeletion(double& prob);

        // Возвращает true, если граф содержал параллельные ребра и был упрощен
        bool parallelEdgesTransformation();

        void parallelSeriesTransformation(double& pstFactor);

        bool isConnective() const;
        double baseProbabilities() const;

        u_int getVertNumb() const;
        u_int getEdgNumb() const;
        u_int deg(u_int i) const;

        // Возвращает {u, v, prob, cost}
        std::tuple<u_int, u_int, double, double> selectEdge() const;

        Graph* mutateOneEdge() const; // Старый метод (можно оставить)

        /**
        * Мутация для задачи расширения сети.
        */
        Graph* mutateNetworkGrowth(u_int originalVertCount, const std::vector<PotentialEdge>& allowedEdges, double maxCost) const;

        Graph* mutateKNetworkGrowth(u_int originalVertCount, const std::vector<PotentialEdge>& allowedEdges, double maxCost, unsigned K) const;
       
        
        Graph* mutateKEdges(u_int K) const; // Старый метод для K ребер

        Graph* addEdge(u_int u, u_int v, double rel = 0.75, double cost = 0.0) const;
    };
}

#endif