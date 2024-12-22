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



#ifndef _CACTOR_H_
#define _CACTOR_H_

#include <allegro.h>

#include "ISortable.h"
#include "CMap.h"
#include <list>

using namespace std;

class CActor : public ISortable {
	public:
		enum EActorType {
			eHuman,
			eZombie,
			ePlayer
		};

		enum EActorCommand {
			eNone = 0,
			eUp = 1,
			eDown = 2,
			eLeft = 4,
			eRight = 8,
			eStop = 16
		};

		CActor(EActorType p_type, int p_isPlayer, int p_level);
		virtual ~CActor();

		void setType(EActorType p_type) { m_type = p_type; }

		void render(BITMAP *p_bmp, int p_x, int p_y);
		void addCommand(EActorCommand p_cmd);
		void setCommand(EActorCommand p_cmd);
		int update(CMap *p_map, list<ISortable *> *p_list);
		void setFrames(BITMAP *p_bmp);
		void move(int p_x, int p_y) {
			m_x += p_x;
			m_y += p_y;
		}

		int getMapRow() { return m_mapRow; }
		CActor :: EActorCommand randomDirection();
		int isStuck(CMap *p_map);
		int collide(CActor *p_actor);

		int isZombie() { return (m_type == eZombie ? 1 : 0); }
		void setWantsToBeZombie() {
			if (!m_humanTime) {
				m_makeZombie = 1; m_makeHuman = 0;
			}
		}
		void setWantsToBeHuman() {
			m_makeZombie = 0; m_makeHuman = 1;
			m_humanTime = 50;
		}
		int wantsToBeZombie() { return m_makeZombie; }
		int wantsToBeHuman() { return m_makeHuman; }
		int getDirection() { return m_direction; }
		int *getBrainAmount() { return &m_brainAmount; }

	protected:
		EActorType m_type;
		int m_command;
		int m_isPlayer;

		// position
		double m_speed;
		double m_fx, m_fy;
		int m_mapRow;
		int m_makeZombie;
		int m_makeHuman;
		int m_seekRadius;
		int m_stillTime;
		int m_speakTime;

		// frames
		BITMAP *m_frames;
		CActor *m_zombieTarget;
		CActor *m_lastTarget;
		int m_targetX, m_targetY;
		int m_frame, m_animCount;
		int m_direction;  // 0 = dn, 1 = up, 2 = lft, 3 = rght
		int m_mayChangeDir;
		int m_level;
		int m_humanTime;
		int m_brainAmount;
		int m_lastDrop;
		int m_stuck;
		int m_bored;
		int m_walking;




};

#endif

