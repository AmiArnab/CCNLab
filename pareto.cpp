#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int LAMBDA = 2, NUMSLOTS = 500;
double shape = 0.7, scale = 2;

int main()
{
    int n = 0, *hist;
    double uval = 0, eval = 0, cumuval = 0, *CEval, *PDF;

    ofstream myfile("data.dat");

    cout << "Enter n:";
    cin >> n;

    CEval = new double[n];
    hist = new int[NUMSLOTS];
    PDF = new double[NUMSLOTS/10];

    for(int i=0;i<NUMSLOTS;++i)
    {
        hist[i] = 0;
    }

    for(int i=0;i<n;++i)
    {
        uval = (float)rand()/RAND_MAX;
        eval = shape*(pow(uval,scale));
        cumuval += eval;
        CEval[i] = cumuval;
    }

    for(int i=0;i<n;++i)
    {
        int j = CEval[i] * (NUMSLOTS/CEval[n-1]);
        hist[j]++;
    }

    for(int i=0;i<NUMSLOTS;++i)
    {
        PDF[hist[i]]+= (1/(float)NUMSLOTS);
    }

    int slotsize = n/NUMSLOTS;
    for(int i=1;i<NUMSLOTS/10;++i)
    {
        myfile << i*(10/(float(NUMSLOTS))) << " " << slotsize*PDF[i] << endl;
    }

    myfile.close();

    delete [] CEval;
    delete [] hist;
    delete [] PDF;

    cout << "N = " << n << "\nN_slots = " << NUMSLOTS << endl;
    cout << "Shape: " << shape << "\nScale: " << scale << endl;

    pid_t childpid=fork();
    if(childpid==0) {
        std::FILE* pipehandle=popen("gnuplot -persist","w");
        //std::fprintf(pipehandle,"set xrange [0:1]\n");
        //std::fprintf(pipehandle,"set yrange [-2:15]\n");
        //std::fprintf(pipehandle,"set label 1 at 0.1 10\n");
        //std::fprintf(pipehandle,"set label 1 \"Poission distributed arrival\" tc lt 3\n");
        std::fprintf(pipehandle,"set title \"Pareto distributed arrival\"\n");
        std::fprintf(pipehandle,"plot \"data.dat\" smooth cspline\n");
        std::fprintf(pipehandle,"quit\n");
        std::fflush(pipehandle);
        std::fclose(pipehandle);
    }

    return 0;
}

