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

#ifndef _HISC_H_
#define _HISC_H_

#include <allegro.h>

#define MAX_HISCORES      5

typedef struct {
   char name[32];
   unsigned int score;
   unsigned int level;
} Thisc;

Thisc* make_hisc_table();
void destroy_hisc_table(Thisc *);
int qualify_hisc_table(Thisc *table, Thisc post);
void sort_hisc_table(Thisc *table);
void enter_hisc_table(Thisc *table, Thisc post);
void reset_hisc_table(Thisc *table, char *name, int hi, int lo);
int load_hisc_table(Thisc *table, PACKFILE *fp);
void save_hisc_table(Thisc *table, PACKFILE *fp);

void draw_hisc_post(Thisc *table, BITMAP *bmp, FONT *fnt, int x, int y, int color);
void draw_hisc_table(Thisc *table, BITMAP *bmp, FONT *fnt, int x, int y, int color);

#endif


