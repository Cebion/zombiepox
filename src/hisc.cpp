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


#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "hisc.h"

// creates a table to work with
Thisc* make_hisc_table() {
	Thisc *tmp;

	tmp = (Thisc *)malloc(MAX_HISCORES*sizeof(Thisc));
	if (!tmp) return NULL;

	return tmp;
}

// destroys a table
void destroy_hisc_table(Thisc *table) {
	free(table);
}

// check if provided score is allowed to enter the table
int qualify_hisc_table(Thisc *table, Thisc post) {
	int i;
	if (post.score == 0) return 0;
	for (i=0;i<MAX_HISCORES;i++) {
		if (post.score > table[i].score) return 1;
	}
	return 0;
}


// sorts the table
void sort_hisc_table(Thisc *table) {
	int i,n;
	Thisc tmp;
	int isSorted;

	n = MAX_HISCORES;

	do {
		isSorted = 1;
		for(i = 0; i < n - 1; i ++) {
			int swap = 0;
			if (table[i].score < table[i + 1].score) swap = 1;
			if (swap) {
				tmp = table[i];
				table[i] = table[i + 1];
				table[i + 1] = tmp;
				isSorted = 0;
			}
		}
	} while(!isSorted);

}

// Replaces the lowest score with the specified
void enter_hisc_table(Thisc *table, Thisc post) {
	unsigned int los=10000000;
	unsigned int lol=10000000;
	int loID=-1;
	int i;

	// find lowest score
	for (i=0;i<MAX_HISCORES;i++) {
		if (table[i].score < los || table[i].level < lol) {
			loID = i;
			los = table[i].score;
			lol = table[i].level;
		}
	}

	if (loID>=0) table[loID] = post;
}

// Resets the table to the values specified
void reset_hisc_table(Thisc *table, char *name, int hi, int lo) {
	int i;
	if (hi < lo);

	for (i=0;i<MAX_HISCORES;i++) {
		strcpy(table[i].name, name);
		table[i].score = (i + 1) * (i * i * i + 1) * 953;
		table[i].level = (i * 2) / 1 + 1;
	}

	table[4].score =  400000;
	table[3].score =  200000;
	table[2].score =  100000;
	table[1].score =   50000;
	table[0].score =   10000;

	table[4].level = 8;
	table[3].level = 6;
	table[2].level = 4;
	table[1].level = 2;
	table[0].level = 1;

	sprintf(table[4].name, "Zombiepox");
	sprintf(table[3].name, "Free Lunch Design");
	sprintf(table[2].name, "Johan Peitz");
	sprintf(table[1].name, "Alex the Allegator");
	sprintf(table[0].name, "Harold the Homeboy");

	sort_hisc_table(table);
}


// generates a checksum for the entry
int generate_checksum(Thisc *entry) {
	int i = (entry->score + entry->level) * 37;
	char *s = entry->name;

	for(; *s; s++) i = 131*i + *s;

	return i;
}


// loads table from disk, returns 1 on success
int load_hisc_table(Thisc *table, PACKFILE *fp) {
	int i;
	int ok = 1;

	for(i=0; i<MAX_HISCORES; i++) {
		int c_disk, c_real;
		// load entry
		pack_fread(&table[i], sizeof(Thisc), fp);
		// load checksum
		pack_fread(&c_disk, sizeof(int), fp);
		// generate check sum
		c_real = generate_checksum(&table[i]);
		// compare checksums
		if (c_real != c_disk) ok = 0; // tampered with
	}

	return ok;
}


// saves table to disk
void save_hisc_table(Thisc *table, PACKFILE *fp) {
	int i;

	for(i=0; i<MAX_HISCORES; i++) {
		int checksum;
		// save entry
		pack_fwrite(&table[i], sizeof(Thisc), fp);
		// generate check sum
		checksum = generate_checksum(&table[i]);
		// save checksum
		pack_fwrite(&checksum, sizeof(int), fp);
	}
}

// draws a single hisc post
void draw_hisc_post(Thisc *table, BITMAP *bmp, FONT *fnt, int x, int y, int color) {
	if (color);
	x ++; y ++;
	textprintf(bmp, fnt, x, y, color, "%-20s", table->name);
	textprintf_right(bmp, fnt, x+200, y, color, "%6d", table->level);
	textprintf_right(bmp, fnt, x+280, y, color, "%6d", table->score);
}

// draws the entire table
void draw_hisc_table(Thisc *table, BITMAP *bmp, FONT *fnt, int x, int y, int color) {
	int i;

	for(i=0;i<MAX_HISCORES;i++)
		draw_hisc_post(&table[i], bmp, fnt, x, y + 20 * i, color);
}




