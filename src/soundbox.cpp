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


#include <string.h>
#include "allegro.h"
#include "aldumb.h"
#include "soundbox.h"
#include "main.h"
#include "../data/data_sfx.h"


extern DATAFILE *sfx;
static DUH *duh = NULL;
static AL_DUH_PLAYER *dp = NULL;
static DUH_SIGRENDERER *sr = NULL;
double music_vol;





void init_soundbox() {
	dumb_register_packfiles();
    dumb_register_dat_mod(DUMB_DAT_MOD);
	dumb_resampling_quality = 4;
	dumb_it_max_to_mix = 128;

	music_vol = 0.5;
}



void uninit_soundbox() {
	unload_datafile(sfx);
}




// stops any mod playing
static void _stop_music(void) {
	if (dp != NULL) {
		al_stop_duh(dp);
		dp = NULL;
	}
}


// starts the mod at position x
static void _start_music(int startorder) {
	const int n_channels = 2; /* stereo */

	_stop_music();

	{
		sr = dumb_it_start_at_order(duh, n_channels, startorder);
		dp = al_duh_encapsulate_sigrenderer(sr, music_vol, 4096, 44100);
		if (!dp) {
			duh_end_sigrenderer(sr); // howto.txt doesn't mention that this is necessary! dumb.txt does ...
			dp = NULL;
			log("  *** failed eto ncapsulate sig renderer");
		}
		else {
			al_poll_duh(dp);
		}
	}
}


void play_music(int id, int start_pattern) {
	if (id != MOD_GAME) return; // there is only one track
	duh = (DUH *)sfx[0].dat;
	_start_music(start_pattern);
}

void poll_music() {
	if (dp != NULL) {
		al_poll_duh(dp);
	}
}

void stop_music() {
	_stop_music();
}


void set_music_volume(double vol) {
	if (dp != NULL) {
		al_duh_set_volume(dp, vol);
	}
}


