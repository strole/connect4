#pragma once
// Deklaracija razreda Board

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
typedef int data1;
const data1 EMPTY = 0;
const data1 CPU = 1;
const data1 HUMAN = 2;

class Board
{
private:
	data1** field;
	int* height;
	int rows, cols;
	data1 LastMover;
	int lastcol;
	void Take();	// zauzmi i popuni prazninama
	void Free();
public:
	Board() : rows(6), cols(7), LastMover(EMPTY), lastcol(-1)
	{
		Take();
	}
	Board(const int row, const int col) : rows(row), cols(col), LastMover(EMPTY), lastcol(-1)
	{
		Take();
	}
	~Board()
	{
		Free();
	}
	int Columns()	// broj stupaca
	{
		return cols;
	}
	Board(const Board& src);
	data1* operator[](const int row);
	bool MoveLegal(const int col);	// moze li potez u stupcu col
	bool Move(const int col, const data1 player);	// napravi potez
	bool UndoMove(const int col);	// vrati potez iz stupca
	bool GameEnd(const int lastcol);	// je li zavrsno stanje
	bool Load(const char* fname);
	void Save(const char* fname);
	void Print();
};

inline data1* Board::operator[](const int row)
{
	assert(row >= 0 && row < rows);
	return field[row];
}