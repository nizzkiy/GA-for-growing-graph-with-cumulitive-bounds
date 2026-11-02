#include <mimalloc-override.h>
#include "GraphUtil.h"
#include "Graph.h"
#include "random.h"
#include <tuple>
#include <iostream>
#include <cstdlib>  // rand(), srand()
#include <algorithm>  // std::shuffle
#include <random>
#include <ctime>
#include <set>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>


static bool g_initRand = false;

namespace math {

Graph *GraphGenerator::generateCompleteGraph(u_int n) {
    u_int gVertNumb = n;
    u_int gEdgNumb = n * (n - 1) / 2;
    u_int *gKAO = new u_int[gVertNumb + 1];
    u_int *gFO = new u_int[2 * gEdgNumb];
    double *gFORel = new double[2 * gEdgNumb];
    double* gFOCost = new double[2 * gEdgNumb]; // Добавлено

    u_int iC = 0;
    gKAO[0] = 0;
    for (u_int i = 1; i <= gVertNumb; i++) {
        gKAO[i] = gKAO[i - 1] + (n - 1);
    }
    for (u_int i = 1; i <= gVertNumb; i++) {
        for (u_int j = 1; j <= gVertNumb; j++) {
            if (i == j) {
                continue;
            }
            gFO[iC] = j;
            gFORel[iC] = 0.75;
            gFOCost[iC] = 0.0;
            iC++;
        }
    }

    Graph *result = new Graph(gVertNumb, gEdgNumb, gKAO, gFO, gFORel,gFOCost);
    return result;
}

Graph *GraphGenerator::generateGrid(u_int n, u_int m) {
    if (m == 0) {
        m = n;
    }

    u_int gVertNumb = (n + 1) * (m + 1);
    u_int gEdgNumb = n * (m + 1) + m * (n + 1);
    u_int *gKAO = new u_int[gVertNumb + 1];
    u_int *gFO = new u_int[2 * gEdgNumb];
    double *gFORel = new double[2 * gEdgNumb];
    double* gFOCost = new double[2 * gEdgNumb];

    for (u_int i = 0; i < (2 * gEdgNumb); i++) {
        gFORel[i] = 0.75;
        gFOCost[i] = 0.0;
    }

    u_int iC = 1;
    u_int jC = -1;
    gKAO[0] = 0;
    for (u_int k = 0; k < (m + 1); k++) {       // grid generation goes by horizontal layers from left to right
        if (k == 0) {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {                   // filling the top left node
                    gKAO[iC] = gKAO[iC - 1] + 2;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC  + (n + 1);                    
                }
                else if (i == n) {              // filling the top right node
                    gKAO[iC] = gKAO[iC - 1] + 2;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + (n + 1);
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + (n + 1);
                }
                iC++;
            }
        }
        else if (k == m) {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {                   // filling the bottom left node
                    gKAO[iC] = gKAO[iC - 1] + 2;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC + 1; 
                }
                else if (i == n) {              // filling the bottom right node
                    gKAO[iC] = gKAO[iC - 1] + 2;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                }
                iC++;
            }
        }
        else {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + (n + 1);
                }
                else if (i == n) {
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + (n + 1);
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 4;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + (n + 1);
                }
                iC++;
            }
        }
    }

    Graph *result = new Graph(gVertNumb, gEdgNumb, gKAO, gFO, gFORel,gFOCost);
    return result;
}

Graph *GraphGenerator::generateGridWithDiags(u_int n, u_int m) {
    if (m == 0) {
        m = n;
    }
    
    u_int gVertNumb = (n + 1) * (m + 1);
    u_int gEdgNumb = 4 * n * m + n + m;
    u_int *gKAO = new u_int[gVertNumb + 1];
    u_int *gFO = new u_int[2 * gEdgNumb];
    double *gFORel = new double[2 * gEdgNumb];
    double* gFOCost = new double[2 * gEdgNumb];

    for (u_int i = 0; i < (2 * gEdgNumb); i++) {
        gFORel[i] = 0.75;
        gFOCost[i] = 0.0;
    }

    u_int iC = 1;
    u_int jC = -1;
    gKAO[0] = 0;
    for (u_int k = 0; k < (m + 1); k++) {
        if (k == 0) {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {                   //filling the top left node
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + (n + 1);
                    gFO[++jC] = iC + (n + 2);
                }
                else if (i == n) {              //filling the top right node
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + n;
                    gFO[++jC] = iC + (n + 1);
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 5;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + n;
                    gFO[++jC] = iC + (n + 1);
                    gFO[++jC] = iC + (n + 2);
                }
                iC++;
            }
        }
        else if (k == m) {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {                   //filling the bottom left node
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - n;
                    gFO[++jC] = iC + 1;
                }
                else if (i == n) {              //filling the bottom right node
                    gKAO[iC] = gKAO[iC - 1] + 3;
                    gFO[++jC] = iC - (n + 2);
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 5;
                    gFO[++jC] = iC - (n + 2);
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - n;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                }
                iC++;
            }
        }
        else {
            for (u_int i = 0; i < (n + 1); i++) {
                if (i == 0) {
                    gKAO[iC] = gKAO[iC - 1] + 5;
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - n;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + (n + 1);
                    gFO[++jC] = iC + (n + 2); 
                }
                else if (i == n) {
                    gKAO[iC] = gKAO[iC - 1] + 5;
                    gFO[++jC] = iC - (n + 2);
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + n;
                    gFO[++jC] = iC + (n + 1); 
                }
                else {
                    gKAO[iC] = gKAO[iC - 1] + 8;
                    gFO[++jC] = iC - (n + 2);
                    gFO[++jC] = iC - (n + 1);
                    gFO[++jC] = iC - n;
                    gFO[++jC] = iC - 1;
                    gFO[++jC] = iC + 1;
                    gFO[++jC] = iC + n;
                    gFO[++jC] = iC + (n + 1);
                    gFO[++jC] = iC + (n + 2);
                }
                iC++;
            }
        }
    }

    Graph *result = new Graph(gVertNumb, gEdgNumb, gKAO, gFO, gFORel,gFOCost);
    return result;
}

Graph* GraphGenerator::generateGraph(u_int vNumb, u_int eNumb)
{
    // Protection against incorrect cases
    if (vNumb < 2) {
        // If at least 1 vertex is needed, it can be simplified, but in general:
        // Return empty graph or nullptr
        std::cout << "Vertices must be more than 2" << std::endl;
        return nullptr;
    }
    if (eNumb < vNumb - 1) {
        // Impossible to build a connected graph with number of edges less than N-1
        std::cout << "Impossible to build a connected graph with number of edges less than N - 1" << std::endl;
        return nullptr;
    }
    // Maximum edges without loops in undirected graph: vNumb*(vNumb-1)/2
    if (eNumb > (vNumb * (vNumb - 1) / 2)) {
        // Too many edges; will have to refuse
        std::cout << "Maximum edges without loops in undirected graph: vNumb*(vNumb-1)/2" << std::endl;
        return nullptr;
    }

    // Initialize random number generator (if not initialized elsewhere)
    static bool seeded = false;
    if (!seeded) {
        std::srand((unsigned)std::time(nullptr));
        seeded = true;
    }

    // --- 1) Build spanning tree from (N-1) edges ---
    //
    // Approach: start with list of visited vertices, where initially only vertex 1.
    // For each of the remaining vertices 2..N randomly choose an already visited vertex
    // and connect to it. This way we get a tree.

    // Store edges in structure (u, v), where u < v, to exclude duplicates.
    std::set<std::pair<u_int, u_int>> edgeSet;

    // Set of already visited vertices (in the original algorithm this is U)
    std::vector<u_int> visited;
    visited.reserve(vNumb);
    visited.push_back(1);

    // For i from 2..N add edge (i, random from visited)
    for (u_int node = 2; node <= vNumb; ++node) {
        // Random index from already visited
        u_int idx = std::rand() % visited.size();
        u_int existing = visited[idx];

        // Add edge (min, max), to avoid confusion between (u, v) and (v, u)
        u_int a = std::min(node, existing);
        u_int b = std::max(node, existing);
        edgeSet.insert({a, b});

        // Replenish the list of visited
        visited.push_back(node);
    }
    // At this stage we have N-1 edges forming a connected tree.

    // --- 2) Add the remaining M - (N-1) edges randomly ---
    //
    // Total we want eNumb edges, so we need to add:
    u_int edgesNeeded = eNumb - (vNumb - 1);

    // Until we add the required number
    while (edgesNeeded > 0) {
        // Randomly take a pair of vertices (u, v)
        u_int u = 1 + std::rand() % vNumb;
        u_int v = 1 + std::rand() % vNumb;
        if (u == v) {
            continue; // don't allow loops
        }
        if (u > v) {
            std::swap(u, v);
        }
        // Check if such edge already exists
        if (edgeSet.find({u, v}) == edgeSet.end()) {
            // Add new edge
            edgeSet.insert({u, v});
            edgesNeeded--;
        }
        // otherwise repeat until we find a new pair
    }

    // Now we have a complete list of M edges edgeSet
    // It remains to pack into KAO / FO / FORel format (as in other generators)

    // Step A. Count vertex degrees (ours are from 1..N)
    std::vector<u_int> degCount(vNumb + 1, 0);
    for (auto &e : edgeSet) {
        // e = (u, v)
        degCount[e.first]++;
        degCount[e.second]++;
    }

    // Step B. Build KAO array
    //   KAO[0] = 0
    //   KAO[i] = KAO[i-1] + deg(i)
    //   Total KAO[vNumb] = 2*M
    u_int *KAO = new u_int[vNumb + 1];
    KAO[0] = 0;
    for (u_int i = 1; i <= vNumb; i++) {
        KAO[i] = KAO[i - 1] + degCount[i];
    }

    // Step C. Build FO array (size 2*M) and FORel array (same size)
    u_int *FO = new u_int[2 * eNumb];
    double *FORel = new double[2 * eNumb];
    double *FOCost = new double[2 * eNumb];
    // Current offsets for writing neighbors
    std::vector<u_int> offsets(vNumb + 1, 0);
    for (u_int i = 1; i <= vNumb; ++i) {
        offsets[i] = KAO[i - 1]; // where in FO neighbors of vertex i will start
    }

    // Fill FO / FORel, iterating over the set of edges
    // For edge (u, v) add v to u's list, and u to v's list
    // For all — probability 0.75
    const double defaultProb = 0.75;
    for (auto &e : edgeSet) {
        u_int u = e.first;
        u_int v = e.second;
        // Entry for u
        FO[offsets[u]] = v;
        FORel[offsets[u]] = defaultProb;
        FOCost[offsets[u]] = 0.0;
        offsets[u]++;
        // Entry for v
        FO[offsets[v]] = u;
        FORel[offsets[v]] = defaultProb;
        FOCost[offsets[v]] = 0.0;
        offsets[v]++;
    }

    // Build the final Graph
    Graph* G = new Graph(vNumb, eNumb, KAO, FO, FORel,FOCost);
    return G;
}


void GraphFactory::choseVerts(Graph *&graph, double &p, u_int &u, u_int &v, u_int &uC, u_int &vC) {
    vC = graph->vertNumb;
    u_int deg = 2 * graph->vertNumb + 2;
    for (u_int i = (2 * graph->edgNumb - 1); i >= graph->KAO[1]; i--) {
        uC = graph->FO[i];
        if (i < graph->KAO[vC - 1]) {
            vC--;
        }
        if (vC > uC) {
            if (graph->KAO[uC] - graph->KAO[uC-1] + graph->KAO[vC] - graph->KAO[vC-1] < 7) {    
                u = uC;
                v = vC;
                p = graph->FORel[i];
                return;
            }
            else if (graph->KAO[uC] - graph->KAO[uC-1] + graph->KAO[vC] - graph->KAO[vC-1] < deg) {
                u = uC;
                v = vC;
                p = graph->FORel[i];
                deg = graph->KAO[uC] - graph->KAO[uC-1] + graph->KAO[vC] - graph->KAO[vC-1];
            }
        }
    }   
}

void GraphFactory::branchingWithNoReverse(Graph *&graph, double &rel, double prob) {
    u_int u, v, uC, vC;
    double pstFactor, p;

    graph->parallelSeriesTransformation(pstFactor);

    if (graph->getVertNumb() > 5) {
        choseVerts(graph, p, u, v, uC, vC);
        Graph *merge = graph->mergeEdge(u, v);
        Graph *cut = graph->cutEdge(u, v);
        if (cut->isConnective()) {
            branchingWithNoReverse(cut, rel, pstFactor * (1 - p) * prob);
        }
        branchingWithNoReverse(merge, rel, pstFactor * p * prob);
        delete merge;
        delete cut;
    }
    else {     
        rel += pstFactor * prob * graph->baseProbabilities();
        return;
    }
}

double GraphFactory::branching(Graph *&graph) {
    u_int u, v, uC, vC;
    double pstFactor, p, result;

    graph->parallelSeriesTransformation(pstFactor);

    if (graph->getVertNumb() > 5) {
        choseVerts(graph, p, u, v, uC, vC);
        Graph *merge = graph->mergeEdge(u, v);
        Graph *cut = graph->cutEdge(u, v);
        if (cut->isConnective()) {
            result = pstFactor * (p * branching(merge) + (1 - p) * branching(cut));
        }
        else {
            //delete cut;   so that unnecessary graphs don't occupy memory, you can move deletion of processed graphs to if-else block
            result = pstFactor * (p * branching(merge));
        }
        delete merge;
        delete cut;
        return result;
    }
    else {
        return pstFactor * graph->baseProbabilities();
    }
}

double GraphFactory::factoring(Graph *&graph, FactoringType factoringType) {
    double shootsProbability = 1.0;
    double result = 0.0;
    time_t start, end;
    time(&start);
    Graph* tempGraph = new Graph(*graph);
    tempGraph->shootsDeletion(shootsProbability);

    if (tempGraph->isConnective()) {
        if (factoringType == FACTORING) {
            result = shootsProbability * branching(tempGraph);
        }
        else if (factoringType == NO_REVERSE_REC_FACTORING) {
            branchingWithNoReverse(tempGraph, result, 1.0);
            result *= shootsProbability;
        }
    }

    time(&end);

    // std::cout << "Work time: " << (int)end - (int)start << std::endl;
    delete tempGraph;

    return result;
}

bool FeasibilityFactoringPST(Graph &G, double porog) {
    double RL = 0.0;   // Lower bound
    double RU = 1.0;   // Upper bound
    bool result = false;

    auto Branch = [&](Graph &G, double p, auto &&BranchRef) -> void {
        if (result) return;

        // Graph transformation: removal/simplification of series and parallel edges
        double p1 = 1.0;
        G.parallelSeriesTransformation(p1);
        RU -= p * (1 - p1);
//        std::cout << "RU " << RU << std::endl;
        if (RU <= porog) {
            result = true;
            return;
        }

        p1 *= p;

        // If graph is small (less than 7 vertices), calculate probability directly
        if (G.getVertNumb() < 6) {
            double y = G.baseProbabilities(); // Calculate probability
//            std::cout << "y " << y << std::endl;
            RL += p1 * y;
            // std::cout << "RL " << RL << std::endl;
            RU -= p1 * (1 - y);
            // std::cout << "RU " << RU << std::endl;
            if (RL >= porog || RU <= porog) {
                result = true;
            }
            return;
        }

        // Select edge for contraction or removal
        auto [u, v, pe, c] = G.selectEdge(); // Returns vertices u, v and probability pe
//        std::cout << "u " << u << std::endl;
//        std::cout << "v " << v << std::endl;
//        std::cout << "pe " << pe << std::endl;

        // Recursively process contraction and removal
        Graph G1 = *G.mergeEdge(u, v);
        Graph G2 = *G.cutEdge(u, v);

        if (G2.isConnective()) {
            BranchRef(G2, p1 * (1 - pe), BranchRef);
        }
        BranchRef(G1, p1 * pe, BranchRef);
    };

    Branch(G, 1.0, Branch);
    std::cout << "RL after recursion " << RL << std::endl;
	std::cout << "RU after recursion " << RU << std::endl;
    return RL >= porog;
}

#ifdef USE_MPI
#include <mpi.h>
#endif

Graph* runMutationCycle(Graph *G, double bestReliability, u_int k)
{
#ifdef USE_MPI
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
    int rank = 0;
#endif

    if (!g_initRand) {
        // Now seed depends on rank and current time
        srand((unsigned)(time(nullptr) + rank * 1000));
        g_initRand = true;
    }


    // Calculate initial reliability
    // double bestReliability = GraphFactory::factoring(G, FactoringType::NO_REVERSE_REC_FACTORING);
    // std::cout << "Initial reliability = " << bestReliability << std::endl;

    int consecutiveNotImprovements = 0;
    const int neededNotImprovements = 40;

    while (consecutiveNotImprovements < neededNotImprovements)
    {

        Graph *candidate = G->mutateKEdges(k);
        if (!candidate) {
            std::cerr << "Mutation failed (null candidate). Stopping...\n";
            break;
        }

        double candReliability = GraphFactory::factoring(candidate, FactoringType::NO_REVERSE_REC_FACTORING);
        if (candReliability > bestReliability)
        {
            // Accepted new individual
            *G = *candidate;
            bestReliability = candReliability;
            // std::cout << "[IMPROVED] new reliability = " << bestReliability
                      // << " (improvements in a row = " << consecutiveNotImprovements << ")\n";
        }
        else
        {
            // Didn't improve — don't accept
//            std::cout << "[NO better] reliability = " << candReliability << std::endl;
            consecutiveNotImprovements++;
            // Counter of consecutive improvements can be reset if desired
            // consecutiveImprovements = 0; // <-- depends on requirement
        }
        delete candidate;
    }

    std::cout << "Mutation finished. Final reliability = " << bestReliability << std::endl;

    return G;
}

Graph* crossoverGraphs(const Graph* parent1, const Graph* parent2, double maxBudget) {
    if (!parent1 || !parent2) return nullptr;

    u_int vertNumb = std::max(parent1->getVertNumb(), parent2->getVertNumb());

    // 1. Собираем все ребра из обоих родителей в один плоский список
    std::vector<PotentialEdge> allEdges;

    auto collect = [&](const Graph* g) {
        for (u_int u = 1; u <= g->getVertNumb(); ++u) {
            for (u_int j = g->getKAOPtr()[u - 1]; j < g->getKAOPtr()[u]; ++j) {
                u_int v = g->getFOPtr()[j];
                if (u < v) { // Чтобы не дублировать (u,v) и (v,u)
                    allEdges.push_back({ u, v, g->getFORelPtr()[j], g->getFOCostPtr()[j] });
                }
            }
        }
    };
    collect(parent1);
    collect(parent2);

    // 2. Сортируем, чтобы найти общие ребра (те, что дублируются в списке)
    std::sort(allEdges.begin(), allEdges.end(), [](const PotentialEdge& a, const PotentialEdge& b) {
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
        });

    std::vector<PotentialEdge> common;
    std::vector<PotentialEdge> unique;

    if (!allEdges.empty()) {
        for (size_t i = 0; i < allEdges.size(); ++i) {
            if (i + 1 < allEdges.size() && allEdges[i].u == allEdges[i + 1].u && allEdges[i].v == allEdges[i + 1].v) {
                common.push_back(allEdges[i]);
                i++; // Пропускаем дубликат
            }
            else {
                unique.push_back(allEdges[i]);
            }
        }
    }

    // 3. Создаем ребенка. Начинаем с абсолютно пустого графа (0 ребер)
    // Используем ваш метод, передавая 0 ребер в начале
    Graph* child = Graph::createEmptyGraph(vertNumb, 0);
    // Инициализируем KAO нулями, иначе addEdge упадет
    for (u_int i = 0; i <= vertNumb; ++i) child->getKAOPtr()[i] = 0;

    double currentCost = 0;

    // Сначала добавляем ОБЩИЕ ребра (наследуем лучшее)
    for (const auto& e : common) {
        if (currentCost + e.cost <= maxBudget) {
            Graph* next = child->addEdge(e.u, e.v, e.p, e.cost);
            if (next) {
                delete child;
                child = next;
                currentCost += e.cost;
            }
        }
    }

    // Перемешиваем уникальные для разнообразия популяции
    std::shuffle(unique.begin(), unique.end(), std::mt19937(std::random_device()()));

    // Добавляем УНИКАЛЬНЫЕ, пока хватает бюджета
    for (const auto& e : unique) {
        if (currentCost + e.cost <= maxBudget) {
            Graph* next = child->addEdge(e.u, e.v, e.p, e.cost);
            if (next) {
                delete child;
                child = next;
                currentCost += e.cost;
            }
        }
    }

    // 4. Финальная проверка: если в погоне за бюджетом мы потеряли связность - кроссовер неудачен
    if (!child->isConnective()) {
        delete child;
        return nullptr;
    }

    return child;
}

Graph* crossoverGraphsV2(const Graph* parent1, const Graph* parent2, double maxBudget) {
    if (!parent1 || !parent2) return nullptr;

    u_int vertNumb = std::max(parent1->getVertNumb(), parent2->getVertNumb());

    // 1. Собираем все ребра из обоих родителей
    std::vector<PotentialEdge> allEdges;
    auto collect = [&](const Graph* g) {
        for (u_int u = 1; u <= g->getVertNumb(); ++u) {
            for (u_int j = g->getKAOPtr()[u - 1]; j < g->getKAOPtr()[u]; ++j) {
                u_int v = g->getFOPtr()[j];
                if (u < v) {
                    allEdges.push_back({ u, v, g->getFORelPtr()[j], g->getFOCostPtr()[j] });
                }
            }
        }
    };
    collect(parent1);
    collect(parent2);

    // 2. Сортируем и разделяем на общие и уникальные
    std::sort(allEdges.begin(), allEdges.end(), [](const PotentialEdge& a, const PotentialEdge& b) {
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
        });

    std::vector<PotentialEdge> common;
    std::vector<PotentialEdge> unique;

    if (!allEdges.empty()) {
        for (size_t i = 0; i < allEdges.size(); ++i) {
            if (i + 1 < allEdges.size() && allEdges[i].u == allEdges[i + 1].u && allEdges[i].v == allEdges[i + 1].v) {
                common.push_back(allEdges[i]);
                i++; // Пропускаем дубликат
            }
            else {
                unique.push_back(allEdges[i]);
            }
        }
    }

    // 3. Формируем список ребер для ребенка
    std::vector<PotentialEdge> childEdges;
    double currentCost = 0;

    // Сначала забираем все общие ребра (каркас)
    for (const auto& e : common) {
        if (currentCost + e.cost <= maxBudget) {
            childEdges.push_back(e);
            currentCost += e.cost;
        }
    }

    // Перемешиваем уникальные для разнообразия
    
    std::shuffle(unique.begin(), unique.end(), math::get_random_generator());

    // Добиваем бюджет уникальными ребрами
    for (const auto& e : unique) {
        if (currentCost + e.cost <= maxBudget) {
            childEdges.push_back(e);
            currentCost += e.cost;
        }
    }

    // 4. Создаем граф ОДНИМ вызовом!
    Graph* child = Graph::createFromEdges(vertNumb, childEdges);

    // 5. Финальная проверка на связность
    if (!child->isConnective()) {
        delete child;
        return nullptr;
    }

    return child;
}

    /// Helper function: generates graph by the appropriate method based on gtype
static Graph* createGraphByType(GraphGenType gtype, u_int N, u_int M)
{
    switch (gtype) {
        case TYPE_COMPLETE:
            return GraphGenerator::generateCompleteGraph(N);
        case TYPE_GRID:
            return GraphGenerator::generateGrid(N, M);
        case TYPE_GRID_DIAGS:
            return GraphGenerator::generateGridWithDiags(N, M);
        case TYPE_RANDOM:
            // For RANDOM can be interpreted as:
            //  N = number of vertices,  M = number of edges
            return GraphGenerator::generateGraph(N, M);
        default:
            // If something is wrong, return complete graph
            return GraphGenerator::generateCompleteGraph(N);
    }
}

struct IndividualResult {
    double reliability;
    Graph* graph;  // can store copy or pointer itself,
                   // depending on what's more convenient for us
    u_int index;   // individual index, if needed
};

// Structure describes a "task" that the thread will process
// In our case — it's just the individual index i.
struct Task {
    u_int index;
};

// Global or static objects (within the example — illustration,
// in production this is organized more carefully):
static std::mutex queueMutex;
static std::condition_variable queueCond;
static bool done = false;  // flag that there will be no more tasks
static std::queue<Task> tasks; // task queue

// Array (or vector) for results
static std::vector<IndividualResult> results;

// Number of threads (3)
static const int NUM_THREADS = 6;

// Parameters that are the same for all individuals,
// so threads can use them:
static u_int g_K, g_N, g_M, g_S;
static GraphGenType g_type;

//////////////////////////////////////////
// Function that each thread will call
//////////////////////////////////////////
void workerThread()
{
    while (true)
    {
        // 1) Take task from queue
        Task currentTask;
        {
            // Lock mutex, wait until task appears or completion signal
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, []{
                return !tasks.empty() || done;
            });

            // If all tasks are processed (done) and queue is empty — exit the loop
            if (done && tasks.empty()) {
                return;
            }

            // Take the task
            currentTask = tasks.front();
            tasks.pop();
        }

        // 2) Execute task: generate individual, mutate, calculate reliability
        //    NB: For clarity, here code "in the raw"; in practice, it can be moved
        //    to a separate function, as we did before.

        // Generate next graph by current index (index is not very important,
        // but can be used for seed, if desired, or simply ignored)
        Graph* g = nullptr;
        switch (g_type) {
            case TYPE_COMPLETE:
                g = GraphGenerator::generateCompleteGraph(g_N);
                break;
            case TYPE_GRID:
                g = GraphGenerator::generateGrid(g_N, g_M);
                break;
            case TYPE_GRID_DIAGS:
                g = GraphGenerator::generateGridWithDiags(g_N, g_M);
                break;
            case TYPE_RANDOM:
                g = GraphGenerator::generateGraph(g_N, g_M);
                break;
            default:
                g = GraphGenerator::generateCompleteGraph(g_N);
        }

        if (!g) {
            // In case generate... returns nullptr
            // Write result with reliability = -1 or something like that
            IndividualResult badRes;
            badRes.reliability = -1.0;
            badRes.graph = nullptr;
            badRes.index = currentTask.index;
            // Write to results
            {
                std::lock_guard<std::mutex> lk(queueMutex);
                results[currentTask.index] = badRes;
            }
            continue;
        }
        // Calculate initial reliability
        double initReli = GraphFactory::factoring(g, NO_REVERSE_REC_FACTORING);

        // Start mutation cycle
        Graph* mutated = runMutationCycle(g, initReli, g_K);

        // Final reliability
        double finalReli = GraphFactory::factoring(mutated, NO_REVERSE_REC_FACTORING);

        // Save result
        IndividualResult indRes;
        indRes.reliability = finalReli;
        // Important: if we want to compare graphs later —
        //  we can save a copy:
        indRes.graph = new Graph(*mutated);
        // or we can save a pointer, but then we need to manage lifespan
        indRes.index = currentTask.index;

        // Write to results by index
        {
            std::lock_guard<std::mutex> lk(queueMutex);
            results[currentTask.index] = indRes;
        }

        // Free original
        delete mutated;
    }
}

///////////////////////////////////////////////////
// Function createPopulationAndSelectBest, parallelized
///////////////////////////////////////////////////
Graph* createPopulationAndSelectBest(u_int K, u_int S, u_int N, u_int M, GraphGenType gtype)
{
    // Initialize global variables
    g_K = K;
    g_S = S;
    g_N = N;
    g_M = M;
    g_type = gtype;

    // Initialize results vector of the appropriate size
    results.resize(S);

    // Fill task queue
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        done = false;
        while (!tasks.empty()) {
            tasks.pop();
        }
        for (u_int i = 0; i < S; i++) {
            Task t;
            t.index = i;
            tasks.push(t);
        }
    }

    // Start NUM_THREADS threads
    std::vector<std::thread> workers;
    workers.reserve(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        workers.emplace_back(workerThread);
    }

    // Wake up threads so they can start taking tasks
    queueCond.notify_all();

    // Wait until all tasks are processed
    {
        // Simplest option: just wait until queue empties and indicate done = true, so threads exit.
        std::unique_lock<std::mutex> lock(queueMutex);
        // As long as queue is not empty, wait a bit
        // (or you can check the number of completed tasks, this is implementation details)
        while (!tasks.empty()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
        }
        done = true;
    }
    // Signal threads that done=true
    queueCond.notify_all();

    // Wait for all worker threads to finish
    for (auto &th : workers) {
        if (th.joinable()) {
            th.join();
        }
    }

    // Now results contains S results.
    // Find maximum reliability
    double bestReli = -1.0;
    int bestIndex = -1;
    for (int i = 0; i < (int)S; i++) {
        double r = results[i].reliability;
        if (r > bestReli) {
            bestReli = r;
            bestIndex = i;
        }
    }

    if (bestIndex < 0) {
        // Means nothing was really achieved (e.g., all nullptr).
        return nullptr;
    }

    // Take best graph
    Graph* bestGraph = results[bestIndex].graph;

    // Other graphs, stored in results (except bestIndex),
    // need to be carefully deleted to avoid leaks
    // (we did new Graph(*mutated)).
    for (int i = 0; i < (int)S; i++) {
        if (i != bestIndex) {
            if (results[i].graph) {
                delete results[i].graph;
                results[i].graph = nullptr;
            }
        }
    }

    return bestGraph;
}
    ///////////////////////////////////////////////////////////////////////
//  createPopulation – simply S graphs, without mutations
///////////////////////////////////////////////////////////////////////
std::vector<Graph*> createPopulation(u_int S,
                                     u_int N, u_int M,
                                     GraphGenType gtype)
{
    std::vector<Graph*> pop;
    pop.reserve(S);
    for (u_int i = 0; i < S; ++i) {
        pop.push_back( createGraphByType(gtype, N, M) );
    }
    return pop;           // by value!
}

///////////////////////////////////////////////////////////////////////
// 1) Mutation with factor-probability criterion
///////////////////////////////////////////////////////////////////////
Graph* mutateByFactoring(Graph* G, u_int k, u_int maxBad)
{
    double best = GraphFactory::factoring(G);
    int bad = 0;
    while (bad < (int)maxBad) {
        std::unique_ptr<Graph> cand{ G->mutateKEdges(k) };
        if (!cand) break;
        Graph* candPtr = cand.get();
        time_t start, end;
        time(&start);
        double r = GraphFactory::factoring(candPtr);
        time(&end);
        // std::cout << "Work time FACT: " << (int)end - (int)start << std::endl;
        if (r > best) {
            *G = *cand;
            best = r;
            bad = 0;
        }
        else {
            ++bad;
        }
    }
    return G;
}

///////////////////////////////////////////////////////////////////////
// 2) Cumulative estimates (RL / RU)
///////////////////////////////////////////////////////////////////////
CumBounds calcBounds(const Graph& G, double porog) {
    double RL = 0.0;
    double RU = 1.0;
    bool stop = false;
    bool exact = true; // изначально считаем что идем до конца

    // Используем std::function или передаем лямбду через auto&&
    auto Branch = [&](const Graph& H, double p, auto&& BranchRef) -> void {
        if (stop) return;

        // Делаем локальную копию для трансформаций, так как H — const
        Graph localH(H);

        // =========================================================
    // 0. ОЧИСТКА ОТ ПАРАЛЛЕЛЬНЫХ РЕБЕР
    // =========================================================
    // Если граф пришел после стягивания (mergeEdge), в нем 100% есть дубликаты.
    // Схлопываем их. Выносить вероятность никуда не нужно!
        localH.parallelEdgesTransformation();

/*
        // ДОБАВЛЯЕМ ВАЛИДАЦИЮ ПЕРЕД ТРАНСФОРМАЦИЕЙ
        if (!localH.isStructureValid("before PST")) {
            std::cerr << "FATAL: Graph structure corrupted before PST\n";
            stop = true;
            return;
        }
*/

// =========================================================
// 1. УДАЛЕНИЕ ВИСЯЧИХ ВЕРШИН (Shoots Deletion)
// =========================================================
        double shootProb = 1.0;
        localH.shootsDeletion(shootProb);

        // Корректируем RU: отсечение листьев уменьшает максимальную оценку
        RU -= p * (1.0 - shootProb);
        if (RU <= porog) {
            stop = true;
            exact = false;
            return;
        }

        // Обновляем текущую вероятность ветви
        p *= shootProb;

        double pst = 1.0;
        localH.parallelSeriesTransformation(pst);
        /*
        //  ДОБАВЛЯЕМ ВАЛИДАЦИЮ ПОСЛЕ ТРАНСФОРМАЦИИ
        if (!localH.isStructureValid("before PST")) {
            std::cerr << "FATAL: Graph structure corrupted after PST\n";
            stop = true;
            return;
        }
        */


        // Корректируем RU: если PST схлопнул ребра с надежностью < 1
        RU -= p * (1.0 - pst);
        if (RU <= porog) 
        { 
            stop = true;
            exact = false;
            return; 
        }

        p *= pst;

        if (localH.getVertNumb() <= 5) {
            double base = localH.baseProbabilities();
            RL += p * base;
            RU -= p * (1.0 - base);
            if (RU <= porog)
            {
                stop = true;
                exact = false;
            }
            return;
        }

        auto [u, v, pe, c] = localH.selectEdge();

        // Ветка Cut (удаление ребра)
        Graph* cutted = localH.cutEdge(u, v);
        if (cutted) {
            if (cutted->isConnective()) {
                BranchRef(*cutted, p * (1.0 - pe), BranchRef);
            }
            else {
                // ВАЖНО: Если граф распался, эта ветка больше не дает надежности
                RU -= p * (1.0 - pe);
                if (RU <= porog) {
                    stop = true;
                    exact = false;
                }
            }
            delete cutted;
        }

        // Ветка Merge (стягивание ребра)
        Graph* merged = localH.mergeEdge(u, v);
        if (merged) {
            BranchRef(*merged, p * pe, BranchRef);
            delete merged;
        }

        
        
    };

        

    Graph initialTemp(G);
    Branch(initialTemp, 1.0, Branch);

    return { RL, RU,exact };
}

    double mutateByCumRL(Graph* G, double porog, u_int k, u_int maxBad)
{
    int bad = 0;
    double lastRL = 0.0;

    while (bad < (int)maxBad) {
        Graph *cand = G->mutateKEdges(k);  // manual allocation
        if (!cand) {
            break;
        }

        time_t start, end;
        time(&start);
        auto bounds = calcBounds(*cand, porog);
        time(&end);

        // std::cout << "Work time CUM: " << (int)(end - start) << std::endl;

        if (bounds.RL > porog) {
            // std::cout << "RL: " << bounds.RL << std::endl;
            porog = bounds.RL;     // new threshold
            *G = *cand;            // copy graph
            lastRL = bounds.RL;
            bad = 0;
        } else {
            ++bad;
        }
        delete cand;
    }

    return lastRL;
}


}

#ifdef USE_MPI
#include <mpi.h>
#endif
#include <algorithm>  // std::min

namespace math {

#ifdef USE_MPI

std::pair<Graph*, double> createPopulationAndSelectBestMPI(u_int K, u_int S, u_int N, u_int M, GraphGenType gtype)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct MetaInfo {
        double reliability;
        double initialReliability; // Add initial reliability
        int    index;
        u_int  vN;
        u_int  eN;
    };

    if (rank == 0) {
        double bestReli = -1.0;
        double bestInitialReli = -1.0;
        u_int bestVN = 0;
        u_int bestEN = 0;
        std::vector<u_int> bestKAO;
        std::vector<u_int> bestFO;
        std::vector<double> bestFORel;

        for (int iProc = 1; iProc < std::min((int)S+1, size); ++iProc) {
            int task = iProc - 1;
            MPI_Send(&task, 1, MPI_INT, iProc, 0, MPI_COMM_WORLD);
        }

        int nextTask = size - 1;
        int tasksDone = 0;

        while (tasksDone < (int)S) {
            MetaInfo meta;
            MPI_Status status;
            MPI_Recv(&meta, sizeof(MetaInfo), MPI_CHAR, MPI_ANY_SOURCE, 0,
                     MPI_COMM_WORLD, &status);

            std::vector<u_int> KAO(meta.vN + 1);
            std::vector<u_int> FO(2 * meta.eN);
            std::vector<double> FORel(2 * meta.eN);

            if (meta.vN > 0 && meta.eN > 0) {
                MPI_Recv(KAO.data(),   meta.vN+1, MPI_UNSIGNED, status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(FO.data(),    2*meta.eN, MPI_UNSIGNED, status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(FORel.data(), 2*meta.eN, MPI_DOUBLE,   status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            if (meta.reliability > bestReli) {
                bestReli = meta.reliability;
                bestInitialReli = meta.initialReliability; // save initial reliability
                bestVN = meta.vN;
                bestEN = meta.eN;
                bestKAO   = std::move(KAO);
                bestFO    = std::move(FO);
                bestFORel = std::move(FORel);
            }

            tasksDone++;

            if (nextTask < (int)S) {
                MPI_Send(&nextTask, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                nextTask++;
            } else {
                int stop = -1;
                MPI_Send(&stop, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            }
        }

        if (bestVN == 0) {
            return {nullptr, -1.0};
        }

        u_int *kaoPtr   = new u_int[bestVN + 1];
        u_int *foPtr    = new u_int[2 * bestEN];
        double *foRelPtr= new double[2 * bestEN];

        std::copy(bestKAO.begin(),   bestKAO.end(),   kaoPtr);
        std::copy(bestFO.begin(),    bestFO.end(),    foPtr);
        std::copy(bestFORel.begin(), bestFORel.end(), foRelPtr);

        Graph* bestGraph = new Graph(bestVN, bestEN, kaoPtr, foPtr, foRelPtr);

        return {bestGraph, bestInitialReli};  // return pair (graph, initial reliability)
    }
    else {
        while (true) {
            int taskIndex;
            MPI_Recv(&taskIndex, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (taskIndex == -1) {
                break;
            }

            Graph* g = createGraphByType(gtype, N, M);
            if (!g) {
                MetaInfo meta {-1.0, -1.0, taskIndex, 0, 0};
                MPI_Send(&meta, sizeof(MetaInfo), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                continue;
            }

            double initReli = GraphFactory::factoring(g, NO_REVERSE_REC_FACTORING);
            Graph* mutated = runMutationCycle(g, initReli, K);

            double finalReli = -1.0;
            if (mutated) {
                finalReli = GraphFactory::factoring(mutated, NO_REVERSE_REC_FACTORING);
            }

            MetaInfo meta;
            meta.reliability = finalReli;
            meta.initialReliability = initReli;  // send initial reliability
            meta.index       = taskIndex;
            if (mutated) {
                meta.vN = mutated->getVertNumb();
                meta.eN = mutated->getEdgNumb();
            } else {
                meta.vN = 0;
                meta.eN = 0;
            }

            MPI_Send(&meta, sizeof(MetaInfo), MPI_CHAR, 0, 0, MPI_COMM_WORLD);

            if (mutated && meta.vN > 0 && meta.eN > 0) {
                const u_int* kaoPtr = mutated->getKAOPtr();
                const u_int* foPtr  = mutated->getFOPtr();
                const double* relPtr = mutated->getFORelPtr();

                MPI_Send(kaoPtr,  meta.vN+1,  MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
                MPI_Send(foPtr,   2*meta.eN, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
                MPI_Send(relPtr,  2*meta.eN, MPI_DOUBLE,   0, 0, MPI_COMM_WORLD);
            }

            if (mutated) delete mutated;
        }
        return {nullptr, -1.0};
    }
}

#endif // USE_MPI

}
 // namespace math

