#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "mpi.h"






bool matches(char sentence[],char query [])
{
    char tmp[strlen(query)];
    strcpy(tmp, query);
    int i;
    int temp=0;
    char* token = strtok(tmp, " ");
    int parts = 0, counter = 0;
    while(token != NULL)
    {
        parts++;
        temp = 0;
        for (i=0 ; i< strlen(sentence) ; i++)
        {
            if(sentence[i]==token[temp])
            {
                temp++;
            }
            else
            {
                temp=0;
            }
            if (temp==(strlen(token)))
            {
                if(i != strlen(sentence))
                {

                    if(sentence[i+1] < 65 || sentence[i+1] > 122 || sentence[i+1] == 115)
                    {

                        counter++;
                        break;
                    }
                    else
                    {
                        temp = 0;
                    }
                }
                else
                {

                    counter++;
                    break;
                }
            }
        }
        token = strtok(NULL, " ");
    }
    free(token);
    if(counter == parts)
        return true;
    return false;
}



/*
bool matches(char sentence[],char query [])
{
    char tmp[strlen(query)];
    strcpy(tmp, query);
    int i;
    int temp=0;
    char* token = strtok(tmp, " ");
    int parts = 0, counter = 0;
    while(token != NULL)
    {
        parts++;
        temp = 0;
        for (i=0 ; i< strlen(sentence) ; i++)
        {
            if(sentence[i]==token[temp])
            {
                temp++;
            }
            else
            {
                temp=0;
            }
            if (temp==(strlen(token)-1))
            {
                counter++;
                break;
            }
        }
        token = strtok(NULL, " ");
    }
    if(counter == parts)
        return true;
    return false;
}
*/


MPI_Status status;

int main(int argc, char * argv[])
{

    int my_rank = 0, p = 0;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    double start = MPI_Wtime();
    double end = 0.0;
    double time = 0.0;
    double totalTime = 0.0;
    double start2 = clock();
    double end2 = 0.0;
    double time2 = 0.0;
    double totalTime2 = 0.0;

    char query[500];
    char sentence[500];
    int counter = 0;
    FILE *readFile;
    FILE *writeFile;
    FILE *writeFile2;
    char mat[1500][500];
    int i = 0, r = 0;

    int slaves = p - 1;
    int j = 0, k = 0, q = 0, res = 0;

    int *arr;

    int *counterArr;
    int files[50];

    int sz = 0, extra = 0, extraIndex = 0;
    int checkExtra = 0;

    char ifile[25];
    int len = 0;
    if(p <= 0 || p > 50)
    {
        if(my_rank == 0)
            printf("Please check the number of processors.\n");
        MPI_Finalize();
        return 0;
    }
    sz = 50 / p;
    extra = 50%p;
    if(my_rank > 49);
    else
    {


        if(my_rank == 0)
        {
            counterArr = malloc(p * sizeof(int));
            for(k = 0 ; k < p ; k++)
                counterArr[k] = 0;

            printf("Enter the Query: ");  /// read the query

            scanf("%[^\n]%*c", &query);

            printf("\n");

            ///get an array with all file names
            for(i = 0; i < 50; i++)
            {
                files[i] = (i + 1);
            }
            len = strlen(query);
            arr = malloc(sz * sizeof(int));
        }
        MPI_Bcast (&len, 1, MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast (&query, 500, MPI_CHAR,0,MPI_COMM_WORLD);
        MPI_Scatter (&files[0], sz, MPI_INT, &arr[0],sz, MPI_INT, 0, MPI_COMM_WORLD);
        counter = 0;
        extraIndex = 0;
        q = 0;
        for(i = 0 ; i < sz; i++)
        {

            sprintf(ifile,"Aristo-Mini-Corpus/Aristo-Mini-Corpus P-%d.txt",arr[i]);
            readFile  = fopen(ifile, "r");

            while ( fgets(&sentence[0], 500, readFile))
            {
                if(matches(sentence,query))
                {
                    strcpy(mat[q], sentence);
                    counter++;
                    q++;
                }
            }
            fclose(readFile);

        }

        MPI_Reduce(&counter,&res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Gather(&counter, 1, MPI_INT, &counterArr[0], 1,MPI_INT, 0, MPI_COMM_WORLD);

        extraIndex = 0;
        if(my_rank == 0)
        {
            for(i = 50 - extra ; i < 50; i++)
            {
                sprintf(ifile,"Aristo-Mini-Corpus/Aristo-Mini-Corpus P-%d.txt",(i+1));
                readFile  = fopen(ifile, "r");

                while ( fgets(&sentence[0], 500, readFile))
                {
                    if(matches(sentence,query))
                    {
                        strcpy(mat[q], sentence);
                        res++;
                        counter++;
                        q++;
                    }
                }
                fclose(readFile);
            }

            writeFile = fopen("Result.txt","w");
            fprintf(writeFile,"%s","Query : ");
            fprintf(writeFile,"%s\n",query);
            fprintf(writeFile,"%s","Found : ");
            fprintf(writeFile,"%d\n\n", res);
            for(i = 0 ; i < counter ; i++)
                fprintf(writeFile, "%s", mat[i]);
            fclose(writeFile);
            writeFile2 = fopen("Result2.txt","w");
            fprintf(writeFile2,"%s","Query : ");
            fprintf(writeFile2,"%s\n",query);
            fprintf(writeFile2,"%s","\nRank : ");
            fprintf(writeFile2,"%d\n", my_rank);
            fprintf(writeFile2,"%s","Found : ");
            fprintf(writeFile2,"%d\n\n", counter);
            for(i = 0 ; i < counter ; i++)
                fprintf(writeFile2, "%s", mat[i]);
            fclose(writeFile2);
            i = 1;
            for(r = 1; r < p; r++)
                MPI_Send(&i, 1, MPI_INT, r, 0, MPI_COMM_WORLD);

            free(arr);
            free(counterArr);
        }
        else
        {
            MPI_Recv(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            writeFile = fopen("Result.txt","a");
            for(i = 0 ; i < counter ; i++)
                fprintf(writeFile, "%s", mat[i]);
            fclose(writeFile);
            writeFile2 = fopen("Result2.txt","a");
            fprintf(writeFile2,"%s","\nRank : ");
            fprintf(writeFile2,"%d\n", my_rank);
            fprintf(writeFile2,"%s","Found : ");
            fprintf(writeFile2,"%d\n\n", counter);
            for(i = 0 ; i < counter ; i++)
                fprintf(writeFile2, "%s", mat[i]);
            fclose(writeFile2);
        }
    }


    end = MPI_Wtime();
    end2 = clock();
    time = end - start;
    time2 = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
    printf("Elapsed Time taken by process %d : %f\nTime taken in Seconds by process %d : %f\n",my_rank, time, my_rank, time2);
    MPI_Reduce(&time,&totalTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time2,&totalTime2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if(my_rank == 0)
    {
        printf("\nTotal Elapsed Time for all Processes : %f\n", totalTime);
        printf("\nTotal Time in Seconds for all Processes : %f\n\n", totalTime2);
    }

    MPI_Finalize();
    return 0;
}
