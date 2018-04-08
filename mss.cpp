/*
 * algorithm: merge splitting sort
 * author: maros vasilisin
 */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <array>
#include <string>
#include <climits>

using namespace std;

#define TAG 0

int main(int argc, char* argv[]) {
    
    int processorCount;
    int myProcessId;
    MPI_Status stat; 

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processorCount); 
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcessId);

    int numbersArg = stoi(argv[1], nullptr, 10);

    // vypocet kolko cisiel sa vojde do kazdeho procesoru
    const int arraySize = ((numbersArg % processorCount) == 0)
        ? (numbersArg / processorCount)
        : ((numbersArg / processorCount) + 1);

    int myNumbers[arraySize];
    int numbersFromNextProc[arraySize];
    for (int i = 0; i < arraySize; i++) {
        myNumbers[i] = INT_MAX;
    }
    for (int i = 0; i < arraySize; i++) {
        numbersFromNextProc[i] = INT_MAX;
    }

    if (myProcessId == 0) {
        char input[]= "numbers";
        
        int numbers[arraySize]; 
        for (int i = 0; i < arraySize; i++) {
            numbers[i] = INT_MAX;
        }
        int invar = 0; // invariant - urcuje cislo proc, ktoremu se bude posielat
        fstream fin; 
        fin.open(input, ios::in);  

        while (fin.good()) {
            for (int i = 0; i < arraySize; i++) {
              numbers[i] = fin.get();
              if (!fin.good()) { // ak nacitame EOF tak koniec
                numbers[i] = INT_MAX;
                break;
              }
            }
            int it = 0;
            while (it < arraySize) {
                if (numbers[it] != INT_MAX) {
                    MPI_Send(&numbers, arraySize, MPI_INT, invar, TAG, MPI_COMM_WORLD); // poslanie hodnot do procesoru s danym cislom invar
                    invar++;
                    break;
                }
                it++;
            }
        
            for (int x = 0; x < (arraySize - 1); x++) {
                if (numbers[x] != INT_MAX) {
                    cout << numbers[x] << " ";
                }
            }
            if (numbers[arraySize - 1] != INT_MAX) {
                cout << numbers[arraySize - 1];
                if (invar != processorCount) {
                    cout << " ";
                }
            }
            for (int i = 0; i < arraySize; i++) {
                numbers[i] = INT_MAX;
            }
        }
        cout << endl;
        // este jedno poslanie, v specialnych pripadoch ak by sa nenaplnili vsetky procesory
        while (invar < processorCount) {
            MPI_Send(&numbers, arraySize, MPI_INT, invar, TAG, MPI_COMM_WORLD);
            invar++;
        }

        fin.close();                                
    }

    // vsetky procesory prijmu hodnotu z procesoru 0
    MPI_Recv(&myNumbers, arraySize, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
 
    // zoradenie hodnot 
    sort(myNumbers, myNumbers + arraySize);
 
    // pocet iteracii
    int iterations = ((processorCount % 2) == 0) ? (processorCount/2) : ((processorCount/2) + 1);
    int limit = processorCount-1;

    for (int j = 1; j <= iterations; j++) { 

        if ((!(myProcessId % 2) || myProcessId == 0) && (myProcessId < limit)) {
            MPI_Send(&myNumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD);        
            MPI_Recv(&myNumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD, &stat); 
        }
        else if ((myProcessId % 2) && (myProcessId <= limit)) {
            MPI_Recv(&numbersFromNextProc, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD, &stat);

            int sumNumbers[arraySize * 2];
            for (int i = 0; i < arraySize; i++) {
                sumNumbers[i] = myNumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                sumNumbers[i] = numbersFromNextProc[i-arraySize];
            }
            sort(sumNumbers, sumNumbers + (arraySize * 2));
            int lowerPart[arraySize];
            int higherPart[arraySize];
            for (int i = 0; i < arraySize; i++) {
                lowerPart[i] = sumNumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                higherPart[i-arraySize] = sumNumbers[i];
            }

            for (int i = 0; i < arraySize; i++) {
                myNumbers[i] = higherPart[i];
            }

            MPI_Send(&lowerPart, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD); 
        }

        if ((myProcessId % 2) && (myProcessId < limit)) {
            MPI_Send(&myNumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD); 
            MPI_Recv(&myNumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD, &stat); 
        }
        else if (myProcessId <= limit && myProcessId != 0 && !(myProcessId % 2)) {
            MPI_Recv(&numbersFromNextProc, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD, &stat);

            int sumNumbers[arraySize * 2];
            for (int i = 0; i < arraySize; i++) {
                sumNumbers[i] = myNumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                sumNumbers[i] = numbersFromNextProc[i-arraySize];
            }
            sort(sumNumbers, sumNumbers + (arraySize * 2));
            int lowerPart[arraySize];
            int higherPart[arraySize];
            for (int i = 0; i < arraySize; i++) {
                lowerPart[i] = sumNumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                higherPart[i-arraySize] = sumNumbers[i];
            }
            for (int i = 0; i < arraySize; i++) {
                myNumbers[i] = higherPart[i];
            }
            MPI_Send(&lowerPart, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD);
        }
        
    }

    int* result = new int [processorCount * arraySize];
    for (int i = 1; i < processorCount; i++) {
        if (myProcessId == i) {
            MPI_Send(&myNumbers, arraySize, MPI_INT, 0, TAG,  MPI_COMM_WORLD);
        }
        if (myProcessId == 0) {
            MPI_Recv(&numbersFromNextProc, arraySize, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
            for (int x = 0; x < arraySize; x++) {
                result[(i*arraySize) + x] = numbersFromNextProc[x];
            }
        }
    }

    if (myProcessId == 0) {
        for (int x = 0; x < arraySize; x++) {
           result[x] = myNumbers[x];
        }
        for (int x = 0; x < ((arraySize * processorCount) - 1); x++) {
            if (result[x] != INT_MAX) {
                cout << result[x] << endl;                
            }
        }
        if (result[(arraySize * processorCount) - 1] != INT_MAX) {
            cout << result[(arraySize * processorCount) - 1] << endl;
        }
    }

    MPI_Finalize(); 
    return 0;

}

