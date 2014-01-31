#include "tileBlock.h"

#include "db.h"
#include <string.h>

#define GETID { *(int *)context = DBINT(0); return 0; }

static LIST_HEAD(tb_cache);

// any
DBSELECT(tb_any, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ?" RANDWEIGHT, "t") GETID

// single direction
DBSELECT(tb_top, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ?" RANDWEIGHT, "tt") GETID
DBSELECT(tb_right, "SELECT id FROM TileBlock WHERE tileset = ? AND right = ?" RANDWEIGHT, "tt") GETID
DBSELECT(tb_bottom, "SELECT id FROM TileBlock WHERE tileset = ? AND bottom = ?" RANDWEIGHT, "tt") GETID
DBSELECT(tb_left, "SELECT id FROM TileBlock WHERE tileset = ? AND left = ?" RANDWEIGHT, "tt") GETID

// corners
DBSELECT(tb_topRight, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ? AND right = ?" RANDWEIGHT, "ttt") GETID
DBSELECT(tb_topLeft, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ? AND left = ?" RANDWEIGHT, "ttt") GETID
DBSELECT(tb_bottomRight, "SELECT id FROM TileBlock WHERE tileset = ? AND bottom = ? AND right = ?" RANDWEIGHT, "ttt") GETID
DBSELECT(tb_bottomLeft, "SELECT id FROM TileBlock WHERE tileset = ? AND bottom = ? AND left = ?" RANDWEIGHT, "ttt") GETID

// accross
DBSELECT(tb_topBottom, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ? AND bottom = ?" RANDWEIGHT, "ttt") GETID
DBSELECT(tb_leftRight, "SELECT id FROM TileBlock WHERE tileset = ? AND left = ? AND right = ?" RANDWEIGHT, "ttt") GETID

// three sides
DBSELECT(tb_leftTopRight, "SELECT id FROM TileBlock WHERE tileset = ? AND left = ? AND top = ? AND right = ?" RANDWEIGHT, "tttt") GETID
DBSELECT(tb_topRightBottom, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ? AND right = ? AND bottom = ?" RANDWEIGHT, "tttt") GETID
DBSELECT(tb_rightBottomLeft, "SELECT id FROM TileBlock WHERE tileset = ? AND right = ? AND bottom = ? AND left = ?" RANDWEIGHT, "tttt") GETID
DBSELECT(tb_bottomLeftTop, "SELECT id FROM TileBlock WHERE tileset = ? AND bottom = ? AND left = ? AND top = ?" RANDWEIGHT, "tttt") GETID

// every side
DBSELECT(tb_topRightBottomLeft, "SELECT id FROM TileBlock WHERE tileset = ? AND top = ? AND right = ? AND bottom = ? AND left = ?" RANDWEIGHT, "ttttt") GETID

DBSELECT(tb_get, "SELECT id, tileset, top, left, bottom FROM TileBlock WHERE id = ?", "i") {
	struct tileBlock *tb = (struct tileBlock *)context;
	tb->id      = DBINT(0);
	tb->tileset = DBSTRING(1);
	tb->top     = DBSTRING(2);
	tb->left    = DBSTRING(3);
	tb->bottom  = DBSTRING(4);
	tb->right   = DBSTRING(5);
}

struct tileBlock *random_tileBlock(const char *tileset, const char *top, const char *right, const char *bottom, const char *left) {
	int c = (top    != NULL ? 1 : 0) +
		    (right  != NULL ? 2 : 0) +
		    (bottom != NULL ? 4 : 0) +
		    (left   != NULL ? 8 : 0) ;
	int r = 0, id = -1;
	struct tileBlock *tb;
	// ugly giant switch statement
	switch(c) {
	case 0:
		r = tb_any(&id, tileset);
		break;
	case 1:
		r = tb_top(&id, tileset, top);
		break;
	case 2:
		r = tb_right(&id, tileset, right);
		break;
	case 3:
		r = tb_topRight(&id, tileset, top, right);
		break;
	case 4:
		r = tb_bottom(&id, tileset, bottom);
		break;
	case 5:
		r = tb_topBottom(&id, tileset, top, bottom);
		break;
	case 6:
		r = tb_bottomRight(&id, tileset, bottom, right);
		break;
	case 7:
		r = tb_topRightBottom(&id, tileset, top, bottom, right);
		break;
	case 8:
		r = tb_left(&id, tileset, left);
		break;
	case 9:
		r = tb_topLeft(&id, tileset, top, left);
		break;
	case 10:
		r = tb_leftRight(&id, tileset, left, right);
		break;
	case 11:
		r = tb_leftTopRight(&id, tileset, left, top, right);
		break;
	case 12:
		r = tb_bottomLeft(&id, tileset, bottom, left);
		break;
	case 13:
		r = tb_bottomLeftTop(&id, tileset, bottom, left, top);
		break;
	case 14:
		r = tb_rightBottomLeft(&id, tileset, right, bottom, left);
		break;
	case 15:
		r = tb_topRightBottomLeft(&id, tileset, top, right, bottom, left);
		break;
	}
	if(r == 0) {
		return NULL;
	}
	list_for_each_entry(tb, struct tileBlock, &tb_cache, cache) {
		if(tb->id == id) {
			return tb;
		}
	}
	tb = (struct tileBlock *)malloc(sizeof(*tb));
	tb_get(tb, id);
	list_add(&tb->cache, &tb_cache);
	return tb;
}

