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

#include "CTile.h"


CTile :: CTile(BITMAP *p_image, int p_id, int p_isWall) {
	m_image = p_image;
	m_imgID = p_id;
	m_isWall = p_isWall;
}


void CTile :: render(BITMAP *p_bmp, int p_x, int p_y, int p_trans) {
	if (m_isWall) {
		if (p_trans) {
			BITMAP *b = create_bitmap(16, 48);
			blit(m_image, b, m_imgID*16, 0, 0, 0, 16, 48);
			draw_trans_sprite(p_bmp, b, p_x, p_y - 32);
			destroy_bitmap(b);
		}
		else {
			masked_blit(m_image, p_bmp, m_imgID*16, 0, p_x, p_y - 32, 16, 48);
		}
	}
	else {
		masked_blit(m_image, p_bmp, m_imgID*16, 0, p_x, p_y - 8, 16, 24);
	}
}

