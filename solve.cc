#include <assert.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#define BOARDSIZE 9
#define BITS_PER_BYTE 8
#define DIVIDE_CEILING(a, b) ((a + b - 1) / b)

char FiveBitLookup[] = { 
    0, 1, 1, 2, 1, 2, 2, 3,  /* 0-7 */
    1, 2, 2, 3, 2, 3, 3, 4,  /* 8-15 */
    1, 2, 2, 3, 2, 3, 3, 4,  /* 16-23 */
    2, 3, 3, 4, 3, 4, 4, 5,  /* 24-31 */
};
 
int bitcount(unsigned int bitset)
{
    int num = 0;
    while (bitset) {
	num += FiveBitLookup[bitset & 0x1f];
	bitset = bitset >> 5;
    }
    return num;
}

class Board {
public:
    Board()
    {
        memset(storage, 0, sizeof(storage));
    }
    void set(int index, char value) {
        int store = index / 2;
        if (index % 2 == 0) {
            storage[store] = (storage[store] & 0xf0) | value;
        } else {
            storage[store] = (storage[store] & 0x0f) | (value << 4);
        }
    }

    char get(int index) const {
        int store = index / 2;
        if (index % 2 == 0) {
            return storage[store] & 0x0f;
        } else {
            return storage[store] >> 4;
        }
    }
private:
    unsigned char storage[DIVIDE_CEILING(4 * BOARDSIZE * BOARDSIZE, BITS_PER_BYTE)]; // 4 bits * 81 squares
};

class CandidateSetIterator {
public:
    CandidateSetIterator()
	: bitset(0),
	  shiftamount(0)
    {}

    CandidateSetIterator(short bitset)
        : bitset(bitset),
          shiftamount(0)
    {}

    bool more() const
    {
        return bitset;
    }

    int next()
    {
	int lowbit;
        do {
            lowbit = bitset & 0x1;
            bitset = bitset >> 1;
            shiftamount++;
        } while (!lowbit && bitset);
        return shiftamount - 1;
    }

    int count() const
    {
	return bitcount(bitset);
    }

private:
    short bitset;
    char shiftamount;
};

class CandidateSet {
public:
    CandidateSet()
        : data(0x3fe)
    {}

    void unset(int bit)
    {
        data &= ~(1 << bit);
    }

    bool isset(int bit) const
    {
        return data & (1 << bit);
    }

    void reset()
    {
        data = 0x3fe;
    }

    int count() const
    {
	return bitcount(data);
    }

    CandidateSetIterator iterator() const
    {
        return CandidateSetIterator(data);
    }

private:
    short data;
};

std::string pretty(Board &board);

int load(FILE *f, Board &b)
{
    int i = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c >= '1' && c <= '9') {
            b.set(i, c - '0');
            i++;
        }
        else if (c == 'x') {
            b.set(i, 0);
            i++;
        }
        else if (c != '\n') {
            fprintf(stderr, "Unknown character: %d=%c\n", c, c);
            return 1;
        }
    }
    if (i != 81) {
        fprintf(stderr, "Wrong number of characters: %d, expecting 81\n", i);
        return 1;
    }
    if (feof(f)) {
        return 0;
    }
    perror("fgetc");
    return 1;
}

bool legal(const Board &board)
{
    CandidateSet cset;
    for (int i = 0; i < 9; i++) {
        cset.reset();
        // check rows
        for (int j = 0; j < 9; j++) {
            int value = board.get(i * 9 + j);
            if (!cset.isset(value)) {
                return false;
            }
            cset.unset(value);
        }
        cset.reset();
        // check columns
        for (int j = 0; j < 9; j++) {
            int value = board.get(i + j * 9);
            if (!cset.isset(value)) {
                return false;
            }
            cset.unset(value);
        }
        cset.reset();
        // check subcells
        for (int j = 0; j < 9; j++) {
            int value = board.get((j / 3 + i / 3 * 3) * 9 + j % 3 + i % 3 * 3);
            if (!cset.isset(value)) {
                return false;
            }
            cset.unset(value);
        }
    }
    return true;
}

CandidateSetIterator legalmoves(Board &board, int index)
{
    CandidateSet cset;
    for (int i = 0; i < 9 & cset.count() > 0; i++)
    {
        cset.unset(board.get(index / 9 * 9 + i));
        cset.unset(board.get(index % 9 + i * 9));
        cset.unset(board.get((i / 3 + index / 27 * 3) * 9 + i % 3 + index % 9 / 3 * 3));
    }
    return cset.iterator();
}

void findnext(Board &board, int &index, CandidateSetIterator &moves)
{
    int bestcount = 10;
    index = -1;

    for (int i = 0; i < BOARDSIZE * BOARDSIZE; i++)
    {
	if (board.get(i) == 0)
	{
	    CandidateSetIterator current = legalmoves(board, i);
	    if (current.count() <= 1)
	    {
		// done
		index = i;
		moves = current;
		return;
	    }
	    else if (current.count() < bestcount)
	    {
		bestcount = current.count();
		index = i;
		moves = current;
	    }
	}
    }
}

bool solve(Board &board)
{
    int index;
    CandidateSetIterator moves;

    findnext(board, index, moves);
    if (index == -1)
    {
	// nothing left to do
	return true;
    }

    while (moves.more())
    {
	board.set(index, moves.next());
	bool correct = solve(board);
	if (correct)
	{
	    return true;
	}
    }
    board.set(index, 0);
    return false;
}

std::string pretty(Board &board)
{
    std::ostringstream s;
    for (int i = 0; i < BOARDSIZE * BOARDSIZE; i++)
    {
	s << (int) board.get(i);
	if (i % 9 == 8)
	{
	    s << std::endl;
	}
	else if (i % 3 == 2)
	{
	    s << " ";
	}
    }
    return s.str();
}

int main(int argc, char **argv)
{
    Board board;
    FILE *input = stdin;
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
    {
	printf("Usage: %s [filename]\n", argv[0]);
	return 3;
    }
    else if (argc == 2)
    {
	input = fopen(argv[1], "r");
	if (input == NULL) {
	    perror("fopen");
	    return 2;
	}
    }

    if (load(input, board) != 0)
    {
	return 2;
    }
    fclose(input);

    bool solved = solve(board);
    if (solved) 
    {
	std::cout << "Solved: " << std::endl
		  << pretty(board) << std::endl;
	return 0;
    }
    else
    {
	    std::cout << "Solution not found" << std::endl;
	    return 1;
    }
    return 0;
}
