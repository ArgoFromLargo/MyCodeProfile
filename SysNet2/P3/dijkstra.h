/**
 * File:   dijkstra.h
 *
 * dijkstra.h describes operations involving dijkstra's shortest path algorithm
 * on networking forwarding tables. This includes initializing adjacency matrices
 * (the data structure used for all operations), running dijkstra's algorithm,
 * finding adjacency tables, and calculating/displaying forwarding tables.
 *
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/22/2017
 * @info Course COP4635
 */

/**
 * Represent infinite cost to a router with the value 999.
 * Any values in the network that should not be accessed from certain routers
 * will be represented by a cost of 999.
 */
#define INF 999

/**
 * Allocates memory for an adjacency matrix (as a doubly-linked square matrix).
 * Each cell is an integer filled with a specified default value.
 *
 * @param size the width of the adjacency matrix
 * @param defaultFill the default value to fill in each cell
 * @return a pointer to the newly-allocated matrix
 */
int** allocAdjMatrix(int size, int defaultFill);

/**
 * Prints the given adjacency matrix to stdout for debugging.
 *
 * @param adjMatrix the adjacency matrix to display
 * @param size the number of rows (equivalently, columns) of the matrix
 */
void debugPrintMatrix(int** adjMatrix, int size);

/**
 * Frees the memory associated with the given array.
 *
 * @param adjMatrix the matrix to free
 * @param size the number of rows (equivalently, columns) of the matrix
 */
void freeAdjMatrix(int** adjMatrix, int size);

/**
 * Converts router label to an index. Ascii values map to 'A'+offset, where
 * the offset is returned. For example, 'A' -> 0, 'B' -> 1, 'C' -> 2, etc.
 * See indToLabel for the inverse function.
 *
 * @param label the label whose index to find
 * @return the index of the label
 */

int labelToInd(char label);

/**
 * Converts an index to a router label. Indices map to 'A' + index, where
 * the offset char is returned. For example, 0 -> 'A', 1 -> 'B' -> 1, 2 -> 'C', etc.
 * See labelToInd for the inverse function.
 *
 * @param index the index whose label to find
 * @return the label associated with the index
 */

char indToLabel(int index);

/**
 * Calculates the shortest path to all nodes from the desired startnode.
 * Based off of the shortest path for each node, Dijkstra will display where the
 * node will forward packets to depending on where the desired destination is.
 *
 * @param adjMatrix  The filled out adjacency matrix that will be used for
 *                   calculating the shortest paths to each node and where to
 *                   jump to first for each route.
 * @param numRouters The number of routers in the network. Running Dijkstra will
 *                   display the forwarding table for the numRouters - 1.
 * @param startnode  The desired node for which Dijkstra's algorithm will be run
 *                   on. This node will not be displayed in the forwarding table
 *                   since there is no need to forward data to oneself.
 */
void dijkstra(int** adjMatrix, int numRouters, int startnode);
