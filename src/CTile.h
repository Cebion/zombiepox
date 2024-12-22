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


#ifndef _CTILE_H_
#define _CTILE_H_

#include <allegro.h>
#include <list>

using namespace std;

class CTile {
	public:
		CTile(BITMAP *p_image, int p_id, int p_isWall);

		void render(BITMAP *p_bmp, int p_x, int p_y, int p_trans);

		int isWall() { return m_isWall; }
		void setID(int p_id) { m_imgID = p_id; m_isWall = 0; }
		void setImage(BITMAP *bmp) { m_image = bmp; }

	private:
		int m_isWall;
		int m_imgID;
		BITMAP *m_image;
};

#endif

