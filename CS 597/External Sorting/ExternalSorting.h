#include <bits/stdc++.h>
#include <string.h>

using namespace std;
 
struct MinHeapNode
{
    // The element to be stored
    int element;
 
    // index of the array from which the element is taken
    int i;
};
 
// Prototype of a utility function to swap two min heap nodes
void swap(MinHeapNode* x, MinHeapNode* y);
 
// A class for Min Heap
class MinHeap
{
    MinHeapNode* harr; // pointer to array of elements in heap
    int heap_size;     // size of min heap
 
public:
    // Constructor: creates a min heap of given size
    MinHeap(MinHeapNode a[], int size);
 
    // to heapify a subtree with root at given index
    void MinHeapify(int);
 
    // to get index of left child of node at index i
    int left(int i) { return (2 * i + 1); }
 
    // to get index of right child of node at index i
    int right(int i) { return (2 * i + 2); }
 
    // to get the root
    MinHeapNode getMin() {  return harr[0]; }
 
    // to replace root with new node x and heapify()
    // new root
    void replaceMin(MinHeapNode x)
    {
        harr[0] = x;
        MinHeapify(0);
    }
	
	// A utility function to swap two elements
	void swap(MinHeapNode* x, MinHeapNode* y);
	
	// Merges two subarrays of arr[].
	// First subarray is arr[l..m]
	// Second subarray is arr[m+1..r]
	void merge(int arr[], int l, int m, int r);
		
	// Merges k sorted files.  Names of files are assumed
	// to be 1, 2, 3, ... k
	void mergeFiles(char *output_file, int n, int k);
	
	// Using a merge-sort algorithm, create the initial runs
	// and divide them evenly among the output files
	void createInitialRuns(char *input_file, int run_size,
                       int num_ways);
	
	// For sorting data stored on disk
	void externalSort(char* input_file,  char *output_file,
                  int num_ways, int run_size);					   
};