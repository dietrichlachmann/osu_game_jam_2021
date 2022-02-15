#include <stdio.h>
#include <stdlib.h>
#include "gb/gb.h"
#include "assets/tileset.c"
#include "assets/level_one.c"
#include "assets/level_one_complete.c"
#include "assets/level_one_sign_one.c"
#include "assets/level_one_sign_two.c"
#include "assets/level_one_sign_three.c"
#include "assets/echo_sprite.c"
#include "assets/endscreen.c"

// random declarations to avoid warnings
void setSign(UINT8 cur_sign);
void setLevel(UINT8 cur_level);

// empty char array for collisions
const char empty[1] = { 0x00 };

// sprite struct
typedef struct Unit {
	UBYTE spriteID[1];
	UINT8 x;
	UINT8 y;
	UINT8 width;
	UINT8 height;
} Unit;

Unit echo;

// less cpu intensive delay function
void performDelay(UINT8 numloops) {
	UINT8 i;
	for (i = 0; i < numloops; i++) {
		wait_vbl_done();
	}
}

// move function depending on sprite memory location
void moveUnit(Unit* c, UINT8 x, UINT8 y) {
	move_sprite(c->spriteID[0], x, y);
}

// checks for background collisions
UBYTE checkCollision(UINT8 player_moveX, UINT8 player_moveY) {
	UINT16 top_left_idxX;
	UINT16 top_left_idxY;
	UINT16 tile_idx;
	UBYTE r;

	// adjusting for screen pixel offsetting
	top_left_idxX = (player_moveX - 8) / 8;
	top_left_idxY = (player_moveY - 16) / 8;
	tile_idx = 20 * top_left_idxY + top_left_idxX;
	// compare r to empty char array
	r = (level_one[tile_idx] == empty[0]);

	// 0x14, 0x15, 0x15, 0x16 - pathway tiles
	// 0x09 - sign tile
	// 0x0E, 0x0D, 0x0E, 0x0F - flower tiles
	if (level_one[tile_idx] == 0x14 || level_one[tile_idx] == 0x15 || level_one[tile_idx] == 0x16 || level_one[tile_idx] == 0x17 
		|| level_one[tile_idx] == 0x09 || level_one[tile_idx] == 0x10 || level_one[tile_idx] == 0x0D || level_one[tile_idx] == 0x0E
		|| level_one[tile_idx] == 0x0F) {
		r = 1;
	}

	return r;
}

// checks for specific order of sign tile and updates map
void checkSign(UINT8 cur_posX, UINT8 cur_posY, UINT8 cur_level, UBYTE* sign_one, UBYTE* sign_two) {
	UINT16 top_left_idxX;
	UINT16 top_left_idxY;
	UINT16 tile_idx;
	UINT8 sign = 0;

	// offsetting math
	top_left_idxX = (cur_posX - 8) / 8;
	top_left_idxY = (cur_posY - 16) / 8;
	tile_idx = 20 * top_left_idxY + top_left_idxX;
	// while current position is a sign and that sign is 212 in the background array
	while (level_one[tile_idx] == 0x09 && tile_idx == 212) {
		sign = 1;
		// update read_sign_xxx variables declared in main
		if (*sign_two == 0 && *sign_one == 1) {
			*sign_two = 1;
		}
		// call setsign function to update background
		setSign(sign);
		// go back to main map if player moves off sign
		if (joypad() & J_LEFT || joypad() & J_RIGHT || joypad() & J_UP || joypad() & J_DOWN) {
			setLevel(cur_level);
			SHOW_SPRITES;
			break;
		}
	}
	while (level_one[tile_idx] == 0x09 && tile_idx == 128) {
		sign = 2;
		if (*sign_one == 0) {
			*sign_one = 1;
		}
		setSign(sign);
		if (joypad() & J_LEFT || joypad() & J_RIGHT || joypad() & J_UP || joypad() & J_DOWN) {
			setLevel(cur_level);
			SHOW_SPRITES;
			break;
		}
	}
	while (level_one[tile_idx] == 0x09 && tile_idx == 83) {
		sign = 3;
		*sign_one = 0;
		*sign_two = 0;
		setSign(sign);
		if (joypad() & J_LEFT || joypad() & J_RIGHT || joypad() & J_UP || joypad() & J_DOWN) {
			setLevel(cur_level);
			SHOW_SPRITES;
			break;
		}
	}
	setLevel(cur_level);
}

// initialization function for echo sprite
void setEcho(UINT8 cur_level) {
	if (cur_level == 1) {
		echo.x = 120;
		echo.y = 144;
		echo.width = 8;
		echo.height = 8;

		set_sprite_tile(0, 0);
		echo.spriteID[0] = 0;
		moveUnit(echo.x, echo.y);
	}
}

// map updating function
void setLevel(UINT8 cur_level){
	if (cur_level == 1) {
		set_bkg_tiles(0, 0, 20, 18, level_one);
	}
	else if (cur_level == 2) {
		set_bkg_tiles(0, 0, 20, 18, level_one_complete);
	}
}

// sign function - REALLY BAD
void setSign(UINT8 cur_sign) {
	if (cur_sign == 1) {
		set_bkg_tiles(0, 0, 20, 18, level_one_sign_one);
		HIDE_SPRITES;
	}
	else if (cur_sign == 2) {
		set_bkg_tiles(0, 0, 20, 18, level_one_sign_two);
		HIDE_SPRITES;
	}
	else if (cur_sign == 3) {
		set_bkg_tiles(0, 0, 20, 18, level_one_sign_three);
		HIDE_SPRITES;
	}
}

// check for endgame parameters
UBYTE checkEndgame(UINT8 cur_posX, UINT8 cur_posY, UBYTE* sign_one, UBYTE* sign_two) {
	UINT16 top_left_idxX;
	UINT16 top_left_idxY;
	UINT16 tile_idx;
	UBYTE r = 0;

	// offsetting math
	top_left_idxX = (cur_posX - 8) / 8;
	top_left_idxY = (cur_posY - 16) / 8;
	tile_idx = 20 * top_left_idxY + top_left_idxX;

	// endgame only triggers after reading signs in correct order
	if (*sign_one == 1 && *sign_two == 1){
		// endgame also requires echo to be standing on specific tiles
		if (tile_idx == 44 || tile_idx == 45) {
			r = 1;
		}
	}
	return r;
}

void main() {
	UINT8 echo_animation = 0;
	UINT8 level = 1;
	UBYTE read_sign_one = 0;
	UBYTE read_sign_two = 0;

	// game initialization
	set_bkg_data(0, 60, tileset);
	set_sprite_data(0, 3, echo_sprite);
	setLevel(level);
	setEcho(level);
	SHOW_SPRITES;
	SHOW_BKG;
	DISPLAY_ON;

	while (1) {
		// movement controls
		if (joypad() & J_LEFT) {
			if (checkCollision(echo.x - 8, echo.y)) {
				echo.x -= 8;
				if (echo_animation != 0) {
					echo_animation = 0;
					set_sprite_tile(0, 0);
				}
				moveUnit(&echo, echo.x, echo.y);
			}
		}
		if (joypad() & J_RIGHT) {
			if (checkCollision(echo.x + 8, echo.y)) {
				echo.x += 8;
				if (echo_animation != 1) {
					echo_animation = 1;
					set_sprite_tile(0, 1);
				}
				moveUnit(&echo, echo.x, echo.y);
			}
		}
		if (joypad() & J_UP) {
			if (checkCollision(echo.x, echo.y - 8)) {
				echo.y -= 8;
				if (echo_animation != 2) {
					echo_animation = 2;
					set_sprite_tile(0, 2);
				}
				moveUnit(&echo, echo.x, echo.y);
			}
		}
		if (joypad() & J_DOWN) {
			if (checkCollision(echo.x, echo.y + 8)) {
				echo.y += 8;
				if (echo_animation != 0) {
					echo_animation = 0;
					set_sprite_tile(0, 0);
				}
				moveUnit(&echo, echo.x, echo.y);
			}
		}
		checkSign(echo.x, echo.y, level, &read_sign_one, &read_sign_two);
		if (read_sign_one == 1 && read_sign_two == 1) {
			level = 2;
			setLevel(level);
		}
		if (checkEndgame(echo.x, echo.y, &read_sign_one, &read_sign_two)) {
			break;
		}
		performDelay(5);
	}
	HIDE_SPRITES;
	set_bkg_tiles(0, 0, 20, 18, endscreen);
}