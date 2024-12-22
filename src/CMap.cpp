/****************************************************************
 *  ______ ____  __  __ ____  _____ ______ _____   ____ __   __ *
 * |___  // __ \|  \/  |  _ \|_   _|  ____|  __ \ / __ \\ \ / / *
 *    / /| |  | | \  / | |_) | | | | |__  | |__) | |  | |\   /  *
 *   / / | |  | | |\/| |  _ <  | | |  __| |  ___/| |  | | > <   *
 *  / /__| |__| | |  | | |_) |_| |_| |____| |    | |__| |/   \  *
 * /_____|\____/|_|  |_|____/|_____|______|_|     \____//_/ \_\ *
 *                                                              *
 ****************************************************************
 *     (c) Free Lunch Design 2003-2005                          *
 *     Written by Johan Peitz                                   *
 *     http://www.freelunchdesign.com                           *
 ****************************************************************
 *     This source code is released under the The GNU           *
 *     General Public License (GPL). Please refer to the        *
 *     document license.txt in the source directory or          *
 *     http://www.gnu.org for license information.              *
 ****************************************************************/


#include "CMap.h"
#include "main.h"


CMap :: CMap(int p_width, int p_height, BITMAP *p_tiles, BITMAP *p_walls, int level) {
	int x, y;
	int wallProbability = 92;

	m_width = MIN(p_width, 100);
	m_height = MIN(p_height, 100);
	m_tiles = p_tiles;
	m_walls = p_walls;

	// NULL tiles
	for(x = 0; x < 100; x ++) {
		for(y = 0; y < 100; y ++) {
			tiles[x][y] = NULL;
		}
	}

	// make tiles
	log("    creating tiles");
	for(x = 0; x < m_width; x ++) {
		for(y = 0; y < m_height; y ++) {
			int wall = 0;
			int wallTile = -1;
			if (y == 0 || x == 0 || x == m_width - 1 || y == m_height - 1) {
				wall = 1;
				wallTile = 0;
			}

			if (wall || rand()%100 > wallProbability) {
				if (wallTile == -1) {
					wallTile = rand()%(level > 3 ? (level > 5 ? 4 : 3) : 2) + 1;
				}
				tiles[x][y] = new CTile(m_walls, wallTile, 1);
			}
			else {
				int tile = rand()%(level > 2 ? (level > 4 ? 7 : 5) : 3) + 2;
				tiles[x][y] = new CTile(m_tiles, (rand()%100 > 80 ? tile : rand()%2), 0);
			}
		}
	}
}


CMap :: ~CMap() {
	int x, y;

	for(x = 0; x < 100; x ++) {
		for(y = 0; y < 100; y ++) {
			if (tiles[x][y] != NULL) {
				delete tiles[x][y];
			}
		}
	}
}


// checks wether the map has pockets/rooms where zombies can be trapped
int CMap :: hasPockets() {
	int x = 0, y = 0;
	BITMAP *map = create_bitmap(m_width, m_height);

	// draw map
	for(x = 0; x < m_width; x ++) {
		for(y = 0; y < m_height; y ++) {
			int c = (isWall(x*16, y*16) ? 1 : 0);
			_putpixel(map, x, y, c);
		}
	}

	// fill map
	int tx = 100 / 16;
	int ty = 100 / 16;
	floodfill(map, tx, ty, 2);

	// test map
	for(x = 0; x < m_width; x ++) {
		for(y = 0; y < m_height; y ++) {
			int c = _getpixel(map, x, y);
			if (c == 0) return 1;
		}
	}

	destroy_bitmap(map);

	return 0;
}



void CMap :: flatSpace(BITMAP *bmp, int x, int y) {
	int tx = x / 16;
	int ty = y / 16;
	tiles[tx+1][ty]->setID(2);
	tiles[tx][ty]->setID(2);
	tiles[tx+1][ty+1]->setID(2);
	tiles[tx][ty+1]->setID(2);
	tiles[tx+1][ty]->setImage(bmp);
	tiles[tx][ty]->setImage(bmp);
	tiles[tx+1][ty+1]->setImage(bmp);
	tiles[tx][ty+1]->setImage(bmp);
}



void CMap :: render(BITMAP *p_bmp, int p_cx, int p_cy, int p_row) {
	int x   /*, y*/;
	int tx, ty;
	int ax, ay;
	ay = p_cy % 16;
	ax = p_cx % 16;

	ty = p_row + p_cy / 16;
	if (ty < 0 || ty >= m_height) return;
	for(x = 0; x < 21; x ++) {
		tx = x + p_cx / 16;
		if (tx >= 0 && tx < m_width) {
			tiles[tx][ty]->render(p_bmp, x * 16 - ax, p_row * 16 - ay, (ty == m_height - 1 ? 1 : 0));
		}
	}
}



int CMap :: isWall(int p_x, int p_y) {
	int tx = p_x / 16;
	int ty = p_y / 16;

	if (tx < 0 || tx >= m_width) return 1;
	if (ty < 0 || ty >= m_height) return 1;

	return 	tiles[tx][ty]->isWall();
}

