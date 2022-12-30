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
	float rotation;	
	f32 width;
	f32 height;
	Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool is_active = false;
} Entity;

#define NUM_BLOCKS_MAP 1000
#define NUM_BULLETS 50

typedef struct Gameplay_Data {
	bool32 IsInitialized;
	Entity Tank = {};
	f32 turret_rotation = {};
	Entity blocks[NUM_BLOCKS_MAP];
	Entity bullets[NUM_BULLETS];
	int block_count;
	Vector2 Camera_Pos = {};
	Vector2 starting_pos;
	Loaded_Sound MusicSound;
	Sprite character_sprite;
	Texture character_texture;
	Texture turret_texture;
	Texture block_texture;
	Texture bullet_texture;
	Sprite block_sprite;
	texture_data map_tex;
} Gameplay_Data;

#define GAME_h
#endif //GAME_H