#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <random>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	// helper functions 
	void set_tile_to_vector(int tile, std::vector< uint8_t >& bit0_v, std::vector< uint8_t >& bit1_v);
	float dist_from_player(int slime_index);
	glm::vec2 dir_to_player(int slime_index);
	void set_bullet_dir();
	float dist_from_bullet(int slime_index);
	void restart_game();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	float respawn_timer = 0.0f;
	float bullet_timer = 0.0f;

	//game size
	float game_width = 255.0f;
	float game_height = 240.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(game_width / 2.0f, game_height / 2.0f);
	uint8_t dir = 4;

	//bullet stats
	glm::vec2 bullet_at = player_at;
	glm::vec2 bullet_dir;
	float bullet_speed = 80.0f;
	float bullet_duration = 0.5f;
	float bullet_spawn_distance = 2.0f;
	float kill_distance = 6.0f;

	uint32_t score = 0;

	//rng
	std::mt19937 mt; //mersenne twister pseudo-random number generator

	// Slime enemies
	struct Slime {
		glm::vec2 at;
		int state;
	};

	static const uint8_t max_slimes = 48;
	static const uint8_t starting_slimes = 4;
	Slime slimes[max_slimes];
	float min_dist = 50.0f;
	float slime_speed = 10.0f;
	float base_slime_speed = 15.0f;
	float max_slime_speed = 25.0f;
	float respawn_rate = 3.0f;
	uint32_t respawn_num = 4;
	float kill_player_dist = 5.0f;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
