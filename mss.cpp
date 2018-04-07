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
    
    int processorCount; // pocet procesorov
    int myProcessId; // id nasho procesu
    MPI_Status stat;            //struct- obsahuje kod- source, tag, error

    //MPI INIT
    MPI_Init(&argc, &argv); // inicializacia MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &processorCount); // zistime, kolko procesov bezi 
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcessId); // zistime id nasho procesu 

    int processorsArg = stoi(argv[2], nullptr, 10);
    int numbersArg = stoi(argv[1], nullptr, 10);

    const int arraySize = ((numbersArg % processorsArg) == 0)
        ? (numbersArg / processorsArg)
        : ((numbersArg / processorsArg) + 1);

    int mynumbers[arraySize];              //moje hodnota
    int neighnumbers[arraySize];            //hodnota souseda
    for (int i = 0; i < arraySize; i++) {
        mynumbers[i] = INT_MAX;
    }
    for (int i = 0; i < arraySize; i++) {
        neighnumbers[i] = INT_MAX;
    }

    //NACITANIE SOUBORU
    if (myProcessId == 0) {
        char input[]= "numbers"; // meno suboru so vstupnymi hodnotami    
        
        int numbers[arraySize]; // hodnota pri nacitani suboru
        for (int i = 0; i < arraySize; i++) {
            numbers[i] = INT_MAX;
        }
        int invar = 0; // invariant - urcuje cislo proc, ktoremu se bude posielat
        fstream fin; // stream pre nacitanie zo suboru
        fin.open(input, ios::in);  

        while (fin.good()) {
            for (int i = 0; i < arraySize; i++) {
              numbers[i] = fin.get();
              if (!fin.good()) {
                numbers[i] = INT_MAX;
                break;
              }
            }
            int it = 0;
            while (it < arraySize) {
                if (numbers[it] != INT_MAX) {
                    MPI_Send(&numbers, arraySize, MPI_INT, invar, TAG, MPI_COMM_WORLD); //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina
                    break;
                }
                it++;
            }
            invar++;
            for (int x = 0; x < (arraySize - 1); x++) {
                if (numbers[x] != INT_MAX) {
                    cout << numbers[x] << " ";
                }
            }
            if (numbers[arraySize - 1] != INT_MAX) {
                cout << numbers[arraySize - 1];
                if (invar != processorsArg) {
                    cout << " ";
                }
            }
            for (int i = 0; i < arraySize; i++) {
                numbers[i] = INT_MAX;
            }
        }
        cout << endl;

        fin.close();                                
    }

    //PRIJETI HODNOTY CISLA
    //vsechny procesory(vcetne mastera) prijmou hodnotu a zahlasi ji
    MPI_Recv(&mynumbers, arraySize, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
 
    sort(mynumbers, mynumbers + arraySize);
 
    //LIMIT PRO INDEXY
    int oddlimit = 2*(processorCount/2)-1;                 //limity pro sude
    int evenlimit = 2*((processorCount-1)/2);              //liche
    int halfcycles = processorCount/2;
    int cycles=0;                                   //pocet cyklu pro pocitani slozitosti

    //RAZENI------------chtelo by to umet pocitat cykly nebo neco na testy------
    //cyklus pro linearitu
    for(int j=1; j<=halfcycles; j++){
        cycles++;           //pocitame cykly, abysme mohli udelat krasnej graf:)

        //sude proc 
        if((!(myProcessId % 2) || myProcessId == 0) && (myProcessId < oddlimit)) {
            MPI_Send(&mynumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD);          //poslu sousedovi svoje cislo
            MPI_Recv(&mynumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD, &stat);   //a cekam na nizsi
        }//if sude
        else if(myProcessId <= oddlimit){//liche prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)
            MPI_Recv(&neighnumbers, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD, &stat); //jsem sudy a prijimam

            int sumNumbers[arraySize * 2];
            for (int i = 0; i < arraySize; i++) {
                sumNumbers[i] = mynumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                sumNumbers[i] = neighnumbers[i-arraySize];
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
                mynumbers[i] = higherPart[i];
            }

            MPI_Send(&lowerPart, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD);       //poslu svoje 
        }//else if (liche)
        else{//sem muze vlezt jen proc, co je na konci
        }//else

        //liche proc 
        if((myProcessId % 2) && (myProcessId < evenlimit)) {
            MPI_Send(&mynumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD);           //poslu sousedovi svoje cislo
            MPI_Recv(&mynumbers, arraySize, MPI_INT, myProcessId+1, TAG, MPI_COMM_WORLD, &stat);    //a cekam na nizsi
        }//if liche
        else if(myProcessId <= evenlimit && myProcessId != 0){//sude prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)
            MPI_Recv(&neighnumbers, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD, &stat); //jsem sudy a prijimam

            int sumNumbers[arraySize * 2];
            for (int i = 0; i < arraySize; i++) {
                sumNumbers[i] = mynumbers[i];
            }
            for (int i = arraySize; i < (arraySize * 2); i++) {
                sumNumbers[i] = neighnumbers[i-arraySize];
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
                mynumbers[i] = higherPart[i];
            }
            MPI_Send(&lowerPart, arraySize, MPI_INT, myProcessId-1, TAG, MPI_COMM_WORLD);       //poslu svoje
        }//else if (sude)
        else{//sem muze vlezt jen proc, co je na konci
        }//else
        
    }//for pro linearitu
    //RAZENI--------------------------------------------------------------------


    //FINALNI DISTRIBUCE VYSLEDKU K MASTEROVI-----------------------------------
    int* final= new int [processorCount * arraySize];
    for (int i = 1; i < processorCount; i++) {
        if (myProcessId == i) {
            MPI_Send(&mynumbers, arraySize, MPI_INT, 0, TAG,  MPI_COMM_WORLD);
        }
        if (myProcessId == 0) {
            MPI_Recv(&neighnumbers, arraySize, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat); //jsem 0 a prijimam
            for (int x = 0; x < arraySize; x++) {
                final[(i*arraySize) + x] = neighnumbers[x];
            }
        }//if sem master
    }//for

    if (myProcessId == 0) {
        for (int x = 0; x < arraySize; x++) {
           final[x] = mynumbers[x];
        }
        for (int x = 0; x < ((arraySize * processorCount) - 1); x++) {
            if (final[x] != INT_MAX) {
                cout << final[x] << endl;                
            }
        }
        if (final[(arraySize * processorCount) - 1] != INT_MAX) {
            cout << final[(arraySize * processorCount) - 1] << endl;
        }
    }//if vypis
    //VYSLEDKY------------------------------------------------------------------
    
    MPI_Finalize(); 
    return 0;

}

