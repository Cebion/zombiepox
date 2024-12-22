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


#include <math.h>
#include "CActor.h"
#include "main.h"
#include "../data/data_sfx.h"

#include <list>

using namespace std;



CActor :: CActor(EActorType p_type, int p_isPlayer, int p_level) {
	m_isPlayer = p_isPlayer;
	m_type = p_type;
	m_frames = NULL;
	m_level = p_level;
	m_x = m_y = m_fx = m_fy = 0;
	m_makeZombie = 0;
	m_makeHuman = 0;
	m_zombieTarget = NULL;
	m_lastTarget = NULL;
	m_frame = m_animCount = 0;
	m_direction = 0;
	m_mayChangeDir = 0;
	m_seekRadius = 500;
	m_stillTime = 0;
	m_speakTime = 1000;
	m_humanTime = 0;
	m_brainAmount =
	m_stuck =
	m_lastDrop = 0;
	m_bored = rand()%1000;
	m_walking = 0;
}


CActor :: ~CActor() {
}


void CActor :: render(BITMAP *p_bmp, int p_ox, int p_oy) {
	int x = (int)m_x - p_ox;
	int y = (int)m_y - p_oy;

	if (m_frames) {
		// shadow
		line(p_bmp, x + 2, y + 4, x + 14, y + 4, 150);
		line(p_bmp, x + 2, y + 5, x + 14, y + 5, 150);
		line(p_bmp, x + 3, y + 6, x + 13, y + 6, 150);
		line(p_bmp, x + 3, y + 3, x + 13, y + 3, 150);
		// frame
		masked_blit(m_frames, p_bmp, (m_frame + m_direction * 4) * 16, 0, x, y - 24, 16, 32);

		// debug stuff:
		//textprintf(p_bmp, font, x, y, 1, "%d", m_stillTime);
		/*
		if (isZombie()) {
			circle(p_bmp, x, y, sqrt(m_seekRadius), 1);
		}
		if (m_zombieTarget != NULL || m_walking) {
			int tx = (int)m_targetX - p_ox;
			int ty = (int)m_targetY - p_oy;
			line(p_bmp, x, y, tx, ty, 1);
		}
		*/
	}
}


void CActor :: setCommand(EActorCommand p_cmd) {
	m_command = p_cmd;
}

void CActor :: addCommand(EActorCommand p_cmd) {
	m_command |= p_cmd;
}


CActor :: EActorCommand CActor :: randomDirection() {
	EActorCommand dir = eNone;
	int r = rand()%4;
	if (r == 0) dir = eUp;
	if (r == 1) dir = eDown;
	if (r == 2) dir = eLeft;
	if (r == 3) dir = eRight;
	return dir;
}

int CActor :: isStuck(CMap *p_map) {
	if (p_map->isWall(m_x, m_y)) return 1;
	if (p_map->isWall(m_x + 15, m_y)) return 1;
	if (p_map->isWall(m_x, m_y + 7)) return 1;
	if (p_map->isWall(m_x + 15, m_y + 7)) return 1;

	return 0;
}

int actorDist(CActor *a, CActor *b) {
	int x = (int)(a->x() - b->x());
	int y = (int)(a->y() - b->y());

	return x * x + y * y;
}


int CActor :: update(CMap *p_map, list<ISortable *> *p_list) {
	int dropBrain = 0;

	m_mapRow = ((int)m_y + 7) / 16;

	if (m_humanTime) m_humanTime --;
	if (m_lastDrop) m_lastDrop --;

	m_speed = 1;
	if (m_type == ePlayer) m_speed = 1.5;
	if (m_type == eZombie) {
		if (m_level > 9) m_speed = 2;
		else m_speed = 0.2 + (double)m_level / 10.0;
	}

	if (m_type == eHuman) {
		m_command = randomDirection();
	}

	int ox = (int)m_x, oy = (int)m_y;

	if (m_mayChangeDir) m_mayChangeDir --;

	if (m_isPlayer) {
		if (m_command & eStop) {
		}
		else {
			if (isZombie()) {
				if (m_command & eDown) { m_y -= m_speed; m_direction = 1; }
				if (m_command & eUp) { m_y += m_speed; m_direction = 0; }
				if (m_command & eRight) { m_x -= m_speed; m_direction = 2; }
				if (m_command & eLeft) { m_x += m_speed; m_direction = 3; }
			}
			else {
				if (m_command & eUp) { m_y -= m_speed; m_direction = 1; }
				if (m_command & eDown) { m_y += m_speed; m_direction = 0; }
				if (m_command & eLeft) { m_x -= m_speed; m_direction = 2; }
				if (m_command & eRight) { m_x += m_speed; m_direction = 3; }
			}
		}
	}
	else {
		m_x += m_fx;
		m_y += m_fy;

		// set direction
		int dir;
		if (ABS(m_fx) > ABS(m_fy)) {
			if (m_fx >= 0) dir = 3;
			else dir = 2;
			if (ABS(m_fx) > 0.1 && !m_mayChangeDir) {
				m_direction = dir;
				m_mayChangeDir = 10;
			}
		}
		else {
			if (m_fy >= 0) dir = 0;
			else dir = 1;
			if (ABS(m_fy) > 0.1 && !m_mayChangeDir) {
				m_direction = dir;
				m_mayChangeDir = 10;
			}
		}
		/*
		if (m_fx == 0 && m_fx == 0) {
			m_direction = 0;
		}
		*/
	}

	// handle collision
	// top left
	if (p_map->isWall(x() + 1, oy)) { m_stuck ++; setX(ox); }
	if (p_map->isWall(ox + 1, y())) { m_stuck ++; setY(oy); }
	// top right
	if (p_map->isWall(x() + 14, oy)) { m_stuck ++; setX(ox); }
	if (p_map->isWall(ox + 14, y())) { m_stuck ++; setY(oy); }
	// btm left
	if (p_map->isWall(x() + 1, oy + 7)) { m_stuck ++; setX(ox); }
	if (p_map->isWall(ox + 1, y() + 7)) { m_stuck ++; setY(oy); }
	// btm right
	if (p_map->isWall(x() + 14, oy + 7)) { m_stuck ++; setX(ox); }
	if (p_map->isWall(ox + 14, y() + 7)) { m_stuck ++; setY(oy); }

	if (ox != (int)m_x || (int)m_y != oy) {
		m_stuck = 0;
		// animate!
		m_animCount ++;
		if (m_animCount == 8) {
			m_animCount = 0;
			m_frame ++;
			if (m_frame == 4) {
				m_frame = 0;
			}
		}
		m_stillTime = 0;
		m_bored = 0;
	}
	else {
		m_stillTime ++;
		m_bored ++;
		if (m_stillTime >= 10) {
			m_zombieTarget = NULL;
			m_stillTime = 0;
			m_walking = 0;
		}

		// try to push the actor out of the object
		if (m_stuck > 2000) {
			int oox = (int)m_x;
			int ooy = (int)m_y;
			m_x -= 1.3 * m_fx;
			m_y -= 1.5 * m_fy;
			if (isStuck(p_map)) {
				m_x = oox;
				m_y = ooy;
			}

		}
	}

	if (m_bored > 500) {
		if (!m_isPlayer && !isZombie()) {
			int mw = ((p_map->getWidth()  - 4) * 16) / 2;
			int mh = ((p_map->getHeight() - 4) * 16) / 2;
			m_targetX = 32 + (int)(m_x + rand()%mw - mw/2);
			m_targetY = 32 + (int)(m_y + rand()%mh - mh/2);
			m_walking = 1;
		}
	}

	// check other actors
	for(list<ISortable *>::iterator i = p_list->begin(); i != p_list->end();  ++ i) {
		CActor *a = (CActor *)(*i);
		if (a != this) {
			if (collide(a)) {
				setX(ox);
				setY(oy);
				if (a->isZombie() && !this->isZombie()) {
					m_makeZombie = 1;
					play_sound(NAMNAM, x(), y());
				}
				else if (!a->isZombie() && this->isZombie()) {
					a->setWantsToBeZombie();
					play_sound(NAMNAM, x(), y());
				}
			}
		}
	}

	m_speakTime ++;
	if (isZombie() && m_zombieTarget == NULL) {
		int seekDist = m_seekRadius;

		for(list<ISortable *>::iterator i = p_list->begin(); i != p_list->end();  ++ i) {
			CActor *a = (CActor *)(*i);
			if (a != this) {
				if (!a->isZombie()) {
					int aDist = actorDist(a, this);
					if (aDist < seekDist && aDist > 100) {
						m_zombieTarget = a;
						seekDist = aDist;
						if (m_zombieTarget != m_lastTarget && m_stillTime < 3) {
							if (m_speakTime > 100) {
								play_sound(BRAINS, x(), y());
								m_speakTime = 0;
							}
							m_lastTarget = m_zombieTarget;
						}
					}
				}
			}
		}



		// increase seek radius if still no target
		if (m_zombieTarget == NULL) {
			m_seekRadius = MIN(m_seekRadius + 1000, 10000000);
		}
	}

	// always check if player is near
	CActor *pa = (CActor *)*(p_list->begin());
	if (isZombie() && !pa->isZombie() && rand()%100 > 50) {
		if (actorDist(pa, this) < 600) {
			m_zombieTarget = pa;
			if (m_zombieTarget != m_lastTarget) {
				if (m_speakTime > 100) {
					play_sound(BRAINS, x(), y());
					m_speakTime = 0;
				}
				m_lastTarget = m_zombieTarget;
			}
		}
	}

	m_fx = m_fy = 0;
	if (isZombie() && m_zombieTarget != NULL) {
		int dx = m_zombieTarget->x() - (int)m_x;
		int dy = m_zombieTarget->y() - (int)m_y;
		int dist = (int)(sqrt(dx * dx + dy * dy) / 10.0);
		m_targetX = m_zombieTarget->x() + fixtoi(dist * fcos(itofix(game_count * 8)));
		m_targetY = m_zombieTarget->y() + fixtoi(dist * fsin(itofix(game_count * 8)));
		fixed angle = fatan2(ftofix(m_targetY - m_y), ftofix(m_targetX - m_x));
		m_fx = m_speed * fixtof(fcos(angle));
		m_fy = m_speed * fixtof(fsin(angle));

		if (m_zombieTarget->isZombie()) {
			m_zombieTarget = NULL;
			m_seekRadius = 400;
		}
	}

	if (!isZombie() && m_walking) {
		fixed angle = fatan2(ftofix(m_targetY - m_y), ftofix(m_targetX - m_x));
		m_fx = m_speed * fixtof(fcos(angle)) / 2;
		m_fy = m_speed * fixtof(fsin(angle)) / 2;

		if (ABS(m_x - m_targetX) < 32 && ABS(m_y - m_targetY) < 32) {
			m_walking = 0;
			m_bored = 0;
		}
	}

	if (m_type != ePlayer) {
		// look for closest actor and try to avoid it
		int closest = 1000;
		CActor *aid = NULL;
		for(list<ISortable *>::iterator i = p_list->begin(); i != p_list->end();  ++ i) {
			CActor *a = (CActor *)(*i);
			if (a != this) {
				int x = (int)(a->x() - m_x);
				int y = (int)(a->y() - m_y);
				int dist = x*x + y*y;
				if (dist < closest) {
					aid = (CActor *)*i;
					closest = dist;
				}
			}
		}

		if (aid != NULL) {
			double mod = 0;
			if (m_type == eHuman) mod = 1;
			else if (m_type == eZombie && aid->isZombie()) mod = 1.1;
			double tx = aid->x();
			double ty=  aid->y();
			fixed angle = fatan2(ftofix(ty - m_y), ftofix(tx - m_x));
			m_fx += -m_speed * fixtof(fcos(angle)) * mod;
			m_fy += -m_speed * fixtof(fsin(angle)) * mod;

			if (aid->isZombie() && m_type == eHuman && m_brainAmount && !m_lastDrop) {
				m_brainAmount --;
				m_lastDrop = 100;
				dropBrain = (m_level < 8 ? 1 : 2);
			}
		}
		else if (m_type == eHuman && !m_walking) {
			m_fx = m_fy = 0;
		}
	}

	return dropBrain;
}


int CActor :: collide(CActor *p_a) {
	if (m_x - 1> p_a->x() + 16) return 0;
	if (m_x + 16 < p_a->x() - 1) return 0;
	if (m_y - 1> p_a->y() + 8) return 0;
	if (m_y +  8 < p_a->y() - 1) return 0;
	return 1;
}


void CActor :: setFrames(BITMAP *p_bmp) {
	m_frames = p_bmp;
}



