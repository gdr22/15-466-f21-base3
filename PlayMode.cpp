#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <fstream>
#include <iostream>

Load< Sound::Sample > note57(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note57.wav"));
});
Load< Sound::Sample > note58(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note58.wav"));
});
Load< Sound::Sample > note59(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note59.wav"));
});
Load< Sound::Sample > note60(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note60.wav"));
});
Load< Sound::Sample > note61(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note61.wav"));
});
Load< Sound::Sample > note62(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note62.wav"));
});
Load< Sound::Sample > note63(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note63.wav"));
});
Load< Sound::Sample > note64(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note64.wav"));
});
Load< Sound::Sample > note65(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note65.wav"));
});
Load< Sound::Sample > note66(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note66.wav"));
});
Load< Sound::Sample > note67(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note67.wav"));
});
Load< Sound::Sample > note68(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note68.wav"));
});
Load< Sound::Sample > note69(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note69.wav"));
});
Load< Sound::Sample > note70(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note70.wav"));
});
Load< Sound::Sample > note71(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note71.wav"));
});
Load< Sound::Sample > note72(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("note72.wav"));
});

void PlayMode::play_note(int note) {
	auto sound = Sound::play_3D(*note_sfx[note - note_base], 1.0f, glm::vec3(0.f));
}

void PlayMode::init_level(int level) {
	boxes.clear();
	crates.clear();

	// Clear all background tiles
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x + PPU466::BackgroundWidth * y] = 255 | (0 << 8);
		}
	}

	// Copy in the tile map
	for(uint32_t x = 0; x < TILEMAP_WIDTH; ++x) {
		for(uint32_t y = 0; y < TILEMAP_HEIGHT; ++y) {
			tile_map[y][x] = levels[level][y][x];

			if(tile_map[y][x] == 1) {
				int bgx = 2 * x;
				int bgy = 2 * (TILEMAP_HEIGHT - y - 1);

				ppu.background[(bgx    ) + PPU466::BackgroundWidth * (bgy    )] = 0;
				ppu.background[(bgx + 1) + PPU466::BackgroundWidth * (bgy    )] = 1;
				ppu.background[(bgx    ) + PPU466::BackgroundWidth * (bgy + 1)] = 16;
				ppu.background[(bgx + 1) + PPU466::BackgroundWidth * (bgy + 1)] = 17;
			}
			else if(tile_map[y][x] == 2) {
				int bgx = 2 * x;
				int bgy = 2 * (TILEMAP_HEIGHT - y - 1);

				Box box;
				box.music_time = (int)boxes.size();
				box.at  = glm::vec2(bgx * TILE_SIZE, bgy * TILE_SIZE);
				box.src = glm::vec2(bgx * TILE_SIZE, bgy * TILE_SIZE);
				boxes.emplace_back(box);
			}
			else if(tile_map[y][x] == 3) {
				int bgx = 2 * x;
				int bgy = 2 * (TILEMAP_HEIGHT - y - 1);

				Crate crate;
				crate.at = glm::vec2(bgx * TILE_SIZE, bgy * TILE_SIZE);
				crate.vel = glm::vec2(0.f);
				crates.emplace_back(crate);
			}


		}
	}

	// Copy the music notes for this level
	music = notes[level];
	note_error = note_errs[level];
	music_times = note_times[level];
	music_errors = 1;
	music_len = (int)music.size();
	music_pos = music_len + 1;
	
	player_at.x = 128;
	player_at.y = 96;

	player_screen_pos.x = (int)player_at.x;
	player_screen_pos.y = (int)player_at.y;
}

PlayMode::PlayMode() {
	std::vector< char > to;
	std::filebuf fb; 
	
	// Load in the binary sprites table
	if (fb.open(data_path("../assets/sprites.bin"), std::ios::in | std::ios::binary)) {
		std::istream from(&fb);
        
		read_chunk(from, "img0", &to);
		printf("Loaded\n");
        
		int offset = 0;
		for (int sprite_idx = 0; sprite_idx < 256; sprite_idx++) {
			for (int row = 0; row < 8; row++) {
				ppu.tile_table[sprite_idx].bit0[row] = to[offset++];
			}
            
			for (int row = 0; row < 8; row++) {
				ppu.tile_table[sprite_idx].bit1[row] = to[offset++];
			}
		}
	}
    
	// Load in the sprite palettes from a PNG
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(data_path("../assets/Palettes.png"), &size, &data, UpperLeftOrigin);

	for (size_t i = 0; i < data.size(); i += 4) {
		for (size_t j = 0; j < 4; j++) {
			ppu.palette_table[i / 4][j] = data[i + j];
		}
	}
	
	sprite_tiles = {
		{0, 1, 16, 17}, {32, 33, 48, 49}, {2, 3, 18, 19}, {4, 5, 20, 21}, {6, 7, 22, 23}
	};

	init_level(level);

	//start music loop playing:
	// (note: position will be over-ridden in update())
	//sound_loop = Sound::loop_3D(*note60, 1.0f, glm::vec3(0.f), 10.0f);
	note_sfx = { 						 note57, note58, note59,
				 note60, note61, note62, note63, note64, note65, 
				 note66, note67, note68, note69, note70, note71, 
				 note72 };

	Sound::listener.set_position_right(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), 1.0f / 60.0f);
}

PlayMode::~PlayMode() {
}

void PlayMode::draw_sprite(int x, int y, int size, bool background, int palette) {
	// Ignore sprites beyond the screen range
	if (x < 0 || x > (int)PPU466::ScreenWidth) return;
	if (y < 0 || y > (int)PPU466::ScreenHeight) return;

	std::vector<int> x_off = { 0, TILE_SIZE, 0, TILE_SIZE };
	std::vector<int> y_off = { 0, 0, TILE_SIZE, TILE_SIZE };

	// Draws 1x1 or 2x2 sprites
	int i = 0;
	for (int tile : sprite_tiles[size]) {
		if (x + x_off[i] < 0 || x + x_off[i] >= (int)PPU466::ScreenWidth) continue;
		if (y - y_off[i] < 0 || y - y_off[i] >= (int)PPU466::ScreenHeight) continue;
		// If we have more than the allowed number of sprites, just ignore to precent crashing.
		if (sprite_cnt == 64) continue;

		ppu.sprites[sprite_cnt].x = x + x_off[i];
		ppu.sprites[sprite_cnt].y = y - y_off[i];
		ppu.sprites[sprite_cnt].index = tile;
		ppu.sprites[sprite_cnt].attributes = background ? 1 << 7 : 0;
		ppu.sprites[sprite_cnt].attributes |= palette;
		sprite_cnt++;
		i++;
	}	
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			play.downs += 1;
			play.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			play.pressed = false;
			return true;
		}
	} 

	return false;
}

bool PlayMode::collide_below(glm::vec2 pos) {
	glm::ivec2 screen_pos((int)pos.x, (int)pos.y);

	int tile_x =  screen_pos.x      / (2 * TILE_SIZE);
	int tile_y = (screen_pos.y - 1) / (2 * TILE_SIZE);
	tile_y = (TILEMAP_HEIGHT - 1) - tile_y;

	// If collision with the tilemap
	if (tile_map[tile_y][tile_x] == 1 || tile_map[tile_y][tile_x + 1] == 1) return true;

	for(Box &box : boxes) {
		glm::vec2 offset = pos - box.at;
		
		if(offset.x > (-2 * TILE_SIZE) + 1  && offset.x < (2 * TILE_SIZE) - 1) {
			if(offset.y > 0 && offset.y < (2 * TILE_SIZE)) {
				return true;
			}
		}
	}

	for(Crate &box : crates) {
		glm::vec2 offset = pos - box.at;
		
		if(offset.x > (-2 * TILE_SIZE) + 1 && offset.x < (2 * TILE_SIZE) - 1) {
			if(offset.y > 0 && offset.y < (2 * TILE_SIZE)) {
				return true;
			}
		}
	}

	return false;
}

bool PlayMode::collide_above(glm::vec2 pos) {
	glm::vec2 offset = player_at - pos;
	
	if(offset.x > (-2 * TILE_SIZE) && offset.x < (2 * TILE_SIZE)) {
		if(offset.y > 0 && offset.y < (2 * TILE_SIZE)) {
			return true;
		}
	}


	for(Box &box : boxes) {
		offset = box.at - pos;
		
		if(offset.x > (-2 * TILE_SIZE) && offset.x < (2 * TILE_SIZE)) {
			if(offset.y > 0 && offset.y < (2 * TILE_SIZE)) {
				return true;
			}
		}
	}

	for(Crate &box : crates) {
		offset = box.at - pos;
		
		if(offset.x > (-2 * TILE_SIZE) && offset.x < (2 * TILE_SIZE)) {
			if(offset.y > 0 && offset.y < (2 * TILE_SIZE)) {
				return true;
			}
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if(play.pressed && music_pos >= music_len) {
		note_timer = 0.f;
		music_pos = 0;
		music_errors = 0;
	}

	player_vel.x = 0;
	if(left.pressed)  player_vel.x = -PLAYER_SPEED;
	if(right.pressed) player_vel.x =  PLAYER_SPEED;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	play.downs = 0;

	// -------------------------------- PLAYER PHYSICS --------------------------------
	// Check if any blocks are below the player
	bool player_collide_below = collide_below(player_at);

	// If we're on the ground, we can jump
	if(up.pressed && player_collide_below && player_vel.y <= 0.f) {
		player_vel.y = GRAVITY * 0.8f;
	}
	player_vel.y -= GRAVITY * elapsed;

	// Move the player along the x axis and push any boxes in the way
	player_at.x += player_vel.x * elapsed;

	if(player_collide_below) {
		for(Crate &box : crates) {
			glm::vec2 offset = player_at - box.at;

			if(offset.y > (-2 * TILE_SIZE) + 1 && offset.y < (2 * TILE_SIZE) - 1) {
				while(offset.x > (-2 * TILE_SIZE) + 1 && offset.x < (1 * TILE_SIZE) - 1) {
					box.at.x++;
					offset.x--;
				}

				while(offset.x < (2 * TILE_SIZE) - 1 && offset.x > (-1 * TILE_SIZE) + 1) {
					box.at.x--;
					offset.x++;
				}
			}
		}
	}



	// If we're going down into the ground, stop falling
	if(player_collide_below && player_vel.y < 0) {
		player_vel.y = 0;

		while(collide_below(player_at + glm::vec2(0.f, 1.f))) {
			player_at.y++;
		}
	}

	player_at.y += player_vel.y * elapsed;

	player_screen_pos.x = (int)player_at.x;
	player_screen_pos.y = (int)player_at.y;

	// -------------------------------- CRATE PHYSICS --------------------------------
	for(Crate &crate : crates) {
		crate.vel.y -= GRAVITY * elapsed;
	
		// If we're going down into the ground, stop falling
		if(collide_below(crate.at) && crate.vel.y < 0) {
			crate.vel.y = 0;
		}

		crate.at += crate.vel * elapsed;
	}

	// -------------------------------- BLOCK PHYSICS --------------------------------

	for(Box &box : boxes) {
		if(!collide_above(box.at) && box.src.y - box.at.y > 0 ) {
			box.at.y++;
		}

		while(collide_above(box.at) && box.src.y - box.at.y < TILE_SIZE ) {
			box.at.y--;
		}
	}

	// Music playing logic
	note_timer -= elapsed;
	if(note_timer <= 0.f && music_pos < (music_len + 1)) {
		if(music_pos < music_len) {
			note_timer = music_times[music_pos] * 60.f / music_bpm;

			// Check to see if this box is being pushed down, and if so, change the note
			int note_offset = note_error[music_pos];
			for(Box &box : boxes) {
				if(box.music_time == music_pos) {
					if(box.at.y < box.src.y) {
						note_offset -= 1;
						break;
					}
				}
			}

			if(note_offset != 0) music_errors++;


			play_note(music[music_pos] + note_offset);
		}
		music_pos++;

		// If we finish the song with no errors, you win!
		if(music_pos > music_len && music_errors == 0) {
			printf("Correct!\n");
			level++;

			if(level >= LEVEL_CNT) {
				printf("You win!");
				Mode::set_current(nullptr);
			}
			else {
				init_level(level);
			}

		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {	
	sprite_cnt = 0;
	
	ppu.background_color = glm::u8vec4(0x40, 0xc0, 0xff, 0xff);
	
	// Draw the player
	draw_sprite(player_screen_pos.x, player_screen_pos.y + TILE_SIZE - 1, 4, false, 6);

	// Draw the boxes
	for(Box &box : boxes) {
		// Change the palette if the note is playing
		int pallete = 2;
		if(box.music_time == (music_pos - 1)) {
			int error = note_error[box.music_time];
			error -= (box.at.y < box.src.y) ? 1 : 0;
			pallete = (error == 0) ? 4 : 3;
		}

		draw_sprite((int)box.at.x, (int)box.at.y + TILE_SIZE - 1, 2, false, pallete);
	}

	// Draw the crates
	for(Crate &crate : crates) {
		draw_sprite((int)crate.at.x, (int)crate.at.y + TILE_SIZE - 1, 3, false, 1);
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}