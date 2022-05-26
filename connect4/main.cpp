// 4 u nizu - glavni program
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<ctime>
#include"board.h"
#include <mpi.h>// razred za igracu plocu
#include <string>
#include<chrono>
using namespace std;


const int DEPTH = 7;	// default dubina stabla

double Evaluate(Board Current, data1 LastMover, int iLastCol, int iDepth);

#define _PODACI 1
#define _ZAHTJEV 2
#define _ZADATAK 3
#define _REZULTAT 4
#define _CEKAJ 5
#define _KRAJ 6

int gotovo(Board B) {
	for (int iCol = 0; iCol < B.Columns(); iCol++)
			if (B.GameEnd(iCol))
			{
				cout << "Igra zavrsena!" << endl;
				return 0;
			}
	return 1;
}





int main(int argc, char** argv)
{

	int in_buffer[100];
	int out_buffer[100];
	int world_size, world_rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	const char* fname = "ploca.txt";
	int x = 1;
	int iDepth = DEPTH;
	srand((unsigned)time(NULL));


	// MASTER
	if (world_rank == 0) {
		MPI_Status status;
		Board B;
		double dResult, dBest;

		int iBestCol;


		bool x = 1;
		do {
			auto start = std::chrono::high_resolution_clock::now();
			dBest = -1;
			iBestCol = 7;
			B.Load(fname);
			int flag = 0;
			double rezultat[2];
			double zadatak[2];
			MPI_Status status;
			int List[7];
			int counter = 0;

			// pretpostavka: na potezu je CPU
			for (int iCol = 0; iCol < B.Columns(); iCol++)
				if (B.GameEnd(iCol))
				{
					B.Print();
					cout << "Igra zavrsena! (pobjeda racunala)" << endl;
					x = 1;
					MPI_Finalize();
					return 0;
				}
			do
			{
				int workers = world_size-1;
				int worker;
				int zadaci[7][7];
				bool done = true;

				for (int i = 0; i < 7; i++) {
					for (int j = 0; j < 7; j++) {
						zadaci[i][j] = 0;
					}
				}
				

				while (done) {
					bool x = 0;
					MPI_Recv(&rezultat, 2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
					
					if (status.MPI_TAG == _ZAHTJEV) {
						//cout << "zahtjev od "<<status.MPI_SOURCE<<"\n" << std::flush;
						for (int i = 0; i < 7; i++) {
							for (int j = 0; j < 7; j++) {
								if (zadaci[i][j] == 0 && B.MoveLegal(i)) {
									//cout << "saljem " << i << ", " << j << "\n" << std::flush;
									zadatak[0] = i;
									zadatak[1] = j;
									zadaci[i][j] = 1;
									MPI_Send(&i, 1, MPI_INT, status.MPI_SOURCE, _ZADATAK, MPI_COMM_WORLD);
									MPI_Send(&j, 1, MPI_INT, status.MPI_SOURCE, _ZADATAK, MPI_COMM_WORLD);
									x = 1;
									i = 9;
									j = 9;

								}
									
							}
						}
						if (x == 0) {
							workers--;
							//cout << "Micem workera.\n" << std::flush;
							if (workers == 0) {
								done = 0;
							}
							MPI_Send(&done, 1, MPI_INT, status.MPI_SOURCE, _CEKAJ, MPI_COMM_WORLD);
						}
						
					}
					else if (status.MPI_TAG == _REZULTAT) {
						if (rezultat[1] > dBest || (rezultat[1] == dBest && rand() % 2 == 0))
						{
							dBest = rezultat[1];
							iBestCol = rezultat[0];
						}
						//cout <<"Primljeno od workera: "<<status.MPI_SOURCE <<" Stupac " << rezultat[0] << ", vrijednost: " << rezultat[1] <<endl<< std::flush;
					}
					

				}
				//cout << "Dubina: " << iDepth << endl;
				//dBest = -1; iBestCol = -1;
				iDepth /= 2;
				// zasto petlja? ako svi potezi vode u poraz, racunamo jos jednom za duplo manju dubinu
				// jer igrac mozda nije svjestan mogucnosti pobjede
			} while (dBest == -1 && iDepth > 0);
			auto finish = std::chrono::high_resolution_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
			cout<<"Time: "<<microseconds.count() <<"ms \n" << std::flush;
			cout << "Najbolji: " << iBestCol << ", vrijednost: " << dBest <<endl<< std::flush;
			B.Move(iBestCol, CPU);
			B.Save(fname);
			// jesmo li pobijedili
			for (int iCol = 0; iCol < B.Columns(); iCol++)
				if (B.GameEnd(iCol))
				{
					B.Print();
					cout << "Igra zavrsena! (pobjeda racunala)" << endl;
					x = 1;
					MPI_Finalize();
					return 0;
				}


			B.Print();
			int redak=10;
			cout << "Unesite redak s kojim zelite igrati: ";
			cin >> redak;
			while (redak < 0 && redak>6) {
				cout << "Unesite redak s kojim zelite igrati: ";
				cin >> redak;
			}
			B.Move(redak, HUMAN);
			B.Save(fname);
			for (int iCol = 0; iCol < B.Columns(); iCol++)
				if (B.GameEnd(iCol))
				{
					B.Print();
					cout << "Igra zavrsena! (pobjeda covjeka!)" << endl;
					x = 1;
					MPI_Finalize();
					return 0;
				}
			
		} while (x==1);
		return 0;

	}
	//SLAVE
		else if(world_rank>0) {
			Board B;
			double dResult, dBest;

			int iBestCol, iDepth = DEPTH;

			const char* fname = "ploca.txt";
				B.Load(fname);
				double rezultat[2];
				int zadatak[2];
				MPI_Status status;
				int a = 0;
				while (true) {
					
					B.Load(fname);
					MPI_Send(&a, 1, MPI_INT, 0, _ZAHTJEV, MPI_COMM_WORLD);
					zadatak[0] = 0;
					zadatak[1] = 0;
					MPI_Recv(&zadatak[0], 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

					
					

					if (status.MPI_TAG != _CEKAJ)
					{ 
						MPI_Recv(&zadatak[1], 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
						//cout << "Primljeno: " << zadatak[0] << " , " << zadatak[1] << std::flush;
					B.Move(zadatak[0], CPU);
					dResult = Evaluate(B, CPU, zadatak[0], iDepth - 1);
					//cout << dResult << std::flush;
					//if (dResult == 1) {
						rezultat[0] = zadatak[0];
						rezultat[1] = dResult;
						B.UndoMove(zadatak[0]);
						MPI_Send(&rezultat, 2, MPI_DOUBLE, 0, _REZULTAT, MPI_COMM_WORLD);
					}
					//}
					/*else {
						B.Move(zadatak[1], HUMAN);
						dResult = Evaluate(B, HUMAN, zadatak[1], iDepth - 1);

						if (dResult == -1) {
							rezultat[0] = zadatak[0];
							rezultat[1] = dResult;
							B.UndoMove(zadatak[0]);
							B.UndoMove(zadatak[1]);
							MPI_Send(&rezultat, 2, MPI_DOUBLE, 0, _REZULTAT, MPI_COMM_WORLD);

						}
						else {
							B.UndoMove(zadatak[1]);
							dResult = Evaluate(B, CPU, zadatak[0], iDepth - 1);
							rezultat[0] = zadatak[0];
							rezultat[1] = dResult;
							B.UndoMove(zadatak[0]);
							MPI_Send(&rezultat, 2, MPI_DOUBLE, 0, _REZULTAT, MPI_COMM_WORLD);


						}
					}*/


				}
				

		
	
	}

	// provjerimo jel igra vec gotova (npr. ako je igrac pobijedio)
}

// rekurzivna funkcija: ispituje sve moguce poteze i vraca ocjenu dobivenog stanja ploce
// Current: trenutno stanje ploce
// LastMover: HUMAN ili CPU
// iLastCol: stupac prethodnog poteza
// iDepth: dubina se smanjuje do 0
double Evaluate(Board Current, data1 LastMover, int iLastCol, int iDepth)
{
	double dResult, dTotal;
	data1 NewMover;
	bool bAllLose = true, bAllWin = true;
	int iMoves;

	if (Current.GameEnd(iLastCol))	// igra gotova?
		if (LastMover == CPU)
			return 1;	// pobjeda
		else //if(LastMover == HUMAN)
			return -1;	// poraz
	// nije gotovo, idemo u sljedecu razinu
	if (iDepth == 0)
		return 0;	// a mozda i ne... :)
	iDepth--;
	if (LastMover == CPU)	// tko je na potezu
		NewMover = HUMAN;
	else
		NewMover = CPU;
	dTotal = 0;
	iMoves = 0;	// broj mogucih poteza u ovoj razini
	for (int iCol = 0; iCol < Current.Columns(); iCol++)
	{
		if (Current.MoveLegal(iCol))	// jel moze u stupac iCol
		{
			iMoves++;
			Current.Move(iCol, NewMover);
			dResult = Evaluate(Current, NewMover, iCol, iDepth);
			Current.UndoMove(iCol);
			if (dResult > -1)
				bAllLose = false;
			if (dResult != 1)
				bAllWin = false;
			if (dResult == 1 && NewMover == CPU)
				return 1;	// ako svojim potezom mogu doci do pobjede (pravilo 1)
			if (dResult == -1 && NewMover == HUMAN)
				return -1;	// ako protivnik moze potezom doci do pobjede (pravilo 2)
			dTotal += dResult;
		}
	}
	if (bAllWin == true)	// ispitivanje za pravilo 3.
		return 1;
	if (bAllLose == true)
		return -1;
	dTotal /= iMoves;	// dijelimo ocjenu s brojem mogucih poteza iz zadanog stanja
	return dTotal;
}
