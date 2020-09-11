#include "PlayMode.hpp"
#include "data_path.hpp"
#include "Load.hpp"
#include "read_write_chunk.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <fstream>
#include <iostream>
#include <math.h>

std::vector< glm::u8vec4 > palette_0, palette_1, palette_2, palette_3, palette_4, palette_5;
std::vector< uint8_t > background_0, background_1, background_2, background_3, background_4, background_5, background_6, background_7;
std::vector< uint8_t > player_N0, player_N1, player_E0, player_E1, player_S0, player_S1, player_W0, player_W1;
std::vector< uint8_t > player_NE0, player_NE1, player_SE0, player_SE1, player_SW0, player_SW1, player_NW0, player_NW1;
std::vector< uint8_t > slime_0, slime_1, superslime_0, superslime_1;
std::vector< uint8_t > bullet_0, bullet_1;
std::vector< uint8_t > digit_00, digit_01, digit_10, digit_11, digit_20, digit_21, digit_30, digit_31, digit_40, digit_41, digit_50, digit_51;
std::vector< uint8_t > digit_60, digit_61, digit_70, digit_71, digit_80, digit_81, digit_90, digit_91;

struct SpriteAtlas {

	std::filebuf fb;

	SpriteAtlas(std::string path) {
		//file open code from http://www.cplusplus.com/reference/istream/istream/istream/
		if (fb.open(path, std::ios::in | std::ios::binary)) {
			std::istream from(&fb);

			read_chunk(from, "pal0", &palette_0);
			read_chunk(from, "bgt0", &background_0);
			read_chunk(from, "bgt1", &background_1);
			read_chunk(from, "bgt2", &background_2);
			read_chunk(from, "bgt3", &background_3);
			read_chunk(from, "bgt4", &background_4);
			read_chunk(from, "bgt5", &background_5);
			read_chunk(from, "bgt6", &background_6);
			read_chunk(from, "bgt7", &background_7);
			read_chunk(from, "pal1", &palette_1);
			read_chunk(from, "pc00", &player_N0);
			read_chunk(from, "pc01", &player_N1);
			read_chunk(from, "pc10", &player_E0);
			read_chunk(from, "pc11", &player_E1);
			read_chunk(from, "pc20", &player_S0);
			read_chunk(from, "pc21", &player_S1);
			read_chunk(from, "pc30", &player_W0);
			read_chunk(from, "pc31", &player_W1);
			read_chunk(from, "pc40", &player_NE0);
			read_chunk(from, "pc41", &player_NE1);
			read_chunk(from, "pc50", &player_SE0);
			read_chunk(from, "pc51", &player_SE1);
			read_chunk(from, "pc60", &player_SW0);
			read_chunk(from, "pc61", &player_SW1);
			read_chunk(from, "pc70", &player_NW0);
			read_chunk(from, "pc71", &player_NW1);
			read_chunk(from, "pal2", &palette_2);
			read_chunk(from, "sl00", &slime_0);
			read_chunk(from, "sl01", &slime_1);
			read_chunk(from, "pal5", &palette_5);
			read_chunk(from, "sl10", &superslime_0);
			read_chunk(from, "sl11", &superslime_1);
			read_chunk(from, "pal3", &palette_3);
			read_chunk(from, "bu00", &bullet_0);
			read_chunk(from, "bu01", &bullet_1);
			read_chunk(from, "pal4", &palette_4);
			read_chunk(from, "0000", &digit_00);
			read_chunk(from, "0001", &digit_01);
			read_chunk(from, "0100", &digit_10);
			read_chunk(from, "0101", &digit_11);
			read_chunk(from, "0200", &digit_20);
			read_chunk(from, "0201", &digit_21);
			read_chunk(from, "0300", &digit_30);
			read_chunk(from, "0301", &digit_31);
			read_chunk(from, "0400", &digit_40);
			read_chunk(from, "0401", &digit_41);
			read_chunk(from, "0500", &digit_50);
			read_chunk(from, "0501", &digit_51);
			read_chunk(from, "0600", &digit_60);
			read_chunk(from, "0601", &digit_61);
			read_chunk(from, "0700", &digit_70);
			read_chunk(from, "0701", &digit_71);
			read_chunk(from, "0800", &digit_80);
			read_chunk(from, "0801", &digit_81);
			read_chunk(from, "0900", &digit_90);
			read_chunk(from, "0901", &digit_91);
		}

	};
	~SpriteAtlas() {
		fb.close();
	};
};

//global Load code based off of https://15466.courses.cs.cmu.edu/lesson/assets
Load< SpriteAtlas > atlas(LoadTagDefault, [](){
	SpriteAtlas* ret = new SpriteAtlas(data_path("../assets/assets.txt"));
	return ret;
});

void PlayMode::set_tile_to_vector(int tile, std::vector< uint8_t > &bit0_v, std::vector< uint8_t > &bit1_v) {
	for (int i = 0; i < 8; i++) {
		ppu.tile_table[tile].bit0[i] = bit0_v[i];
		ppu.tile_table[tile].bit1[i] = bit1_v[i];
	}
}

float PlayMode::dist_from_player(int slime_index) {
	glm::vec2 pos = slimes[slime_index].at;
	float dx = pos.x - player_at.x;
	float dy = pos.y - player_at.y;
	return sqrt(dx * dx + dy * dy);
}

float PlayMode::dist_from_bullet(int slime_index) {
	glm::vec2 pos = slimes[slime_index].at;
	float dx = pos.x - bullet_at.x;
	float dy = pos.y - bullet_at.y;
	return sqrt(dx * dx + dy * dy);
}

glm::vec2 PlayMode::dir_to_player(int slime_index) {
	glm::vec2 pos = slimes[slime_index].at;
	float dist = dist_from_player(slime_index);

	return (player_at - pos) / dist;
}

void PlayMode::set_bullet_dir() {
	if (dir == 4) {
		bullet_dir.x = 0.0f;
		bullet_dir.y = 1.0f;
	}
	else if (dir == 5) {
		bullet_dir.x = 1.0f;
		bullet_dir.y = 0.0f;
	}
	else if (dir == 6) {
		bullet_dir.x = 0.0f;
		bullet_dir.y = -1.0f;
	}
	else if (dir == 7) {
		bullet_dir.x = -1.0f;
		bullet_dir.y = 0.0f;
	}
	else if (dir == 8) {
		bullet_dir.x = 1.0f;
		bullet_dir.y = 1.0f;
	}
	else if (dir == 9) {
		bullet_dir.x = 1.0f;
		bullet_dir.y = -1.0f;
	}
	else if (dir == 10) {
		bullet_dir.x = -1.0f;
		bullet_dir.y = -1.0f;
	}
	else if (dir == 11) {
		bullet_dir.x = -1.0f;
		bullet_dir.y = 1.0f;
	}
}

void PlayMode::restart_game() {
	player_at = glm::vec2(game_width / 2.0f, game_height / 2.0f);
	dir = 4;
	bullet_at = player_at;
	score = 0;

	for (uint32_t i = 0; i < starting_slimes; i++) {
		slimes[i].at.x = (mt() / float(mt.max())) * game_width;
		slimes[i].at.y = (mt() / float(mt.max())) * game_height;
		while (dist_from_player(i) < min_dist) {
			slimes[i].at.x = (mt() / float(mt.max())) * game_width;
			slimes[i].at.y = (mt() / float(mt.max())) * game_height;
		}
		slimes[i].state = 1;
	}
	for (uint32_t i = starting_slimes; i < max_slimes; i++) {
		slimes[i].state = 0;
	}
}

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	ppu.background_color = glm::u8vec4(255, 255, 255, 255);

	ppu.palette_table[0][0] = palette_0[0];
	ppu.palette_table[0][1] = palette_0[1];
	ppu.palette_table[0][2] = palette_0[2];

	ppu.palette_table[1][0] = palette_1[0];
	ppu.palette_table[1][1] = palette_1[1];
	ppu.palette_table[1][2] = palette_1[2];
	ppu.palette_table[1][3] = palette_1[3];

	ppu.palette_table[2][0] = palette_2[0];
	ppu.palette_table[2][1] = palette_2[1];
	ppu.palette_table[2][2] = palette_2[2];

	ppu.palette_table[3][0] = palette_3[0];
	ppu.palette_table[3][1] = palette_3[1];

	ppu.palette_table[4][0] = palette_4[0];
	ppu.palette_table[4][1] = palette_4[1];

	ppu.palette_table[5][0] = palette_5[0];
	ppu.palette_table[5][1] = palette_5[1];
	ppu.palette_table[5][2] = palette_5[2];

	set_tile_to_vector(0, background_0, background_1);
	set_tile_to_vector(1, background_2, background_3);
	set_tile_to_vector(2, background_4, background_5);
	set_tile_to_vector(3, background_6, background_7);

	set_tile_to_vector(4, player_N0, player_N1);
	set_tile_to_vector(5, player_E0, player_E1);
	set_tile_to_vector(6, player_S0, player_S1);
	set_tile_to_vector(7, player_W0, player_W1);
	set_tile_to_vector(8, player_NE0, player_NE1);
	set_tile_to_vector(9, player_SE0, player_SE1);
	set_tile_to_vector(10, player_SW0, player_SW1);
	set_tile_to_vector(11, player_NW0, player_NW1);

	set_tile_to_vector(12, slime_0, slime_1);
	set_tile_to_vector(24, superslime_0, superslime_1);

	set_tile_to_vector(13, bullet_0, bullet_1);

	set_tile_to_vector(14, digit_00, digit_01);
	set_tile_to_vector(15, digit_10, digit_11);
	set_tile_to_vector(16, digit_20, digit_21);
	set_tile_to_vector(17, digit_30, digit_31);
	set_tile_to_vector(18, digit_40, digit_41);
	set_tile_to_vector(19, digit_50, digit_51);
	set_tile_to_vector(20, digit_60, digit_61);
	set_tile_to_vector(21, digit_70, digit_71);
	set_tile_to_vector(22, digit_80, digit_81);
	set_tile_to_vector(23, digit_90, digit_91);

	for (int w = 0; w < 32; w++) {
		for (int h = 0; h < 32; h++) {
			uint16_t val = 0;
			if (h % 2 == 1) {
				val += 2;
			}
			if (w % 2 == 1) {
				val += 1;
			}
			ppu.background[w * ppu.BackgroundWidth + h] = val;
		}
	}

	for (uint32_t i = 0; i < starting_slimes; i++) {
		slimes[i].at.x = (mt() / float(mt.max())) * game_width;
		slimes[i].at.y = (mt() / float(mt.max())) * game_height;
		while (dist_from_player(i) < min_dist) {
			slimes[i].at.x = (mt() / float(mt.max())) * game_width;
			slimes[i].at.y = (mt() / float(mt.max())) * game_height;
		}
		slimes[i].state = 1;
	}
	for (uint32_t i = starting_slimes; i < max_slimes; i++) {
		slimes[i].state = 0;
	}

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	respawn_timer += elapsed;
	bullet_timer += elapsed;

	//spawn new slimes
	if (respawn_timer > respawn_rate) {
		for (uint32_t s = 0; s < respawn_num; s++) {
			for (uint32_t i = 0; i < max_slimes; i++) {
				if (slimes[i].state == 0) {
					slimes[i].at.x = (mt() / float(mt.max())) * game_width;
					slimes[i].at.y = (mt() / float(mt.max())) * game_height;
					while (dist_from_player(i) < min_dist) {
						slimes[i].at.x = (mt() / float(mt.max())) * game_width;
						slimes[i].at.y = (mt() / float(mt.max())) * game_height;
					}
					slimes[i].state = 1;
					break;
				}
			}
		}
		respawn_timer -= respawn_rate;
	}

	// Count alive slimes
	int slime_count = 0;
	for (uint32_t i = 0; i < max_slimes; i++) {
		if (slimes[i].state == 1) {
			slime_count++;
		}
	}
	if (slime_count == max_slimes)
		slime_speed = max_slime_speed;
	else
		slime_speed = base_slime_speed;

	bool game_over = false;

	for (uint32_t i = 0; i < max_slimes; i++) {
		if (slimes[i].state == 1) {
			if (dist_from_bullet(i) < kill_distance) {
				slimes[i].state = 0;
				score++;
				continue;
			}
			if (dist_from_player(i) < kill_player_dist) {
				game_over = true;
				break;
			}
			glm::vec2 slime_dir = dir_to_player(i);
			slimes[i].at += slime_speed * slime_dir * elapsed;
		}
	}

	if (bullet_timer > bullet_duration) {
		set_bullet_dir();
		bullet_at = player_at + bullet_dir*bullet_spawn_distance;
		bullet_timer -= bullet_duration;
	}

	bullet_at += bullet_speed * bullet_dir * elapsed;
	if (bullet_at.x > game_width) bullet_at.x = game_width;
	else if (bullet_at.x < 0.0f) bullet_at.x = 0.0f;
	if (bullet_at.y > game_height) bullet_at.y = game_height;
	else if (bullet_at.y < 0.0f) bullet_at.y = 0.0f;

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	if (player_at.x > game_width) player_at.x = game_width;
	else if (player_at.x < 0.0f) player_at.x = 0.0f;
	if (player_at.y > game_height) player_at.y = game_height;
	else if (player_at.y < 0.0f) player_at.y = 0.0f;

	// Get player orientation
	if (up.pressed && right.pressed)
		dir = 8;
	else if (down.pressed && right.pressed)
		dir = 9;
	else if (down.pressed && left.pressed)
		dir = 10;
	else if (up.pressed && left.pressed)
		dir = 11;
	else if (up.pressed)
		dir = 4;
	else if (right.pressed)
		dir = 5;
	else if (down.pressed)
		dir = 6;
	else if (left.pressed)
		dir = 7;

	if (game_over) restart_game();
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	/*ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);*/

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	//for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
	//	for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
	//		ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
	//	}
	//}

	//background scroll:
	//ppu.background_position.x = int32_t(-0.5f * player_at.x);
	//ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	ppu.sprites[0].index = dir;
	ppu.sprites[0].attributes = 1;

	//draw slime enemies
	for (uint32_t i = 0; i < max_slimes; ++i) {
		if (slimes[i].state == 1) {
			ppu.sprites[i + 1].x = int32_t(slimes[i].at.x);
			ppu.sprites[i + 1].y = int32_t(slimes[i].at.y);
			if (slime_speed == base_slime_speed) {
				ppu.sprites[i + 1].index = 12;
				ppu.sprites[i + 1].attributes = 2;
			}
			else {
				ppu.sprites[i + 1].index = 24;
				ppu.sprites[i + 1].attributes = 5;
			}
		}
		else {
			ppu.sprites[i + 1].x = 255;
			ppu.sprites[i + 1].y = 255;
			ppu.sprites[i + 1].index = 12;
			ppu.sprites[i + 1].attributes = 2;
		}
	}
	 //draw bullet
	ppu.sprites[63].x = int32_t(bullet_at.x);
	ppu.sprites[63].y = int32_t(bullet_at.y);
	ppu.sprites[63].index = 13;
	ppu.sprites[63].attributes = 3;

	//draw score
	int ones, tens, hund;
	ones = score % 10;
	tens = (score % 100 - ones) / 10;
	hund = (score % 1000 - tens - ones) / 100;

	ppu.sprites[60].x = 215;
	ppu.sprites[60].y = 230;
	ppu.sprites[60].index = hund + 14;
	ppu.sprites[60].attributes = 4;

	ppu.sprites[61].x = 225;
	ppu.sprites[61].y = 230;
	ppu.sprites[61].index = tens + 14;
	ppu.sprites[61].attributes = 4;

	ppu.sprites[62].x = 235;
	ppu.sprites[62].y = 230;
	ppu.sprites[62].index = ones + 14;
	ppu.sprites[62].attributes = 4;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
