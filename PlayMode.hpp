#include "Mode.hpp"
#include "PPU466.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include "Load.hpp"

#define TILE_SIZE 8
#define LEVEL_CNT 3

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

#define TILEMAP_WIDTH  (SCREEN_WIDTH  / (2 * TILE_SIZE))
#define TILEMAP_HEIGHT (SCREEN_HEIGHT / (2 * TILE_SIZE))

#define PLAYER_SPEED 60.f
#define GRAVITY 180.f

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	void draw_sprite(int x, int y, int size, bool background, int palette);
	void play_note(int note);
	void init_level(int note);

	bool collide_below(glm::vec2 pos);
	bool collide_above(glm::vec2 pos);

	//----- game state -----
	int level = 0;

	// note sound effects
	std::vector<Load< Sound::Sample >> note_sfx;
	int note_base = 57;

	std::vector< int > notes[LEVEL_CNT] = 
		{ 
			{ 60, 64, 67, 70, 72 },
			{ 67, 67, 67, 63, 65, 65, 65, 62},
			{ 67, 70, 68, 60, 60, 58, 68, 67, 62, 63 } 
		};
	std::vector< int > note_errs[LEVEL_CNT] = 
		{ 
			{ 0, 1, 0, 1, 0 },
			{ 0, 1, 0, 0, 1, 0, 1, 0 },
			{ 0, 1, 0, 1, 0, 0, 0, 0, 1, 0 } 
		};
	std::vector< float > note_times[LEVEL_CNT] = 
		{ 
			{ 1.f, 1.f, 1.f, 1.f, 1.f },
			{ .5f, .5f, .5f, 1.5f, .5f, .5f, .5f, 1.5f },
			{ 1.f, 1.f, 1.f, .6f, .4f, 1.f, 1.f, 1.f, .6f, .4f } 
		};

	float music_bpm = 120.f;
	float note_timer = 1.0f;
	int music_errors;
	int music_len;
	int music_pos;
	std::vector< int > music;
	std::vector< int > note_error;
	std::vector< float > music_times;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, play;

	typedef struct Box {
		glm::vec2 at;
		glm::vec2 src;

		int music_time;
	} Box;

	typedef struct Crate {
		glm::vec2 at;
		glm::vec2 vel;
	} Crate;

	std::vector<Box> boxes;
	std::vector<Crate> crates;


	// Data to keep track of sprite drawing
	std::vector<std::vector<int>> sprite_tiles;
	int sprite_cnt;
	
	glm::vec2 player_at;
	glm::vec2 player_vel;
	glm::ivec2 player_screen_pos;

	int levels[LEVEL_CNT][TILEMAP_HEIGHT][TILEMAP_WIDTH] = 
		{
		{
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0}, 
			{0, 0, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
		},
		{
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 0}, 
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  
			{0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0}, 
			{0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0},  
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
		},
		{
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 2, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 1, 1, 1, 1, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
			{0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 2, 0, 0, 0}, 
			{1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 1}, 
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
		}
		};

	int tile_map[TILEMAP_HEIGHT][TILEMAP_WIDTH];

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
