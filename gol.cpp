/*
 * CSc103 Project 3: Game of Life
 * See readme.html for details.
 * Please list all references you made use of in order to complete the
 * assignment: your classmates, websites, etc.  Aside from the lecture notes
 * and the book, please list everything.  And remember- citing a source does
 * NOT mean it is okay to COPY THAT SOURCE.  What you submit here **MUST BE
 * YOUR OWN WORK**.
 * References:
 *
 *
 * Finally, please indicate approximately how many hours you spent on this:
 * #hours: 
 */

#include <cstdio>
#include <stdlib.h> // for exit();
#include <getopt.h> // to parse long arguments.
#include <unistd.h> // sleep
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;

static const char* usage =
"Usage: %s [OPTIONS]...\n"
"Text-based version of Conway's game of life.\n\n"
"   --seed,-s     FILE     read start state from FILE.\n"
"   --world,-w    FILE     store current world in FILE.\n"
"   --fast-fw,-f  NUM      evolve system for NUM generations and quit.\n"
"   --help,-h              show this message and exit.\n";

size_t max_gen = 0; /* if > 0, fast forward to this generation. */
string wfilename =  "/tmp/gol-world-current"; /* write state here */
FILE* fworld = 0; /* handle to file wfilename. */
string initfilename = "/tmp/gol-world-current"; /* read initial state from here. */

vector<vector<bool> > grid; //current grid
vector<vector<bool> > nextGen; //new/future grid to be used for updating

size_t nbrCount(size_t i, size_t j, const vector<vector<bool> >& g); //to count neighbors of each cell
void update(); //updates the grid after counting each cell's neighbors
int initFromFile(const string& fname); //opens the file to be used
void mainLoop();
void dumpState(FILE* f); //where the file will be written
void printGen(); //used when max_gen > 0 and needs to print out that gen's cells

int main(int argc, char *argv[]) {
	// define long options
	static struct option long_opts[] = {
		{"seed",    required_argument, 0, 's'},
		{"world",   required_argument, 0, 'w'},
		{"fast-fw", required_argument, 0, 'f'},
		{"help",    no_argument,       0, 'h'},
		{0,0,0,0}
	};
	// process options:
	char c;
	int opt_index = 0;
	while ((c = getopt_long(argc, argv, "hs:w:f:", long_opts, &opt_index)) != -1) {
		switch (c) {
			case 'h':
				printf(usage,argv[0]);
				return 0;
			case 's':
				initfilename = optarg;
				break;
			case 'w':
				wfilename = optarg;
				break;
			case 'f':
				max_gen = atoi(optarg);
				break;
			case '?':
				printf(usage,argv[0]);
				return 1;
		}
	}

	initFromFile(initfilename);
	mainLoop();
	return 0;
}

void mainLoop() {
	/* update, write, sleep */
	fworld = fopen(wfilename.c_str(), "wb");
	while(max_gen == 0) {
		update();
		dumpState(fworld);
		sleep(1);
	}
	for(size_t i = 0; i < max_gen; i++) {
		update();
		dumpState(fworld);
		if(i == max_gen - 1) printGen();
	}
}

void printGen() {
	for(size_t i = 0; i < grid.size(); i++) {
		for(size_t j = 0; j < grid[i].size(); j++) {
			if(grid[i][j] == false) printf(".");
			else if(grid[i][j] == true) printf("O");
		}
		printf("\n");
	}
}

int initFromFile(const string& fname) {
	FILE* f = fopen(fname.c_str(), "rb");
	if (!f) exit(1);
	int letter; //for reading individual characters
	vector<bool> row; //the current row of the grid
	
	//determines the values of the grid based on characters within the file
	while((letter = getc(f)) != EOF) {
		if(letter == '\n') {
			grid.push_back(row);
			row.clear();
		}
		else if(letter == '.') row.push_back(false); //cell is dead
		else if(letter == 'O') row.push_back(true); //cell is alive
	}
	rewind(f); 

	return 0;
}

void update() {
	/* If the cell is alive and has < 2 neighbors or > 3 neighbors, it dies from 
	 * underpopulation and overpopulation respectively.
	 * If the cell is dead but it has exactly 3 neighbors, the cell becomes alive from
	 * reproduction.
	 * If the cell has exactly 2 neighbors, the cell does not change at all.
	 * */
	vector<bool> temp;
	for(size_t i = 0; i < grid.size(); i++) {
		 for(size_t j = 0; j < grid[i].size(); j++) {
			if(nbrCount(i,j,grid) < 2 || nbrCount(i,j,grid) > 3) {
				temp.push_back(false);
			} else if(nbrCount(i,j,grid) == 3) {
				temp.push_back(true);
			} else if(nbrCount(i,j,grid) == 2) { 
				temp.push_back(grid[i][j]);
			}
		 }
		 nextGen.push_back(temp);
		 temp.clear();
	} 
	grid.clear();
	grid = nextGen;
	nextGen.clear();
}

size_t nbrCount(size_t i, size_t j, const vector<vector<bool> >& g) {
	size_t neighbors = 0;
	int lastColumn = g[0].size()-1; //last column index number
	int lastRow = g.size()-1; //last row index number
	
	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			if(x == 0 && y == 0) continue;
			int checkColumn = j + x;
			int checkRow = i + y;

			if(checkColumn < 0) {
				checkColumn = lastColumn;
			} else if(checkColumn > lastColumn) {
				checkColumn = 0;
			}

			if(checkRow < 0) {
				checkRow = lastRow;
			} else if(checkRow > lastRow) {
				checkRow = 0;
			}

			if(g[checkRow][checkColumn] == true) neighbors++;
		}
	}
	return neighbors;
}

void dumpState(FILE* f) {
	char c;
	//writes the alive/dead character based on its boolean value from the vector
	for(size_t i = 0; i < grid.size(); i++) {
		for(size_t j = 0; j < grid[i].size(); j++) {
			if(grid[i][j]) c = 'O';
			else c = '.';
			fwrite(&c,1,1,f);
		}
		c = '\n';
		fwrite(&c,1,1,f);
	}
	rewind(f);
}
