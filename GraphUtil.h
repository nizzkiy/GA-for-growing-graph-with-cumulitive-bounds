#ifndef SRC_GRAPHUTIL_H
#define SRC_GRAPHUTIL_H

#include "base_def.h"
#include "Graph.h"
#include <utility> // for std::pair
#include "vector"


namespace math {

class Graph;

typedef enum {
    FACTORING,
    NO_REVERSE_REC_FACTORING,
   //NO_RECURSION_FACTORING
} FactoringType;

typedef enum {
    SERIAL
    //PARALLEL
} FactoringExecutionType;

    /// Graph type variants for generation
    enum GraphGenType {
        TYPE_COMPLETE     = 0,  // Complete graph
        TYPE_GRID         = 1,  // Grid
        TYPE_GRID_DIAGS   = 2,  // Grid with diagonals
        TYPE_RANDOM       = 3,  // Random connected graph
        TYPE_GEANT        = 4   // GEANT model
    };

    bool FeasibilityFactoringPST(Graph &G, double porog);

    Graph* runMutationCycle(Graph *G, double bestReliability, u_int k);

    Graph* crossoverGraphs(const Graph* parent1, const Graph* parent2, double MAX_BUDGET);

    Graph* crossoverGraphsV2(const Graph* parent1, const Graph* parent2, double maxBudget);

    // === POPULATION without mutations ==========================================
    std::vector<Graph*> createPopulation(u_int S,
                                         u_int N, u_int M,
                                         GraphGenType gtype);

    // === MUTATION CYCLES ===============================================
    Graph* mutateByFactoring   (Graph* G, u_int k, u_int maxBad = 40);

    double mutateByCumRL(Graph *G, double porog,
                         u_int k, u_int maxBad = 40);

    // === CUMULATIVE BOUNDS ============================================
    struct CumBounds 
    { 
        double RL, RU;
        bool exact;
    };
    CumBounds calcBounds(const Graph& G, double porog);


    /**
 * Creates a population of S graphs, performs mutation for each (K edges)
 * and selects the best by reliability.
 *
 * @param K     Number of edges for mutation
 * @param S     Number of graphs in population
 * @param N     Parameter for graph size/generation (depends on type)
 * @param M     Additional parameter (if needed, e.g. for grid)
 * @param gtype Graph type (see enum GraphGenType)
 * @return Pointer to the best graph (dynamically created!).
 *         Responsibility for delete lies with the calling code.
 */
    Graph* createPopulationAndSelectBest(u_int K, u_int S, u_int N, u_int M, GraphGenType gtype);


/**
 * Class-generator of various graphs
*/
class GraphGenerator {
    public:
        /**
         * Generates a complete graph on n vertices
         * @param n number of vertices
         * @return Pointer to the generated graph
        */
        static Graph *generateCompleteGraph(u_int n);


        /**
         * Generates n * m grid graph
         * @param n number of edges in length
         * @param m number of edges in width
         * @return Pointer to the generated graph
        */
        static Graph *generateGrid(u_int n, u_int m = 0);

        /**
         * Generates n * m grid graph, 
         * where 1 * 1 square blocks are connected by diagonal edges
         * @param n number of edges in length
         * @param m number of edges in width
         * @return Pointer to the generated graph
        */
        static Graph *generateGridWithDiags(u_int n, u_int m = 0);

        /**
         * Generates a random graph (in development)
         * @param vNumb number of vertices
         * @param eNumb number of edges
         * @return Pointer to the generated graph
        */
        static Graph *generateGraph(u_int vNumb, u_int eNumb);

        /**
         * Generates GEANT model graph
         * @return Pointer to the generated graph
        */
        static Graph *generateGEANT();
};

/**
 * Class for graph factoring
 * Can be extended by adding new factoring types to enum FactoringType 
 * and adding the corresponding static private method
*/
class GraphFactory {
    
    /**
     * Edge selection in graph for contraction/removal
     * @param graph graph of the current factoring step
     * @param prob probability of selected edge presence in the graph
     * @param u first vertex of the edge
     * @param v second vertex of the edge
     * @param uC auxiliary parameter
     * @param vC auxiliary parameter
    */
    static void choseVerts(Graph *&graph, double &p, u_int &u, u_int &v, u_int &uC, u_int &vC);

    /**
     * Factoring with probability passing deep into recursion
     * @param graph graph of the current factoring step
     * @param rel total probability
     * @param prob probability passed to the next recursion step. Product of series-parallel transformation probability, 
     * selected edge presence/absence probability in the graph, probability from the previous step)
    */
    static void branchingWithNoReverse(Graph *&graph, double &rel, double prob);

    /**
     * Ordinary branching with return when obtaining a small-dimension graph or connected graph
     * @param graph graph of the current factoring step
     * @return product of probabilities on the path to the obtained graph, multiplied by its probability
    */
    static double branching(Graph *&graph);
    
public:
    /**
     * Starting function for factoring with ability to specify factoring type
     * @param graph original graph
    */
    static double factoring(Graph *&graph, FactoringType factoringType = NO_REVERSE_REC_FACTORING);
};

#ifdef USE_MPI
    /**
     * Creates a population (S individuals) on different MPI processes, runs mutations,
     * then sends the best (by reliability) mutated graph to rank=0.
     * @param K - number of mutation iterations (passed to runMutationCycle)
     * @param S - population size
     * @param N - parameter for graph generation (number of vertices, etc.)
     * @param M - parameter for graph generation (number of edges, etc.)
     * @param gtype - generator type (TYPE_GRID, TYPE_RANDOM, etc.)
     * @return Pointer to the best mutated graph (only on rank=0, on other ranks will be nullptr)
     */
    std::pair<Graph*, double> createPopulationAndSelectBestMPI(u_int K, u_int S, u_int N, u_int M, GraphGenType gtype);
#endif
}



#endif