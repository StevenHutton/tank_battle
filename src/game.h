//game.h
#ifndef GAME_H

#include "summerjam.h"
#include "audio.h"

typedef struct Vector2 {
	f32 x;
	f32 y;
} Vector2;

typedef union Color
{
	f32 color[4];
	struct
	{
		f32 r, g, b, a;
	} compontents;
} Color;

typedef struct Vertex {
	float verts[3];
	float uv[2];
	Color color = { 1.0f, 1.0f, 1.0f, 1.0f };
} Vertex;

typedef struct Quad {
	Vertex verts[4];
} Quad;

typedef struct Texture {
	int width, height;
	uint32 handle;
} Texture;

#define QUAD_BUFFER_SIZE 256
#define QUAD_BUFFER_MAX 256

//todo(shutton) - probably make the quad buffer variable size at runtime
typedef struct QuadBuffer {
	uint32 texture_handle;
	Quad quads[QUAD_BUFFER_SIZE];
	uint32 quad_count = 0;
} QuadBuffer;

typedef struct RenderBuffer {
	int buffer_count;
	QuadBuffer *quadBuffers[QUAD_BUFFER_MAX];
} RenderBuffer;

RenderBuffer global_render_buffer;
Vector2 Global_Camera_Position;

inline void *
Copy(size_t size, void *sourceInit, void *destInit)
{
    uint8 *source = (uint8 *)sourceInit;
    uint8 *dest = (uint8 *)destInit;
    while(size--) {*dest++ = *source++;}
    
    return destInit;
}

typedef void platform_add_quad_to_render_buffer(Quad quad, uint32 texture_handle);

struct read_file_result
{
	uint32 dataSize;
	void* data;
};
typedef read_file_result read_entire_file(char *filename);
typedef Texture load_texture(char *filename);

typedef struct
{
	platform_add_quad_to_render_buffer * AddQuadToRenderBuffer;
    
    read_entire_file *ReadEntireFile;    
	load_texture *LoadTexture;
    bool32 QuitRequested;    
} platform_api;

typedef struct button_state
{
	int32 HalfTransitionCount;
	bool ended_down;
} button_state;

#define NUM_BUTTONS 12

#define NUM_BLOCKS_MAX 1024
#define NUM_BLOCKS_MAP 100
#define NUM_ENEMIES 10

typedef struct Input_State
{    
	union
	{
		button_state Buttons[NUM_BUTTONS];
		struct
		{
			button_state MoveUp;
			button_state MoveDown;
			button_state MoveLeft;
			button_state MoveRight;
            
			button_state ActionUp;
			button_state ActionDown;
			button_state ActionLeft;
			button_state ActionRight;
            
			button_state LeftShoulder;
			button_state RightShoulder;
            
			button_state Back;
			button_state Start;
		};
	};
} controller_input;

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