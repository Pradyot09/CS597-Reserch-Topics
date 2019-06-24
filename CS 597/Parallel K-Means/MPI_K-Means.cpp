#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#include "MPI_K-Means.h"

#define MASTER 0

void readHeaders(FILE *input,int* num_points)
{
	fscanf(input,"%d\n",num_points);
	printf("%d\n",*num_points);
}


void readPoints(FILE* input,Point *points,int num_points)
{
	int dex;
	for(dex=0;dex<num_points;dex++)
	{
		fscanf(input,"%lf,%lf",&points[dex]._x,&points[dex]._y);

	}
}

void initialize(Point* centroids,int num_clusters)
{
	int dex;
	srand(time(NULL));
	for(dex=0;dex<num_clusters;dex++)
	{
		centroids[dex]._x=((double)(rand()%1000))/1000;
		centroids[dex]._y=((double)(2*rand()%1000))/1000;
	}
}

int resetData(int *data,int num_points)
{
	int dex;
	for(dex=0;dex<num_points;dex++)
	{
		data[dex]=-1;
	}	
	return 0;	
}

double calculateDistance(Point point1,Point point2)
{
	return (pow((point1._x-point2._x)*100,2)+pow((point1._y-point2._y)*100,2));	
}

int rearrangePoints(Point point,Point* centroids,int num_centroids)
{
	int daddy=0;
	double distance=0;
	double minDistance=calculateDistance(point,centroids[0]);
	int dex;

	for(dex=1;dex<num_centroids;dex++)
	{	
		distance=calculateDistance(point,centroids[dex]);
		if(minDistance>=distance)
		{
			daddy=dex;
			minDistance=distance;
		}
	}
	return daddy;
}

void calculateNewCentroids(Point* points,int* data,Point* centroids,int num_clusters,int num_points)
{
	Point* newCentroids=(Point*)malloc(sizeof(Point)*num_clusters);
	int* population=(int*)malloc(sizeof(int)*num_clusters);
	int dex;

	for(dex=0;dex<num_clusters;dex++)
	{
		population[dex]=0;
		newCentroids[dex]._x=0;
		newCentroids[dex]._y=0;
	}	
	for(dex=0;dex<num_points;dex++)
	{
		population[data[dex]]++;
		newCentroids[data[dex]]._x+=points[dex]._x;
		newCentroids[data[dex]]._y+=points[dex]._y;
	}
	for(dex=0;dex<num_clusters;dex++)
	{
		if(population[dex]!=0.0)
		{
			newCentroids[dex]._x/=population[dex];
			newCentroids[dex]._y/=population[dex];
		}
	}
	for(dex=0;dex<num_clusters;dex++)
	{
		centroids[dex]._x=newCentroids[dex]._x;
		centroids[dex]._y=newCentroids[dex]._y;
	}	
}

int checkConvergence(int *former_clusters,int *latter_clusters,int num_points)
{
	int dex;
	for(dex=0;dex<num_points;dex++)
		if(former_clusters[dex]!=latter_clusters[dex])
			return -1;
	return 0;
}

int main(int argc, char* argv[])
{

	int rank;
	int size;
	int num_clusters;
	int num_points;
	int dex;
	int job_size;
	int rund_job_size;
	int job_done=0;
	Point* centroids;
	Point* points;
	Point* received_points;
	int  * slave_clusters;
	int  * former_clusters;
	int  * latter_clusters;

	MPI_Init(&argc, &argv);

	MPI_Status status;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	if (argc<4 )
	{
		printf("Enter the number of : \nClusters\nInput File\nOutput File\n");
		exit(0);
	}


	//creation of derived MPI structure
	MPI_Datatype MPI_POINT;
	MPI_Datatype type=MPI_DOUBLE;
	int blocklen=2;
	MPI_Aint disp=0;
	MPI_Type_create_struct(1,&blocklen,&disp,&type,&MPI_POINT);
	MPI_Type_commit(&MPI_POINT);

	/******************************MASTER PROCESSOR WORKS HERE******************************************************/ 
	if(size==1){
		printf("There cannot be one processes\n");
		exit(0);
	}

	if(rank==MASTER)
	{
		//inputting from file

		num_clusters=atoi(argv[1]);
		FILE *input;
		input=fopen(argv[2],"r");

		// readHeaders(input,&num_clusters,&num_points);
		readHeaders(input,&num_points);

		points=(Point*)malloc(sizeof(Point)*num_points);

		readPoints(input,points,num_points);

		fclose(input);

		//other needed memory locations
		former_clusters=(int*)malloc(sizeof(int)*num_points);
		latter_clusters=(int*)malloc(sizeof(int)*num_points);

		// remaining_points=num_points%(size-1);
		job_size=num_points/(size-1);
		printf("Job size :%d \n",job_size);
		printf("Size value Num vale %d, %d \n",size,num_points);


		// job_size=ceil(num_points/(size-1));
		rund_job_size=round(job_size);
		printf("Rounded Job size %d\n",rund_job_size);

		// remaing_size=rund_job_size+remaining_points;
		centroids=(Point*)malloc(sizeof(Point)*num_clusters);

		//reseting and initializing to default behaviour		
		initialize(centroids,num_clusters);
		resetData(former_clusters,num_points);
		resetData(latter_clusters,num_points);

		//Sending the essential data to slave processors
		for(dex=1;dex<size;dex++)
		{
			printf("Sending to [%d]\n",dex);
			MPI_Send(&rund_job_size              ,1           , MPI_INT             ,dex,0,MPI_COMM_WORLD);
			MPI_Send(&num_clusters               ,1           , MPI_INT             ,dex,0,MPI_COMM_WORLD);
			MPI_Send(centroids              ,num_clusters     , MPI_POINT           ,dex,0,MPI_COMM_WORLD);
			MPI_Send(points+((dex-1)*rund_job_size),rund_job_size    , MPI_POINT      ,dex,0,MPI_COMM_WORLD);
		}
		printf("Sent!\n");

		MPI_Barrier(MPI_COMM_WORLD);

		//Main job of master processor is done here		
		while(1)
		{	
			MPI_Barrier(MPI_COMM_WORLD);

			printf("Master Receiving\n");
			for(dex=1;dex<size;dex++)

				MPI_Recv(latter_clusters+(rund_job_size*(dex-1)),rund_job_size,MPI_INT,dex,0,MPI_COMM_WORLD,&status);

			printf("Master Received\n");

			calculateNewCentroids(points,latter_clusters,centroids,num_clusters,num_points);
			printf("New Centroids are done!\n");
			if(checkConvergence(latter_clusters,former_clusters,num_points)==0)
			{
				printf("Converged!\n");
				job_done=1;

			}
			else    
			{
				printf("Not converged!\n");
				for(dex=0;dex<num_points;dex++)
					former_clusters[dex]=latter_clusters[dex];
			}

			//Informing slaves that no more job to be done
			for(dex=1;dex<size;dex++)
				MPI_Send(&job_done,1, MPI_INT,dex,0,MPI_COMM_WORLD);

			MPI_Barrier(MPI_COMM_WORLD);
			if(job_done==1)
				break;

			//Sending the recently created centroids			
			for(dex=1;dex<size;dex++)
				MPI_Send(centroids,num_clusters, MPI_POINT,dex,0, MPI_COMM_WORLD);

			MPI_Barrier(MPI_COMM_WORLD);
		}

		//Outputting to the output file		
		FILE* output=fopen(argv[3],"w");
		fprintf(output,"%d\n",num_clusters);
		fprintf(output,"%d\n",num_points);
		for(dex=0;dex<num_clusters;dex++)
			fprintf(output,"%lf,%lf\n",centroids[dex]._x,centroids[dex]._y);
		for(dex=0;dex<num_points;dex++)
			fprintf(output,"%lf,%lf,%d\n",points[dex]._x,points[dex]._y,latter_clusters[dex]+1);
		fclose(output);
	}
	/***********************END OF MASTER PROCESSOR'S BRANCH ******************************/

	/******************** SLAVE PROCESSORS' JOB IS TO FOLLOW ************************/
	else
	{
		//Receiving the essential data

		printf("Receiving\n");
		MPI_Recv(&rund_job_size    ,1           ,MPI_INT  ,MASTER,0,MPI_COMM_WORLD,&status);
		MPI_Recv(&num_clusters,1           ,MPI_INT  ,MASTER,0,MPI_COMM_WORLD,&status);

		centroids=(Point*)malloc(sizeof(Point)*num_clusters);
		MPI_Recv(centroids    ,num_clusters,MPI_POINT,MASTER,0,MPI_COMM_WORLD,&status);
		printf("part_size =%d\n",rund_job_size);
		received_points=(Point*)malloc(sizeof(Point)*rund_job_size);
		slave_clusters=(int*)malloc(sizeof(int)*rund_job_size);

		MPI_Recv(received_points,rund_job_size,MPI_POINT      ,MASTER,0,MPI_COMM_WORLD,&status);

		printf("Received [%d]\n",rank);

		MPI_Barrier(MPI_COMM_WORLD);

		while(1)
		{
			printf("Calculation of new clusters [%d]\n",rank);
			for(dex=0;dex<rund_job_size;dex++)
			{
				slave_clusters[dex]=rearrangePoints(received_points[dex],centroids,num_clusters);
			}

			printf("sending to master [%d]\n",rank);
			MPI_Send(slave_clusters,rund_job_size, MPI_INT,MASTER, 0, MPI_COMM_WORLD);


			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Recv(&job_done,1, MPI_INT,MASTER,0,MPI_COMM_WORLD,&status);

			if(job_done==1) //No more work to be done
				break;


			//Receiving recently created centroids from master
			MPI_Recv(centroids,num_clusters,MPI_POINT,MASTER,0, MPI_COMM_WORLD,&status);
			MPI_Barrier(MPI_COMM_WORLD);


		}

	}

	//End of all	
	MPI_Barrier(MPI_COMM_WORLD);
	//MPI_Finalize();

	return 1;
}
/* EOF */