#include "tileMap.h"

#include <string.h>
#include <stdlib.h>

// TODO test and fix, will cause islands of unconnected blocks

int _not_valid(struct tileMap *tileMap, int *row, int *column) {
	int r, c, o;
	for(r = 0; r < 32; r++) {
		for(c = 0; c < 32; c++) {
			if(tileMap->blocks[r][c]) {
				if(tileMap->blocks[r][c]->top && tileMap->blocks[r-1][c] == NULL) {
					*row = r-1;
					*column = c;
					return 1;
				}
				if(tileMap->blocks[r][c]->right && tileMap->blocks[r][c+1] == NULL) {
					*row = r;
					*column = c+1;
					return 1;
				}
				if(tileMap->blocks[r][c]->bottom && tileMap->blocks[r+1][c] == NULL) {
					*row = r+1;
					*column = c;
					return 1;
				}
				if(tileMap->blocks[r][c]->left && tileMap->blocks[r][c-1] == NULL) {
					*row = r;
					*column = c-1;
					return 1;
				}
			}
		}
	}
	return 0;
}

void tileMap_generate(struct tileMap *tileMap, const char *tileset) {
	int row, column, i;

	// clear out the tileMap
	memset(tileMap, 0, sizeof(*tileMap));

	// seed the map with a bunch of random blocks
	for(i = 0; i < 128; i++) {
		row = (rand() % 30) + 1;
		column = (rand() % 30) + 1;

		// generate a random block, respecting previously droped tileBlocks
		tileMap->blocks[row][column] = random_tileBlock(tileset,
			tileMap->blocks[row-1][column] ? tileMap->blocks[row-1][column]->bottom : NULL ,
			tileMap->blocks[row][column+1] ? tileMap->blocks[row][column+1]->left   : NULL ,
			tileMap->blocks[row+1][column] ? tileMap->blocks[row+1][column]->top    : NULL ,
			tileMap->blocks[row][column-1] ? tileMap->blocks[row][column-1]->right  : NULL );
	}

	// keep placing blocks until we cannot find a invalid block
	while(_not_valid(tileMap, &row, &column)) {
		tileMap->blocks[row][column] = random_tileBlock(tileset,
			row    ==  0 ? "" : tileMap->blocks[row-1][column] ? tileMap->blocks[row-1][column]->bottom : NULL ,
			column == 31 ? "" : tileMap->blocks[row][column+1] ? tileMap->blocks[row][column+1]->left   : NULL ,
			row    == 31 ? "" : tileMap->blocks[row+1][column] ? tileMap->blocks[row+1][column]->top    : NULL ,
			column ==  0 ? "" : tileMap->blocks[row][column-1] ? tileMap->blocks[row][column-1]->right  : NULL );
		if(tileMap->blocks[row][column] == NULL) {
			if(row    !=  0) tileMap->blocks[row-1][column] = NULL;
			if(column != 31) tileMap->blocks[row][column+1] = NULL;
			if(row    != 31) tileMap->blocks[row+1][column] = NULL;
			if(column !=  0) tileMap->blocks[row][column-1] = NULL;
		}
	}
}
