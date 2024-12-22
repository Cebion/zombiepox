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




#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include <list>

#include "main.h"
#include "CMap.h"
#include "CActor.h"
#include "CItem.h"
#include "hisc.h"
#include "soundbox.h"

#include "../data/data_gfx.h"
#include "../data/data_sfx.h"


#define LOG_FILE_NAME	"log.txt"
#define GAME_NAME		"Zombiepox"
#define GAME_VERSION	"1.1"

// game states
#define GS_TITLE			 1
#define GS_GOTO_TITLE		10
#define GS_GOTO_SCORES		11
#define GS_PLAY				 2
#define GS_HELP				 3
#define GS_SCORES   		 4
#define GS_QUIT				 5


using namespace std;

// mm.... globals!
BITMAP *swap_screen = NULL;
BITMAP *bar_screen = NULL;
DATAFILE *gfx = NULL;
DATAFILE *sfx = NULL;
int game_state;
int sound_volume = 150;
int window = 1;
int init_ok = 0;
int game_is_paused = 0;
int show_map;

// actors
list<ISortable *> actors;
list<ISortable *> items;
int zombie_count;
int actor_count;

// player stuff
CActor *player = NULL;
int brain_time;
int brain_level;
int game_over = 0;
int level_complete = 0;
int show_level_name = 0;
int score;
int level;
int bonus;
int tick_count;
int ok_to_cheat = 0;
int i_am_a_cheat;

// blend stuff
COLOR_MAP dark_map, trans_map;

// the game map
CMap *gameMap = NULL;

// sound intervals
int sound_box[10];

// level names
char *levelNames[] = { "BRAAAINS!",
				"6 p.m.",
				"7 p.m.",
				"8 p.m.",
				"9 p.m.",
				"10 p.m.",
				"11 p.m.",
				"midnight",
				"moon shadow",
				"It ends tonight",
				"Wow, you're good."
			  };

// scores
Thisc *hiscores = NULL;

// timer stuff
volatile int frame_count;
volatile int fps;
volatile int logic_count;
volatile int lps;
volatile int cycle_count;
volatile int game_count;

typedef struct {
	int cursor_pos;
	int menu_level;
} T_title_data;
T_title_data title_data;


// some function declarations
void new_level();
void new_game();
void draw_game();
void update_game();


// keeps track of frames each second
void fps_counter(void) {
	fps = frame_count;
	frame_count = 0;
	lps = logic_count;
	logic_count = 0;
}
END_OF_FUNCTION(fps_counter);


// keeps track of internal game speed
void cycle_counter(void) {
	cycle_count++;
	game_count++;
}
END_OF_FUNCTION(game_counter);


// writes a string of formated text to the log file
void log(char *format, ...) {
	va_list ptr;
 	FILE *fp;

	fp = fopen(LOG_FILE_NAME, "at");
	if (fp) {
		va_start(ptr, format);
		vfprintf(fp, format, ptr);
		fprintf(fp, "\n");
		va_end(ptr);

		fclose(fp);
	}
}


void play_sound(int id, int x, int y) {
	if (sound_volume == 0) return;

	// don't play sound to close to last (same sound)
	if (sound_box[id] > 0 && game_state == GS_PLAY) return;

	// remember sound
	sound_box[id] += 30;

	if (player == NULL) {
		play_sample((SAMPLE *)sfx[id].dat, sound_volume, 128, 900 + rand()%200, 0);
		return;
	}

	int dx = x - player->x();
	int dy = y - player->y();
	int dist = dx * dx + dy * dy;
	if (dist > 64000) return;
	int vol = (int)(sound_volume * ((64000.0 - (double)dist) / 64000.0));
	int pan = 128;
	if (ABS(dx) < 0) {
		if (dx < -200) pan = 0;
		else pan = 128 * (-dx) / 200;
	}
	else {
		if (dx > 200) pan = 255;
		else pan = 127 + 128 * dx / 200;
	}
	play_sample((SAMPLE *)sfx[id].dat, vol, pan, 900 + rand()%200, 0);
}


// blits a bitmap to screen
void blit_to_screen(BITMAP *bmp) {
	//textprintf(bmp, font, 0, 0, 2, "%d/%d", lps, fps);

	acquire_screen();
	stretch_blit(bmp, screen, 0, 0, bmp->w, bmp->h, 0, 0, SCREEN_W, SCREEN_H);
	release_screen();
}


//
void progress(int p) {
	static int prog = 0;
	if (p);

	line(screen, 0, prog, 639, prog, 1);
	line(screen, 0, prog+1, 638, prog+1, 1);
	prog += 2;
}


//
int toggle_gfx() {
	window = !window;
    if (set_gfx_mode((window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN), 640, 480, 0, 0)) {
		log("  *** failed");
		allegro_message("%s:\nFailed to enter gfx mode.", GAME_NAME);
		set_palette((RGB *)gfx[0].dat);
		return FALSE;
	}
	set_display_switch_mode(SWITCH_BACKGROUND);
	set_palette((RGB *)gfx[0].dat);
	return TRUE;
}


// load
int load_game_data() {
	int ok = 1;
	PACKFILE *fp = pack_fopen("data/zombiepox.dat", "rb");
	if (fp) {
		pack_fread(&window, sizeof(window), fp);
		pack_fread(&sound_volume, sizeof(sound_volume), fp);
		pack_fread(&music_vol, sizeof(music_vol), fp);
		if (!load_hisc_table(hiscores, fp)) {
			ok = 0;
		}

		pack_fclose(fp);
	}
	else {
		ok = 0;
	}

	return ok;
}

// save
void save_game_data() {
	PACKFILE *fp = pack_fopen("data/zombiepox.dat", "wb");
	if (fp) {
		pack_fwrite(&window, sizeof(window), fp);
		pack_fwrite(&sound_volume, sizeof(sound_volume), fp);
		pack_fwrite(&music_vol, sizeof(music_vol), fp);
		save_hisc_table(hiscores, fp);

		pack_fclose(fp);
	}
}


// init the game
int init_game() {
	// start new logfile
	FILE *fp = fopen(LOG_FILE_NAME, "wt");
	fclose(fp);
	log("%s v%s log file\n---------------------------------------------\n", GAME_NAME, GAME_VERSION);

	log("\nInit game:");

	// start allegro
	log(" initializing allegro");
	allegro_init();
	text_mode(-1);

	// install timers
	log(" setting up timers");
	install_timer();
	LOCK_VARIABLE(cycle_count);
	LOCK_VARIABLE(logic_count);
	LOCK_VARIABLE(lps);
	LOCK_VARIABLE(fps);
	LOCK_VARIABLE(frame_count);
	srand(time(NULL));

	LOCK_FUNCTION(fps_counter);
	install_int(fps_counter, 1000);
	fps = 0;
	frame_count = 0;
	cycle_count = 0;

	LOCK_FUNCTION(cycle_counter);
	install_int(cycle_counter, 20);
	game_count = 0;


	log(" setting color depth (8)");
	set_color_depth(8);

	// init music and sound
	// install sound
	log(" installing sound");
	reserve_voices(64, 0);
	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL)) {
		log("  *** failed");
	}
	init_soundbox();

	// allocating memory
	log(" allocating memory buffers");
	swap_screen = create_bitmap(320, 240);
	bar_screen = create_bitmap(320, 30);
	if (swap_screen == NULL) {
		log("  *** failed");
		allegro_message("%s:\nFailed to allocate memory for buffers.", GAME_NAME);
		return FALSE;
	}
	set_window_close_button(FALSE);
	set_window_title(GAME_NAME);

	// creating hisc
	log(" creating hisc table");
	hiscores = make_hisc_table();

	// load game data
	log(" loading game data");
	if (!load_game_data()) {
		log("  *** failed, resetting to defaults");
		sound_volume = 150;
		window = 1;
		reset_hisc_table(hiscores, "Johan", 0, 0);
	}


	log(" entering gfx mode");

    if (set_gfx_mode((window ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN), 640, 480, 0, 0)) {
		log("  *** failed");
		allegro_message("%s:\nFailed to enter gfx mode.", GAME_NAME);
		return FALSE;
	}
	set_display_switch_mode(SWITCH_BACKGROUND);

	clear(swap_screen);
	textout_centre(swap_screen, font, "please wait...", 160, 200, 1);
	blit_to_screen(swap_screen);

	// load data
	log(" loading data");
	packfile_password("piracy_is_theft");
	gfx = load_datafile("data/gfx.dat");
	sfx = load_datafile("data/sfx.dat");
	packfile_password(NULL);
	if (gfx == NULL || sfx == NULL) {
    	log("  *** failed");
		allegro_message("%s:\nFailed to load data.", GAME_NAME);
		return FALSE;
	}
	// change color 0 to black in palette
	((RGB *)gfx[0].dat)[0].r =
	((RGB *)gfx[0].dat)[0].g =
	((RGB *)gfx[0].dat)[0].b =
	((RGB *)gfx[DARKPAL].dat)[0].r =
	((RGB *)gfx[DARKPAL].dat)[0].g =
	((RGB *)gfx[DARKPAL].dat)[0].b = 0;
	for(int i = 0; i < 64; i ++) {
		((RGB *)gfx[0].dat)[64 + i].r = ((RGB *)gfx[0].dat)[i].r / 2;
		((RGB *)gfx[0].dat)[64 + i].g = ((RGB *)gfx[0].dat)[i].g / 2;
		((RGB *)gfx[0].dat)[64 + i].b = ((RGB *)gfx[0].dat)[i].b / 2;
		((RGB *)gfx[DARKPAL].dat)[64 + i].r = ((RGB *)gfx[DARKPAL].dat)[i].r / 2;
		((RGB *)gfx[DARKPAL].dat)[64 + i].g = ((RGB *)gfx[DARKPAL].dat)[i].g / 2;
		((RGB *)gfx[DARKPAL].dat)[64 + i].b = ((RGB *)gfx[DARKPAL].dat)[i].b / 2;
	}
	set_palette((RGB *)gfx[0].dat);
	create_light_table(&dark_map, (RGB *)gfx[0].dat, 0, 0, 0, progress);
	create_trans_table(&trans_map, (RGB *)gfx[0].dat, 180, 180, 180, progress);

	// install some parts of allegro
	log(" installing keyboard");
	install_keyboard();

	// sounds
	for(int i = 0; i < 10; i ++) {
		sound_box[i] = 0;
	}

	// misc
	title_data.cursor_pos = 0;
	title_data.menu_level = 0;

	log(" init OK!");
	init_ok = 1;

	return TRUE;
}


// uninits the game
void uninit_game() {
	log("\nExit game:");

	log(" destroying bitmaps");
	if (swap_screen != NULL) destroy_bitmap(swap_screen);
	if (bar_screen != NULL) destroy_bitmap(bar_screen);

	log(" unloading graphics");
	if (gfx != NULL) unload_datafile(gfx);

	log(" unloading sounds");
	if (sfx != NULL) unload_datafile(sfx);

	if (init_ok) {
		log(" saving game data");
		save_game_data();
	}

	log(" destroy hisc table");
	if (hiscores != NULL) destroy_hisc_table(hiscores);

	log(" exiting allegro");
	allegro_exit();
}


void textoutline(BITMAP *p_bmp, FONT *p_font, char *p_txt, int x, int y, int color) {
	textout(p_bmp, p_font, p_txt, x-1, y, color);
	textout(p_bmp, p_font, p_txt, x, y+1, color);
	textout(p_bmp, p_font, p_txt, x+1, y, color);
	textout(p_bmp, p_font, p_txt, x, y-1, color);
	textout(p_bmp, p_font, p_txt, x, y, -1);
}


void textwave(BITMAP *p_bmp, FONT *p_font, char *p_txt, int x, int y, int amp, int center) {
	int tx = (center ? x - text_length(p_font, p_txt)/2 : x);
	char buf[2] = { 0, 0 };

	for(unsigned int i = 0; i < strlen(p_txt); i ++) {
		buf[0] = p_txt[i];
		int a = fixtoi(fcos(itofix((game_count + (i<<2))<<2)) * (amp) + fcos(itofix((game_count + (i<<3))<<1)) * (amp/2));
		textout(p_bmp, p_font, buf, tx, y + a, -1);
		tx += text_length(p_font, buf);
	}
}



CActor *spawn_actor(int x, int y, CActor :: EActorType p_type) {
	CActor *a = new CActor(p_type, 0, level);
	a->setX(x); a->setY(y);
	switch(p_type) {
		case CActor :: eZombie: {
			a->setFrames((BITMAP *)gfx[ZOMBIE].dat);
			break;
		}
		default: {
			a->setFrames((BITMAP *)gfx[HUMAN].dat);
			break;
		}
	}
	return a;
}


int actorStuck(CActor *a) {
	for(list<ISortable *>::iterator i = actors.begin(); i != actors.end(); ++ i) {
		if (*i != a) {
			if (a->collide((CActor *)(*i))) return 1;
		}
	}
	return 0;
}


void draw_map() {
	int x, y;
	int mx = 2, my = 2;
	BITMAP *map = create_bitmap(2 * gameMap->getWidth(), 2 * gameMap->getHeight());
	clear_to_color(map, 6);

	for(x = 0; x < gameMap->getWidth(); x ++) {
		for(y = 0; y < gameMap->getHeight(); y ++) {
			int c = (gameMap->isWall(x*16, y*16) ? 2 : 5);
			rect(map, x * 2, y * 2, x * 2 + 1, y * 2 + 1, c);
		}
	}

	color_map = &trans_map;
	draw_trans_sprite(swap_screen, map, mx, my);
	clear(map);

	// draw actors
	for(list<ISortable *>::iterator i = actors.begin(); i != actors.end();  ++ i) {
		CActor *a = (CActor *)(*i);
		int ax = 2 * (a->x() + 8) / 16;
		int ay = 2 * (a->y() + 8) / 16;
		int c = (a->isZombie() ? 27 : 21);
		if (a == player) c = 24;
		rect(map, ax - 2, ay - 1, ax + 1, ay, c);
		rect(map, ax - 1, ay - 2, ax, ay + 1, c);
	}
	// draw items
	for(list<ISortable *>::iterator i = items.begin(); i != items.end();  ++ i) {
		CItem *it = (CItem *)(*i);
		int ix = 2 * (it->x() + 8) / 16;
		int iy = 2 * (it->y() + 8) / 16;
		int c = (it->getType() == CItem :: eDynamite ? 37 : 39);

		rect(map, ix, iy, ix + 1, iy + 1, c);
	}

	draw_sprite(swap_screen, map, mx, my);

	destroy_bitmap(map);
}



void draw_game() {
	int y;
	// camera
	int cx = MAX(MIN(player->x() - 160, gameMap->getWidth() * 16 - 320), 0);
	int cy = MAX(MIN(player->y() - 140, gameMap->getHeight() * 16 - 240), -16);

	// put all sortables in one list
	list<ISortable *> stuff;
	for(list<ISortable *>::iterator i = actors.begin(); i != actors.end();  ++ i) {
		stuff.push_back(*i);
	}
	for(list<ISortable *>::iterator i = items.begin(); i != items.end();  ++ i) {
		stuff.push_back(*i);
	}
	// sort it
	stuff.sort(ByYPos());


	// draw map
	clear_bitmap(swap_screen);
	color_map = &dark_map;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	for(y = 0; y < 18; y ++) {
		// draw map line
		color_map = &trans_map;
		gameMap->render(swap_screen, cx, cy, y);

		int ty = y + cy / 16;

		color_map = &dark_map;
		for(list<ISortable *>::iterator i = stuff.begin(); i != stuff.end();  ++ i) {
			if ((*i)->getMapRow() == ty) {
				(*i)->render(swap_screen, cx, cy);
			}
		}

	}

	solid_mode();

	clear(bar_screen);
	if (!show_map) textout(bar_screen, (FONT *)gfx[FONT_WHITE].dat, "Zombiepox", 4, -1, -1);
	textprintf_centre(bar_screen, (FONT *)gfx[FONT_WHITE].dat, 160, -1, -1, "Score: %d", score);
	textprintf_right(bar_screen, (FONT *)gfx[FONT_WHITE].dat, 316, -1, -1, "Level: %d", level);
	draw_character(swap_screen, bar_screen, -1, 0, 0);
	draw_character(swap_screen, bar_screen, 1, 0, 0);
	draw_character(swap_screen, bar_screen, 0, 1, 0);
	draw_character(swap_screen, bar_screen, 0, -1, 0);
	draw_sprite(swap_screen, bar_screen, 0, 0);

	// draw bars
	if (actor_count > 0) {
		int zw = (100 * zombie_count) / actor_count;
		stretch_sprite(swap_screen, (BITMAP *)gfx[BAR_GREEN].dat, 192, 228, 100, 8);
		stretch_sprite(swap_screen, (BITMAP *)gfx[BAR_RED].dat, 292 - zw, 228, zw, 8);
	}
	draw_sprite(swap_screen, (BITMAP *)gfx[ZOMBIE_BAR].dat, 165, 210);

	stretch_sprite(swap_screen, (BITMAP *)gfx[BAR_RED].dat, 32, 228, 100, 8);
	stretch_sprite(swap_screen, (BITMAP *)gfx[BAR_GREEN].dat, 32, 228, 100 - brain_time, 8);
	draw_sprite(swap_screen, (BITMAP *)gfx[BRAIN_BAR].dat, 1, 216);

	draw_sprite(swap_screen, (BITMAP *)gfx[DYNA_BAR].dat, 1, 176);
	char buf[32];
	sprintf(buf, "%d", brain_level);
	textoutline(swap_screen, (FONT *)gfx[FONT_WHITE].dat, buf, 8, 190, 6);

	if (show_map) draw_map();

	// draw various information
	if (game_is_paused) {
		color_map = &dark_map;
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		int gh = MIN(game_is_paused * 4, 30);
		rectfill(swap_screen, 0, 120 - gh, 319, 120 + gh, 100);
		solid_mode();
		textout_centre(swap_screen, (FONT *)gfx[FONT_WHITE].dat, "GAME PAUSED", 160, 100, -1);
		textout_centre(swap_screen, (FONT *)gfx[FONT_WHITE].dat, "ESC TO QUIT - ANY KEY TO CONTINUE", 160, 120, -1);
	}
	if (game_over) {
		color_map = &dark_map;
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		int gh = MIN(game_over / 2, 30);
		int y = 120 - (game_over < 200 ? 0 : MIN(game_over - 200, 90));
		rectfill(swap_screen, 0, y - gh, 319, y + gh, 100);
		solid_mode();
		textwave(swap_screen, (FONT *)gfx[FONT_WHITE].dat, "GAME OVER", 160, y - 10, 8, 1);
	}
	else if (level_complete) {
		color_map = &dark_map;
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		int gh = MIN(level_complete / 2, 30);
		rectfill(swap_screen, 0, 120 - gh, 319, 120 + gh, 100);
		solid_mode();
		textwave(swap_screen, (FONT *)gfx[FONT_WHITE].dat, "LEVEL COMPLETE", 160, 110, 8, 1);
	} else if (show_level_name) {
		color_map = &dark_map;
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		int gh = MIN(show_level_name / 2, 30);
		rectfill(swap_screen, 0, 120 - gh, 319, 120 + gh, 100);
		solid_mode();
		textwave(swap_screen, (FONT *)gfx[FONT_WHITE].dat, levelNames[level - 1], 160, 110, 8, 1);
	}
}



void clean_up_game() {
	for(list<ISortable *>::iterator i = actors.begin(); i != actors.end();  ++ i) {
		delete (*i);
	}
	actors.clear();

	for(list<ISortable *>::iterator i = items.begin(); i != items.end();  ++ i) {
		delete (*i);
	}
	items.clear();

	player = NULL;

	delete gameMap;
	gameMap = NULL;
}



// gets a string from the user
int get_string(BITMAP *bmp, char *string, int max_size, FONT *f, int pos_x, int pos_y, int colour) {
    int i = 0, c;
    BITMAP *block = create_bitmap(320, text_height(f) + 2);
	char letters[] = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!?";
	if (pos_x);

    if (block == NULL) {
		strncpy(string, "something went wrong", 32);
		return 1;
	}

	blit(bmp, block, 0, pos_y, 0, 0, block->w, block->h);

    clear_keybuf();
    while(1) {
		cycle_count = 0;
        string[i] = '_';
		string[i + 1] = '\0';
        blit(block, bmp, 0, 0, 0, pos_y, block->w, block->h);
        textout_centre(bmp, f, string, 160, pos_y, colour);
		blit_to_screen(bmp);

		// update music
		poll_music();

		// get input
		if (keypressed()) {
			c = readkey();
			switch((c >> 8)) {
	            case KEY_BACKSPACE :
					i--;
					i = (i < 0)?0 :i;
					break;

				case KEY_ENTER :
					string[i] = '\0';
					destroy_bitmap(block);
					return 0;
					break;

				default :
	                if (i < max_size - 2 && strchr(letters, c & 0xff)) {
						string[i] = c & 0xff;
						i++;
					}
					break;
			}
		}

		while(!cycle_count) yield_timeslice();

    }
}



void update_game() {
	int check_game_over = 1;
	int esced = 0;

	if (show_level_name) {
		show_level_name --;
		if (key[KEY_SPACE]) show_level_name = 0;
		return;
	}

	// sounds
	for(int i = 0; i < 10; i ++) {
		if (sound_box[i] > 0) sound_box[i] --;
	}

	if (game_is_paused) {
		if (game_is_paused < 100) game_is_paused ++;
		cycle_count = 0;
		if (key[KEY_ESC]) {
			while(key[KEY_ESC]) poll_music();
			clean_up_game();
			game_state = GS_GOTO_TITLE;
			esced = 1;
			stop_music();
			fade_out(16);
			return;
		}
		else if (keypressed()) {
			// restore upper music
			set_music_volume(music_vol);
			game_is_paused = 0;
		}
		else return;
	}

	// toggle map on/off
	// THIS CODE MAKES THE MAP TOGGLABLE
	/*
	{
		static int m_ok = 1;
		if (key[KEY_M] && m_ok) {
			m_ok = 0;
			show_map = !show_map;
		}
		if (!key[KEY_M]) m_ok = 1;
	}
	*/
	// THIS CODE MAKES THE USER HOLD DOWN 'M'
	show_map = key[KEY_M];

	// get player input
	player->setCommand(CActor :: eNone);
	if (key[KEY_LEFT]) player->addCommand(CActor :: eLeft);
	if (key[KEY_RIGHT]) player->addCommand(CActor :: eRight);
	if (key[KEY_DOWN])  player->addCommand(CActor :: eDown);
	if (key[KEY_UP]) player->addCommand(CActor :: eUp);
	if (key[KEY_SPACE] && !brain_time && !player->isZombie() && game_over < 50 && level_complete < 50) {
		CItem :: EItemType type = (brain_level < 1 ? CItem :: eBrain : CItem :: eBombBrain);
		CItem *bi = new CItem(type);
		bi->setFrames((BITMAP *)gfx[ITEMS].dat, (type == CItem :: eBrain ? 0 : 4));
		if (brain_level) brain_level --;
		int x = player->x();
		int y = player->y();
		int dx = 0, dy = 0;
		int speed = 4 + (level / 2);
		if (player->getDirection() == 0) dy += speed;
		if (player->getDirection() == 1) dy -= speed;
		if (player->getDirection() == 2) dx -= speed;
		if (player->getDirection() == 3) dx += speed;
		bi->setY(y);
		bi->setX(x);
		bi->setZ(10);
		bi->setSpeed(dx, dy, -2);
		brain_time = MAX(100 - 5 * level, 50);
		play_sound(DROP, player->x(), player->y());
		items.push_back(bi);
	}

	// here be cheats
	if (key[KEY_LSHIFT] && ok_to_cheat) {
		if (key[KEY_PLUS_PAD]) {
			if (level < 7) {
				level ++;
				i_am_a_cheat = 1;
			}
			while(key[KEY_PLUS_PAD]) poll_music();
		}
		if (key[KEY_MINUS_PAD]) {
			if (level > 1) {
				level --;
				i_am_a_cheat = 1;
			}
			while(key[KEY_MINUS_PAD]) poll_music();
		}
		if (key[KEY_ASTERISK] && level < 7) {
			brain_level ++;
			i_am_a_cheat = 1;
			while(key[KEY_ASTERISK]) poll_music();
		}
	}
	// end cheats

	if ((key[KEY_ESC] && !level_complete) || (key[KEY_SPACE] && game_over > 50)) {
		if (game_is_paused || game_over > 50) {
			while(key[KEY_ESC] || key[KEY_SPACE]) poll_music();
			if (!game_over) game_over = 1;
			else {
				clean_up_game();
				game_state = GS_GOTO_TITLE;
			}
			game_is_paused = 0;
		}
		else {
			game_is_paused = 1;
			// lower music
			set_music_volume(music_vol / 4);
			while(key[KEY_ESC]) poll_music();
			clear_keybuf();
		}
	}
	if (key[KEY_SPACE] && level_complete > 50 && bonus == 0) {
		while(key[KEY_SPACE]) poll_music();
		stop_music();
		clean_up_game();
		fade_out(16);
		new_level();
		return;
	}
	if (brain_time) brain_time = MAX(0, brain_time - 2);

	actor_count = 0;
	zombie_count = 0;
	list<ISortable *> newBrains;
	for(list<ISortable *>::iterator i = actors.begin(); i != actors.end();  ++ i) {
		CActor *a = (CActor *)(*i);
		actor_count ++;
		int dropBrain = a->update(gameMap, &actors);
		if (dropBrain && !game_over) {
			CItem *itNew = new CItem((dropBrain == 1 ? CItem :: eBrain : CItem :: eBombBrain));
			itNew->setFrames((BITMAP *)gfx[ITEMS].dat, (dropBrain == 1 ? 0 : 4));
			itNew->setX(a->x());
			itNew->setY(a->y());
			itNew->setZ(10);
			newBrains.push_back(itNew);
			play_sound(DROP, a->x(), a->y());
		}

		if (a->wantsToBeZombie()) {
			a->setType(CActor :: eZombie);
			a->setFrames((BITMAP *)gfx[(a == player ? ZOMBIE2 : ZOMBIE)].dat);
		}

		if (a->wantsToBeHuman() && !game_over) {
			if (a == player) {
				a->setType(CActor :: ePlayer);
				a->setFrames((BITMAP *)gfx[PLAYER].dat);
			}
			else {
				a->setType(CActor :: eHuman);
				a->setFrames((BITMAP *)gfx[HUMAN].dat);
			}
		}

		if (a->isZombie() || a == player || 1) {
			if (a->isZombie()) zombie_count ++;

			// collision w/ items?
			for(list<ISortable *>::iterator iit = items.begin(); iit != items.end(); ) {
				CItem *it = (CItem *)(*iit);
				if (it->collide(a)) {
					if (it->getType() == CItem :: eDynamite) {
						if (a == player) {
							play_sound(YEAH, a->x(), a->y());
							brain_level += 2;
							if (brain_level > 99) brain_level = 99;
							delete (*iit);
							iit = items.erase(iit);
						}
						else {
							++ iit;
						}
					}
					else if (it->getType() == CItem :: eBombBrain && !a->isZombie() && a != player) {
						// human pick up bomb brain
						play_sound(YEAH, a->x(), a->y());
						int *ba = a->getBrainAmount();
						(*ba) += (level < 8 ? 2 : 1);
						CItem *itNew = new CItem(CItem :: eBrain);
						itNew->setFrames((BITMAP *)gfx[ITEMS].dat, 0);
						fixed angle = itofix(rand()%256);
						itNew->setX(a->x());
						itNew->setY(a->y());
						itNew->setZ(0);
						itNew->setSpeed(fixtof(5 * fcos(angle)), fixtof(5 * fsin(angle)), 3);
						newBrains.push_back(itNew);
						play_sound(DROP, a->x(), a->y());
						delete (*iit);
						iit = items.erase(iit);
					}
					else if (a->isZombie() && !a->wantsToBeHuman()) {
						if (it->getType() == CItem :: eBombBrain) {
							// make new brains
							for(int bb = 0; bb < 3; bb ++) {
								CItem *itNew = new CItem(CItem :: eBrain);
								itNew->setFrames((BITMAP *)gfx[ITEMS].dat, 0);
								fixed angle = itofix(rand()%256);
								itNew->setX(it->x());
								itNew->setY(it->y());
								itNew->setZ(0);
								itNew->setSpeed(fixtof(5 * fcos(angle)), fixtof(5 * fsin(angle)), 3);
								newBrains.push_back(itNew);
								play_sound(DROP, it->x(), it->y());
							}
						}
						score += 100 * level;
						a->setWantsToBeHuman();
						play_sound(YEAH, a->x(), a->y());
						check_game_over = 0;
						delete (*iit);
						iit = items.erase(iit);
						if (level >= 4 && rand()%100 > (75 - 5 * (level - 3)) && a != player) {
							// drop dymanite box
							CItem *itNew = new CItem(CItem :: eDynamite);
							itNew->setFrames((BITMAP *)gfx[ITEMS].dat, 8);
							fixed angle = itofix(rand()%256);
							itNew->setX(it->x());
							itNew->setY(it->y());
							itNew->setZ(0);
							itNew->setSpeed(fixtof(5 * fcos(angle)), fixtof(5 * fsin(angle)), 3);
							newBrains.push_back(itNew);
						}
					}
					else {
						++ iit;
					}
				}
				else {
					++ iit;
				}
			}
			// insert new brains
			for(list<ISortable *>::iterator iit = newBrains.begin(); iit != newBrains.end(); ++ iit) {
				items.push_back(*iit);
			}
			newBrains.clear();
		}
	}

	int brain_count = 0;
	for(list<ISortable *>::iterator iit = items.begin(); iit != items.end(); ) {
		CItem *it = (CItem *)(*iit);
		if (it->getType() != CItem :: eDynamite) brain_count ++;
		if (it->update(gameMap, NULL) < 0) {
			delete (*iit);
			iit = items.erase(iit);
		}
		else {
			iit ++;
		}
	}

	if (level_complete && bonus) {
		if (bonus) {
			bonus --;
		}
	}

	if (!game_over && !level_complete) {
		tick_count ++;
		if (tick_count > 50 && !player->isZombie()) {
			score += level;
			// if (bonus > 0) bonus --;
			tick_count = 0;
		}
		if (check_game_over) {
			if (brain_count == 0 && player->isZombie()) game_over = 1;
		}
		if (zombie_count == 0) level_complete = 1;
	}
	else {
		if (game_over == 1) play_sound(GAMEOVER, player->x(), player->y());
		if (level_complete == 1) play_sound(LEVELCOMPLETE, player->x(), player->y());
		if (game_over) {
			game_over ++;

			double vol = MAX(0, music_vol - game_over * 0.005);
			if (vol > 0) set_music_volume(vol);
			else stop_music();
		}
		if (level_complete) {
			level_complete ++;
			double vol = MAX(0, music_vol - level_complete * 0.005);
			if (vol > 0) set_music_volume(vol);
			else stop_music();
		}
	}

	if (game_state == GS_GOTO_TITLE) {
		//change music
		stop_music();
		// game over
		// check for hi scores
		Thisc post;
		sprintf(post.name, "dummy %02d", rand()%100);
		post.score = score;
		post.level = level;
		if (qualify_hisc_table(hiscores, post) && !esced && !i_am_a_cheat) {
			play_sound(HIGHSCORE, 0, 0);
			rectfill(swap_screen, 0, 120 - 29, 319, 120 + 29, 0);
			textout_centre(swap_screen, (FONT *)gfx[FONT_WHITE].dat, "HIGH SCORE! ENTER YOUR NAME:", 160, 100, -1);
			get_string(swap_screen, post.name, 16, (FONT *)gfx[FONT_WHITE].dat, 0, 120, -1);
			enter_hisc_table(hiscores, post);
			sort_hisc_table(hiscores);
			game_state = GS_GOTO_SCORES;
		}

		fade_out(16);
	}
}

void new_level() {
	int i, w, h, nz, nh;
	int mapOK = 0;

	log("\nStarting level (%d).", level + 1);

	// set up some data for zombie and human count
	level = MIN(level + 1, 11);
	nz = level * 3;
	nh = 5 + level * 4;
	// and level size
	w = 15 + level * 5;
	h = 10 + level * 5;

	log("  creating player");
	player = new CActor(CActor :: ePlayer, 1, level);
	actors.push_back(player);
	player->move(100, 100);
	player->setFrames((BITMAP *)gfx[PLAYER].dat);
	brain_time = 100;
	level_complete = 0;
	tick_count = 0;
	brain_level = 0;
	bonus = 25;

	while(!mapOK) {
		log("  creating map");
		gameMap = new CMap(w, h, (BITMAP *)gfx[TILES].dat, (BITMAP *)gfx[WALLS].dat, level);
		gameMap->flatSpace((BITMAP *)gfx[TILES].dat, player->x(), player->y());
		if (gameMap->hasPockets()) {
			log("    * map has pockets, must be recreated");
			delete gameMap;
		}
		else {
			mapOK = 1;
		}
	}

	log("  creating zombies");
	for(i = 0; i < nz; i ++) {
		CActor *a = spawn_actor(0, 0, CActor :: eZombie);
		while(a->isStuck(gameMap) || actorStuck(a)) {
			int x = (rand()%(w/2) + w/2) * 16;
			int y = (rand()%(h/2) + h/2) * 16;
			a->setX(x);
			a->setY(y);
		}
		actors.push_back(a);
	}

	log("  creating humans");
	for(i = 0; i < nh; i ++) {
		CActor *a = spawn_actor(0, 0, CActor :: eHuman);
		while(a->isStuck(gameMap) || actorStuck(a)) {
			int x = rand()%(w/2) * 16;
			int y = rand()%h * 16;
			a->setX(x);
			a->setY(y);
		}
		actors.push_back(a);
	}

	// sounds
	for(int i = 0; i < 10; i ++) {
		sound_box[i] = 0;
	}

	log("  starting game play");
	update_game();
	draw_game();
	blit_to_screen(swap_screen);

	PALETTE p;
	fade_interpolate((RGB *)gfx[0].dat, (RGB *)gfx[DARKPAL].dat, p, MIN((int)((double)(level - 1) * 9), 64), 0, 255);
	fade_in(p, 16);
	show_level_name = 200;
	play_sound(BRAINS, 100, 100);

	if (music_vol > 0) {
		log("  starting music");
		play_music(MOD_GAME, 0);
		log("    ok");
	}

	cycle_count = 0;
}


void new_game() {
	log("\nStarting new game.");
	i_am_a_cheat = 0;
	game_is_paused = 0;
	game_over = 0;
	score = 0;
	level = 0;
	brain_level = 0;
}



void update_title() {
	static int playing_music = 0;

	int kp = 0;
	if (keypressed()) {
		kp = readkey() >> 8;
	}
	if (kp == KEY_UP && title_data.cursor_pos) {
		title_data.cursor_pos --;
		play_sound(DROP, 0, 0);
	}
	if (kp == KEY_DOWN && title_data.cursor_pos < (title_data.menu_level == 0 ? 4 : 3)) {
		title_data.cursor_pos ++;
		play_sound(DROP, 0, 0);
	}
	if (kp == KEY_ESC && title_data.menu_level == 0) {
		play_sound(DROP, 0, 0);
		if (title_data.cursor_pos == 4) {
			game_state = GS_QUIT;
		}
		else {
			title_data.cursor_pos = 4;
		}
	}

	if (kp == KEY_SPACE || kp == KEY_ENTER) {
		play_sound(DROP, 0, 0);
		if (title_data.menu_level == 0) {
			switch(title_data.cursor_pos) {
				case 0:
					if (key[KEY_TAB]) ok_to_cheat = 1;
					fade_out(16);
					new_game();
					new_level();
					game_state = GS_PLAY;
					break;
				case 1:
					game_state = GS_HELP;
					break;
				case 2:
					game_state = GS_SCORES;
					break;
				case 3:
					title_data.menu_level = 1;
					title_data.cursor_pos = 0;
					break;
				case 4:
					game_state = GS_QUIT;
					break;
			}
		}
		else {
			switch(title_data.cursor_pos) {
				case 0:
				case 1:
					break;
				case 2:
					toggle_gfx();
					break;
				case 3:
					title_data.menu_level = 0;
					title_data.cursor_pos = 3;
					break;
			}
		}
	}
	if (kp == KEY_LEFT || kp == KEY_RIGHT) {
		if (title_data.menu_level == 1) {
			if (title_data.cursor_pos == 0) {
				int r = rand()%100;
				if (sound_volume > 0 && key[KEY_LEFT]) sound_volume = MAX(sound_volume - 25, 0);
				if (sound_volume < 250 && key[KEY_RIGHT]) sound_volume = MIN(sound_volume + 25, 250);
				if (r < 25) play_sound(LEVELCOMPLETE, 0, 0);
				else if (r < 50) play_sound(NAMNAM, 0, 0);
				else if (r < 75) play_sound(YEAH, 0, 0);
				else play_sound(BRAINS, 0, 0);
			}
			if (title_data.cursor_pos == 1) {
				if (music_vol > 0 && key[KEY_LEFT]) music_vol = MAX(music_vol - 0.1, 0);
				if (music_vol < 1 && key[KEY_RIGHT]) music_vol = MIN(music_vol + 0.1, 1);
				set_music_volume(music_vol);
			}
		}
	}

	// handle music
	if (title_data.menu_level == 1) {
		if (title_data.cursor_pos == 1) {
			if (!playing_music) {
				play_music(MOD_GAME, 0);
				playing_music = 1;
			}
		}
		else {
			stop_music();
			playing_music = 0;
		}
	}
}



void update_help() {
	int kp = 0;
	if (keypressed()) {
		kp = readkey() >> 8;
	}
	if (kp == KEY_SPACE || kp == KEY_ENTER) {
		play_sound(DROP, 0, 0);
		game_state = GS_TITLE;
	}

}


void update_scores() {
	int kp = 0;
	if (keypressed()) {
		kp = readkey() >> 8;
	}
	if (kp == KEY_SPACE || kp == KEY_ENTER) {
		play_sound(DROP, 0, 0);
		game_state = GS_TITLE;
	}

}






void draw_title() {
	clear_to_color(swap_screen, 1);
	draw_sprite(swap_screen, (BITMAP *)gfx[LOGO].dat, 15, 40);
	textprintf(swap_screen, (FONT *)gfx[FONT_TINY].dat, 265, 90, 240, "v%s", GAME_VERSION);

	if (key[KEY_TAB]) {
		textprintf_centre(swap_screen, (FONT *)gfx[FONT_TINY].dat, 160, 4, 240, "A CHEATER IS YOU!");
	}

	int mx = 150, my = 110;
	if (title_data.menu_level == 1) mx = 130;

	if (title_data.menu_level == 0) {
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "PLAY", mx, my, -1);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "HELP", mx, my + 20, -1);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "SCORES", mx, my + 40, -1);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "OPTIONS", mx, my + 60, -1);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "QUIT", mx, my + 80, -1);
	}
	else if (title_data.menu_level == 1) {
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "SOUND VOL:", mx, my, -1);
		BITMAP *brainBmp = create_bitmap(16, 16);
		for(int i = 0; i < sound_volume / 25; i ++) {
			blit((BITMAP *)gfx[ITEMS].dat, brainBmp, ((i*7 + (game_count>>3))%4) * 16, 8, 0, 0, 16, 16);
			draw_sprite(swap_screen, brainBmp, mx + 88 + i * 8, my + 6);
		}
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "MUSIC VOL:", mx, my + 20, -1);
		for(int i = 0; i < music_vol * 10; i ++) {
			blit((BITMAP *)gfx[ITEMS].dat, brainBmp, ((i*7 + (game_count>>3))%4) * 16, 8, 0, 0, 16, 16);
			draw_sprite(swap_screen, brainBmp, mx + 88 + i * 8, my + 6 + 20);
		}
		destroy_bitmap(brainBmp);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, (window ? "FULLSCREEN OFF" : "FULLSCREEN ON"), mx, my + 40, -1);
		textout(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "BACK", mx, my + 60, -1);
	}

	masked_blit((BITMAP *)gfx[PLAYER].dat, swap_screen, 0 * 16, 0, (mx - 30), 100 + 20 * title_data.cursor_pos, 16, 32);

	// draw credits
	draw_sprite(swap_screen, (BITMAP *)gfx[RUNNER].dat, 0, 232);
}



void draw_scores() {
	clear_to_color(swap_screen, 1);
	draw_sprite(swap_screen, (BITMAP *)gfx[LOGO].dat, 15, 40);
	draw_sprite(swap_screen, (BITMAP *)gfx[RUNNER].dat, 0, 232);

	draw_hisc_table(hiscores, swap_screen, (FONT *)gfx[FONT_BLACK].dat, 20, 110, -1);
}



void draw_help() {
	clear_to_color(swap_screen, 1);
	draw_sprite(swap_screen, (BITMAP *)gfx[LOGO].dat, 15, 40);
	draw_sprite(swap_screen, (BITMAP *)gfx[RUNNER].dat, 0, 232);

	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Zombies will try to convert your", 160, 110, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "friends into new zombies. Save", 160, 130, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "them by giving new brains.", 160, 150, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Arrows to move, space for brains.", 160, 180, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Press M to see the map.", 160, 200, -1);
}

void show_credits() {
	stop_music();
	clear_to_color(swap_screen, 1);
	draw_sprite(swap_screen, (BITMAP *)gfx[LOGO].dat, 15, 40);
	draw_sprite(swap_screen, (BITMAP *)gfx[RUNNER].dat, 0, 232);

	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Thanks for playing", 160, 130, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Code, gfx and sfx by Johan Peitz", 160, 160, -1);
	textout_centre(swap_screen, (FONT *)gfx[FONT_BLACK].dat, "Music by Anders Svensson", 160, 180, -1);
	blit_to_screen(swap_screen);
	cycle_count = 0;
	while(key[KEY_ESC]) poll_music();
	while(cycle_count < 250 && !key[KEY_ESC]) {
		yield_timeslice();
		poll_music();
	}
}

void run() {
	int game_is_on = 1;
	int draw = 1;
	int fade = 0;

	game_count = 0;
	while(game_is_on) {
		// do logic
		while(cycle_count > 0) {
			logic_count ++;

			poll_music();

			switch(game_state) {
				case GS_TITLE:
					update_title();
					break;
				case GS_PLAY:
					update_game();
					break;
				case GS_SCORES:
					update_scores();
					break;
				case GS_GOTO_TITLE:
					ok_to_cheat = 0;
					game_state = GS_TITLE;
					fade = 1;
					clear_keybuf();
					break;
				case GS_GOTO_SCORES:
					game_state = GS_SCORES;
					fade = 1;
					clear_keybuf();
					break;
				case GS_HELP:
					update_help();
					break;
				case GS_QUIT:
					game_is_on = 0;
					break;
			}

			draw = 1;
			yield_timeslice();
			cycle_count --;
		}
		// draw
		if (draw) {
			frame_count ++;
			switch(game_state) {
				case GS_TITLE:
					draw_title();
					break;
				case GS_PLAY:
					draw_game();
					break;
				case GS_HELP:
					draw_help();
					break;
				case GS_SCORES:
					draw_scores();
					break;
			}
			blit_to_screen(swap_screen);
			draw = 0;
			if (fade) {
				fade_in((RGB *)gfx[0].dat, 16);
				fade = 0;
			}
		}
	}
}



int main() {
	if (!init_game()) {
		allegro_message("%s:\nFailed to init game.", GAME_NAME);
		uninit_game();
		return -1;
	}

	clear_keybuf();

	game_state = GS_TITLE;
	run();
	show_credits();

	uninit_game();
	return 0;
}
END_OF_MAIN();

