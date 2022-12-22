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
	Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Sprite sprite;
} Entity;

#define NUM_BLOCKS_MAP 100

typedef struct Gameplay_Data {
	bool32 IsInitialized;
	Entity Character = {};
	Entity blocks[NUM_BLOCKS_MAP];
	Vector2 Camera_Pos = {};
	Vector2 starting_pos;
	Loaded_Sound MusicSound;
	Sprite character_sprite;
	Texture character_texture;
	Texture block_texture;
	Sprite block_sprite;
} Gameplay_Data;

#define GAME_h
#endif //GAME_H