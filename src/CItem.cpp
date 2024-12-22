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


#include "CItem.h"

CItem :: CItem(EItemType p_type) {
	m_type = p_type;
	m_frames = NULL;
	m_frame = 0;
	m_animCount = 0;
	m_life = 100 * 10; // ten seconds
	m_dx = m_dy = m_dz = m_z = 0;
}


CItem :: ~CItem() {
}


void CItem :: render(BITMAP *p_bmp, int p_ox, int p_oy) {
	int x = (int)m_x - p_ox;
	int y = (int)m_y - p_oy;
	int by = (m_life < 24 ? 24 - m_life :  0);

	if (!by) ellipsefill(p_bmp, x + 8, y + 3, 6, 3, 150);
	masked_blit(m_frames, p_bmp, (m_baseID + m_frame)* 16, 0, x, y - 16 - (int)m_z + by, 16, 24 - by);
}


int CItem :: update(CMap *p_map, list<ISortable *> *p_list) {
	if (p_map && p_list);

	m_mapRow = ((int)m_y + 7) / 16;

	if (m_type == eBrain || m_type == eBombBrain)  {
		m_animCount ++;
		if (m_animCount == 15) {
			m_frame ++;
			m_animCount = 0;
			if (m_frame == 4) {
				m_frame = 0;
			}
		}
	}

	// store old positions
	int ox = (int)m_x;
	int oy = (int)m_y;
	int inWall = 0;

	// check if already inside a wall
	if (p_map->isWall(x() + 1, y())) inWall = 1;
	else if (p_map->isWall(x() + 1, y() + 7)) inWall = 1;
	else if (p_map->isWall(x() + 14, y())) inWall = 1;
	else if (p_map->isWall(x() + 14, y() + 7)) inWall = 1;

	// apply gravity
	m_z += m_dz;
	m_dz -= 0.5;
	if (m_z < 0) {
		m_z = 0;
		m_dz = -m_dz * 0.2;
	}

	// move it if not stuck
	if (!inWall) {
		int stop = 0;
		m_x += m_dx;
		m_y += m_dy;

		if (p_map->isWall(x() + 1, y())) stop = 1;
		else if (p_map->isWall(x() + 1, y() + 7)) stop = 1;
		else if (p_map->isWall(x() + 14, y())) stop = 1;
		else if (p_map->isWall(x() + 14, y() + 7)) stop = 1;

		if (stop) {
			setX(ox);
			setY(oy);
			m_dx = -m_dx * 0.8;
			m_dy = -m_dy * 0.8;
		}

		m_dx *= 0.8;
		m_dy *= 0.8;
	}
	else {
		m_dx = 0;
		m_dy = 0;
	}

	m_life --;

	if (m_life <= 0) return -1;
	return 0;
}


void CItem :: setFrames(BITMAP *p_bmp, int p_baseID) {
	m_frames = p_bmp;
	m_baseID = p_baseID;
}

int CItem :: collide(CActor *p_a) {
	if (m_x      > p_a->x() + 15) return 0;
	if (m_x + 15 < p_a->x()     ) return 0;
	if (m_y      > p_a->y() +  7)  return 0;
	if (m_y +  7 < p_a->y()     ) return 0;
	return 1;
}

