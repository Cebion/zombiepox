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


#ifndef _MAIN_H_
#define _MAIN_H_

extern volatile int game_count;

void play_sound(int id, int x, int y);
void log(char *format, ...);

#endif
