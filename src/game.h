//game.h
#ifndef GAME_H

#include "summerjam.h"
#include "audio.h"

typedef struct Sprite {
	uint8 frame_width, frame_height, frame_count, current_frame;
	f32 frame_duration, frame_time;
	Vector2 size;
	Texture tex;
	Vector2 offset;
} Sprite;

typedef struct Entity {
	Vector2 pos;
	Vector2 velocity;
	
	f32 width;
	f32 height;
	bool is_facing_right;
	Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Sprite sprite;
} Entity;

typedef struct Enemy {
	Vector2 initial_pos;
	Entity entity;
} Enemy;

typedef struct Gameplay_Data {	
	bool32 IsInitialized;	
	Entity Character = {};
	Entity Blocks[NUM_BLOCKS_MAX];
	Entity Castle = {};
	Entity Title = {};
	Entity EndScreen = {};
	Entity Mountains[2];
	Entity Clouds[2];
	Entity Trees[2];
	Entity Underwood[2];
	Entity Stars[3];
	Entity Spikes[NUM_BLOCKS_MAX] = {};
	Enemy SideEnemies[NUM_ENEMIES] = {};
	Vector2 Camera_Pos = {};    
	Loaded_Sound JumpSound;
	Loaded_Sound MusicSound;
	Loaded_Sound DeathSound;
	Loaded_Sound DeathSound2;
    
	uint32 GlobalBlockCount = NUM_BLOCKS_MAP;
    
	Sprite kid_sprite;
	Sprite kid_walk_sprite;
	Sprite smash_sprite;
	Sprite block_place_sprite;
	
	Texture kid_texture;
	Texture kid_walk_texture;
	Texture smash_texture;
	Texture block_place_texture;

	Texture grass_tex;
	Texture block_tex;
	Texture castle_tex;
	Texture mountains_tex;
	Texture clouds_tex;
	Texture trees_tex;
	Texture underwood_tex;
	Texture enemy_tex;
	Texture star_tex;
	Texture spike_tex;
	Texture title_tex;
	Texture end_tex;
    
	Vector2 starting_pos;
	Vector2 starting_block;
    
	int map_width;
	int map_height;
	void * map_data;
    
	Vector2 camera_velocity = {};
    
	uint8 StarsCount = 0;
	bool draw_title = true;
} Gameplay_Data;

#define GAME_h
#endif //GAME_H