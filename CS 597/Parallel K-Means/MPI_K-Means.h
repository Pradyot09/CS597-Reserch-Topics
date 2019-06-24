/**
* Point structure for point data
*/
typedef struct 
{
	double _x;
	double _y;
} Point;

class  MPI_K_Means
{

public:
	/**
	* reader function of the points in the input file
	* This function must be called after  readHeaders(...) function
	* @param input input file handler
	* @param points pointer to return the array of points
	* @param num_points number of points to read
	*/
	void readHeaders(FILE *input,int* num_points);
	/**
	* reader function of the points in the input file
	* This function must be called after  readHeaders(...) function
	* @param input input file handler
	* @param points pointer to return the array of points
	* @param num_points number of points to read
	*/
	void readPoints(FILE* input,Point *points,int num_points);
	/**
	* initializer function that initializes the all cluster array values to -1
	* @param data pointer to return array of cluster data
	* @param num_points number of points to initialize
	*/
	void initialize(Point* centroids,int num_clusters);

	/**
	* initializer function that initializes the all cluster array values to -1
	* @param data pointer to return array of cluster data
	* @param num_points number of points to initialize
	*/
	int resetData(int *data,int num_points);

private:
	/**
	* calculate distance between two points
	* @param point1 first point
	* @param point2 second point
	* @return distance in double precision
	*/
	double calculateDistance(Point point1,Point point2);

	/**
	* Wierd name but essential function; decides witch centroid is closer to the given point
	* @param point point given
	* @param centroids pointer to centroids array
	* @param num_centroids number of centroids to check
	* @return closest centroid's index in centroids array(2nd param)
	*/
	int rearrangePoints(Point point,Point* centroids,int num_centroids);

	/**
	* Cumulative function that must be called after the closest centroid for each point is found
	* Calculates new centroids as describen in kmeans algorithm
	* @param points array of points
	* @param data array of cluster assignments
	* @param centroids return array of centroids
	* @param num_clusters number of clusters(so number of centroids)
	* @param num_points number of points 
	*/
	void calculateNewCentroids(Point* points,int* data,Point* centroids,int num_clusters,int num_points);
	/**
	* Convergence checker (see project description for further info)
	* @param former_clusters pointer to array of older cluster assignments
	* @param latter_clusters pointer to array of newer cluster assignments
	* @param num_points number of points 
	* @return -1 if not converged, 0 if converged.
	*/
	int checkConvergence(int *former_clusters,int *latter_clusters,int num_points);
};