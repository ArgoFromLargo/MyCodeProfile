/**
 * Systems and Networks II
 * Dijkstra's Algorithm for Router Networks
 * Luke Kledzik and Adam Mooers
 */

#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"

/*
int main(int argc, char** argv) {
	int i, j, numRouters;
	char routerLabel;
	// Get the number of routers and malloc an adjMatrix
	printf("\nEnter the number of routers: ");
	scanf("%d", &numRouters);
	int** adjMatrix = (int**)malloc(numRouters * sizeof(int*));
	for(i = 0; i < numRouters; i++) {
		adjMatrix[i] = (int*)malloc(numRouters * sizeof(int));
	}
	// Set values of the adjMatrix
	printf("\nEnter the adjacency matrix:\n");
	for(i=0;i < numRouters; i++) {
		for(j=0;j < numRouters; j++) {
			scanf("%d", &adjMatrix[i][j]);
		}
	}
	// Enter node on which to run Dijsktra's on
	printf("\nEnter the starting node: ");
	scanf("%c", &routerLabel);
	scanf("%c", &routerLabel);
	printf("\n");
	// Call Dijkstra's algorithm
	dijkstra(adjMatrix, numRouters, (int)(routerLabel - 'A'));
	for(i = 0; i < numRouters; i++) {
		free(adjMatrix[i]);
	}
	free(adjMatrix);
	return 0;
}
*/

int** allocAdjMatrix(int size, int defaultFill) {
	int i, j;
	int** adjMatrix = (int**)malloc(size * sizeof(int*)); // Allocate columns

	// Iterate through the array row-by-row
	for(i = 0; i < size; i++) {
		adjMatrix[i] = (int*)malloc(size * sizeof(int)); // Allocate row
		for(j = 0; j < size; j++) {
			adjMatrix[i][j] = defaultFill; // Fill cell
		}
	}

	return adjMatrix;
}

void debugPrintMatrix(int** adjMatrix, int size) {
	int i, j;

	// Iterate through the array row-by-row
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			// Print the current cell to stdout
			if (adjMatrix[i][j] == INF) {
				printf("INF");
			} else {
				printf("%d", adjMatrix[i][j]);
			}
			putchar('\t');
		}
		putchar('\n');
	}
}

void freeAdjMatrix(int** adjMatrix, int size) {
	int i;

	for(i = 0; i < size; i++) {
		free(adjMatrix[i]);
	}
	free(adjMatrix);
}

inline int labelToInd(char label)
{
	return (label - 'A');
}

inline char indToLabel(int index)
{
	return (index + 'A');
}

void dijkstra(int** adjMatrix, int numRouters, int startnode) {
	// Malloc arrays to help in Dijsktra's algorithm
	int i, j, count, minDist, nextnode, nextstep;
	int* distance = (int*)malloc(numRouters * sizeof(int));
	int* previous = (int*)malloc(numRouters * sizeof(int));
	int* visited = (int*)malloc(numRouters * sizeof(int));

	for(i = 0; i < numRouters; i++) {
		distance[i] = adjMatrix[startnode][i];
		previous[i] = startnode;
		visited[i] = 0;
	}
	distance[startnode] = 0;
	visited[startnode] = 1;
	count = 1;
	while(count < numRouters - 1) {
		// Initialize minDist to infinity
		minDist = INF;
		for(i = 0; i < numRouters; i++) {
			if(distance[i] < minDist && !visited[i]) {
				minDist = distance[i];
				nextnode = i;
			}
		}
		visited[nextnode] = 1;
		for(i = 0; i < numRouters; i++) {
			// If the node is not visited yet...
			if(!visited[i]) {
				// And the minDist + the cost to the nextnode < distance
				if(minDist + adjMatrix[nextnode][i] < distance[i]) {
					// Update the value of distance...
					distance[i] = minDist + adjMatrix[nextnode][i];
					// And now store nextnode in previous array
					previous[i] = nextnode;
				}
			}
		}
		count++;
	}
	// Calculation done, ready to print forwarding table...
	for(i = 0; i < numRouters; i++) {
		if(i != startnode) {
			printf("From Router #%c -> Dest. Router #%c......", (startnode + 'A'), (i + 'A'));
			j = i;
			do {
				nextstep = j;
				j = previous[j];
			} while(j != startnode);
			printf("Forward to #%c\n", (nextstep + 'A'));
		}
	}
	free(distance);
	free(previous);
	free(visited);
}
