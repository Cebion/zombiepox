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



#ifndef _CITEM_H_
#define _CITEM_H_

#include <allegro.h>

#include "ISortable.h"
#include "CItem.h"
#include "CActor.h"

class CItem : public ISortable {
	public:
		enum EItemType {
			eBrain,
			eBombBrain,
			eDynamite
		};

		CItem(EItemType p_type);
		virtual ~CItem();

		EItemType getType() { return m_type; }

		void render(BITMAP *p_bmp, int p_x, int p_y);
		int update(CMap *p_map, list<ISortable *> *p_list);
		void setFrames(BITMAP *p_bmp, int p_baseID);
		void setZ(int z) { m_z = z; }
		void setSpeed(double dx, double dy, double dz) { m_dx = dx; m_dy = dy; m_dz = dz; };

		int getMapRow() { return m_mapRow; }
		int collide(CActor *p_actor);

	protected:
		EItemType m_type;

		// position
		int m_mapRow;
		int m_life;

		// movement
		double m_dx, m_dy, m_dz;
		double m_z;

		// frames
		BITMAP *m_frames;
		int m_baseID;
		int m_frame;
		int m_animCount;


};

#endif

