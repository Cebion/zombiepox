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



#ifndef _CSORTABLE_H_
#define _CSORTABLE_H_

#include <list>

using namespace std;

// forward declarations
class CActor;
class CMap;


class ISortable {
	public:
		int x() { return (int)m_x; }
		int y() { return (int)m_y; }
		void setX(int p_x) { m_x = p_x; }
		void setY(int p_y) { m_y = p_y; }
		virtual void render(BITMAP *p_bmp, int p_x, int p_y) = 0;
		virtual int update(CMap *p_map, list<ISortable *> *p_list) = 0;
		virtual int getMapRow() = 0;

	protected:
		// position
		double m_x, m_y;

};


class ByYPos {
   public:
      bool operator()(ISortable *a, ISortable *b) {
		  return a->y() < b->y();
		}
};



#endif

