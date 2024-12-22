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



#ifndef _CMAP_H_
#define _CMAP_H_

#include "CTile.h"


class CMap {
	public:
		CMap(int p_width, int p_height, BITMAP *p_tiles, BITMAP *p_walls, int level);
		virtual ~CMap();

		void render(BITMAP *p_bmp, int p_cx, int p_cy, int p_row);

		int isWall(int p_x, int p_y);

		int hasPockets();

		void flatSpace(BITMAP *p_bmp, int x, int y);
		int getWidth() { return m_width; }
		int getHeight() { return m_height; }

	private:
		int m_width, m_height;
		CTile *tiles[100][100];
		BITMAP *m_tiles;
		BITMAP *m_walls;
};

#endif

