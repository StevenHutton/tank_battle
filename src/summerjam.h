//summerjam.h

#ifndef SUMMERJAM_H

#include <stdint.h>
#include <intrin.h>

#if SLOW
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#else
#define Assert(...)
#endif

#define InvalidDefaultCase default: { Assert(0); }
#define InvalidCodePath Assert(0);

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef float f32;
typedef int32 bool32;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define QUAD_BUFFER_SIZE 256
#define QUAD_BUFFER_MAX 256

inline uint32
RoundF32ToUint32(f32 Value)
{
    uint32 Result = (uint32)_mm_cvtss_si32(_mm_set_ss(Value));
    return Result;
}

typedef struct Game_Memory
{
	uint64 persistent_memory_size;
	void * persistent_memory;
} Game_Memory;

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

struct read_file_result
{
	uint32 dataSize;
	void* data;
};

typedef read_file_result read_entire_file(char *filename);
typedef Texture load_texture(char *filename);

typedef struct button_state
{
	int32 HalfTransitionCount;
	bool ended_down;
} button_state;

#define NUM_BUTTONS 12

#define NUM_BLOCKS_MAX 1024
#define NUM_BLOCKS_MAP 100
#define NUM_ENEMIES 10

typedef void platform_add_quad_to_render_buffer(Quad quad, uint32 texture_handle);
typedef void set_camera_position(Vector2 pos);

typedef struct
{
	platform_add_quad_to_render_buffer * AddQuadToRenderBuffer;
	set_camera_position * SetCameraPos;
	read_entire_file *ReadEntireFile;    
	load_texture *LoadTexture;
	bool32 QuitRequested;
} platform_api;

inline void *
	Copy(size_t size, void *sourceInit, void *destInit)
{
	uint8 *source = (uint8 *)sourceInit;
	uint8 *dest = (uint8 *)destInit;
	while(size--) {*dest++ = *source++;}
    
	return destInit;
}

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

//todo(shutton) - probably make the quad buffer variable size at runtime
typedef struct QuadBuffer {
	uint32 texture_handle;
	Quad quads[QUAD_BUFFER_SIZE];
	uint32 quad_count = 0;
} QuadBuffer;

typedef struct RenderBuffer {
	Vector2 camera_pos;
	int buffer_count;
	QuadBuffer *quadBuffers[QUAD_BUFFER_MAX];
} RenderBuffer;

RenderBuffer global_render_buffer;

#define SUMMERJAM_H
#endif //SUMMERJAM_H