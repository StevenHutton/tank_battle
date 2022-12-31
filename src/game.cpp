#include <math.h>

#include "audio.h"
#include "game.h"
#include "useful_summerjam.cpp"
#include "vector_summerjam.cpp"
#include "matrix_summer.cpp"
#include "audio.cpp"

static bool32 WasPressed(button_state State)
{
	bool32 Result = ((State.HalfTransitionCount > 1) ||
		(State.HalfTransitionCount == 1) && State.ended_down);
	return Result;
}

typedef struct rect 
{
	f32 left;
	f32 right;
	f32 top;
	f32 bottom;
} rect;

typedef struct Collision 
{
	f32 time;
	Vector2 normal;
} Collision;

typedef struct Penetration 
{
	Vector2 depth;
	Vector2 normal;
} Penetration;

#define PLAYER_GROUND_ACCELERATION 0.8f
#define PLAYER_DRAG 3.0f
#define MAX_VELOCITY 0.8f

static Sprite create_sprite(Texture tex, uint8 frame_count, f32 size_x, f32 size_y, 
                            f32 offset_x = 0.0f, f32 offset_y = 0.0f)
{
	Sprite sprite = {};

	sprite.tex = tex;
	sprite.frame_count = frame_count;
	sprite.current_frame = 0;
	sprite.frame_width = (uint8)sprite.tex.width / sprite.frame_count;
	sprite.frame_height = sprite.frame_width;
	sprite.frame_duration = 1.f/6.f;
	sprite.size.x = size_x;
	sprite.size.y = size_y;
	sprite.offset.x = offset_x;
	sprite.offset.y = offset_y;

	return sprite;
}

static void InitGameObjecets(Game_Memory * memory)
{
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;

	data->Tank.width = 1.0f;
	data->Tank.height = 1.0f;
	data->Tank.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Tank.pos = {5.f, 14.f};

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		data->bullets[i].width = 0.55f;
		data->bullets[i].height = 0.55f;
		data->bullets[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	
	int count = 0;
	uint8* cursor = (uint8 *)data->map_tex.data;
	for (int i = 0; i < data->map_tex.height; i++)
	{
		for (int j = 0; j < data->map_tex.width; j++)
		{
			uint8 r, g, b = 0;
			r = *cursor++;
			g = *(cursor++);
			b = *(cursor++);

			if (r == 0 && g == 0 && b == 0)
			{
				data->blocks[count].pos = {(f32)j, (f32)i};
				data->blocks[count].width = 1.00f;
				data->blocks[count].height = 1.00f;
				data->blocks[count].color = { 1.0f, 1.0f, 1.0f, 1.0f };
				count++;
			}
		}
	}
	data->block_count = count;
	data->Camera_Pos = { 24.0f, 14.5f };
}

static rect GetEntityRect(Entity ent)
{
	rect r;
	
	f32 half_width = ent.width / 2.0f;
	f32 half_height = ent.height / 2.0f;
    
	r.left = ent.pos.x - half_width;
	r.right = ent.pos.x + half_width;
	r.top = ent.pos.y + half_height;
	r.bottom = ent.pos.y - half_height;
	
	return r;
}

static rect GetExpandedRect(Entity ent, f32 halfWidth, f32 halfHeight, f32 modifier = 0.0f)
{
	rect r = GetEntityRect(ent);
	
	r.left = r.left - halfWidth - modifier;
	r.right = r.right + halfWidth + modifier;
	r.top = r.top + halfHeight + modifier;
	r.bottom = r.bottom - halfHeight - modifier;
	
	return r;
}

#define COLLISION_EPSILON 0.00001f

static bool Is_Penetration(Entity e1, Entity e2, Penetration& pen)
{
	f32 offset_x = e1.pos.x - e2.pos.x;
	f32 offset_y = e1.pos.y - e2.pos.y;
    
	f32 offset_mag_x = abs(offset_x);
	f32 offset_mag_y = abs(offset_y);
	
	f32 combined_half_width = (e1.width / 2.f) + (e2.width / 2.f);
	f32 combined_half_height = (e1.height / 2.f) + (e2.height / 2.f);
    
	if (((combined_half_width - offset_mag_x) >= COLLISION_EPSILON) &&
		((combined_half_height - offset_mag_y) >= COLLISION_EPSILON))
	{
		pen.depth.x = combined_half_width - offset_mag_x;
		pen.depth.y = combined_half_height - offset_mag_y;
		
		//move apart in the axis of least penetration
		if (pen.depth.x <= pen.depth.y)
		{
			if (offset_x > 0)
				pen.normal = { 1, 0 };
			else
				pen.normal = { -1, 0 };
		}
		else
		{
			if (offset_y > 0)
				pen.normal = { 0, 1 };
			else
				pen.normal = { 0, -1 };
		}
        
		return true;
	}
	return false;
}

static int get_worst_pen_index(Penetration pens[], int pen_count)
{
	float pen_depth = 0.0f;
	int index = -1;
	for (int i = 0; i < pen_count; i++)
	{
		if (pens[i].depth.x >= pen_depth || pens[i].depth.y >= pen_depth)
		{
			pen_depth = maxf(pens[i].depth.x, pens[i].depth.y);
			index = i;
		}
	}
    
	return index;
}

static bool Is_Collision(Entity e1, Entity e2, Collision& col, f32 dt)
{
	rect r = GetExpandedRect(e2, e1.width/2, e1.height/2, -0.01f);
	
	//check for obvious misses
	if(e1.velocity.x == 0.0f &&
		(e1.pos.x <= r.left || e1.pos.x >= r.right))
		return false;
	
	if(e1.velocity.y == 0.0f &&
		(e1.pos.y <= r.bottom || e1.pos.y >= r.top))
		return false;
	
	f32 left_offset = (r.left - e1.pos.x);
	f32 right_offset = (r.right - e1.pos.x);
	f32 top_offset = (r.top - e1.pos.y);
	f32 bottom_offset = (r.bottom - e1.pos.y);
    
	f32 leftIntercept = left_offset / (e1.velocity.x * dt);
	f32 rightIntercept = right_offset / (e1.velocity.x * dt);
	f32 topIntercept = top_offset / (e1.velocity.y * dt);
	f32 bottomIntercept = bottom_offset / (e1.velocity.y * dt);
	
	f32 x1, x2, y1, y2;
	
	if(leftIntercept < rightIntercept)
	{
		x1 = leftIntercept;
		x2 = rightIntercept;
	}
	else
	{
		x2 = leftIntercept;
		x1 = rightIntercept;
	}
	
	if(topIntercept < bottomIntercept)
	{
		y1 = topIntercept;
		y2 = bottomIntercept;
	}
	else
	{
		y2 = topIntercept;
		y1 = bottomIntercept;
	}
	
	if(x1 > y2 || y1 > x2) return false;
	
	f32 c1 = maxf(x1, y1);
	if(c1 < 0.0f || c1 >= 1.0f) return false;
	f32 c2 = minf(x2, y2);
	if(c2 < 0) return false;
    
	col.time = c1;
	
	if (x1 > y1)
	{
		if (e1.velocity.x < 0)
			col.normal = { 1, 0 };		
		else
			col.normal = { -1, 0 };
	}
	else if (x1 < y1)
	{
		if (e1.velocity.y < 0)
			col.normal = { 0, 1 };
		else
			col.normal = { 0, -1 };
	}
	
	return true;
}

static void resolve_swept_collisions_with_terrain(Entity * ent, Collision cols[], int num_cols)
{
	for (int i = 0; i < num_cols; i++)
	{
		Collision col = cols[i];
		
		ent->velocity.x += abs(ent->velocity.x)
		* col.normal.x * (1 - col.time);
		ent->velocity.y += abs(ent->velocity.y)
		* col.normal.y * (1 - col.time);
	}
}

static bool Is_Penetration_Naive(Entity e1, Entity e2)
{
	rect r1 = GetExpandedRect(e1, e2.width /2.f, e2.height/2.f);
    
	if(e2.pos.x > r1.left && e2.pos.x < r1.right && 
		e2.pos.y > r1.bottom && e2.pos.y < r1.top)
	{
		return true;
	}
	return false;
}

static Quad make_quad(f32 pos_x, f32 pos_y, f32 width, f32 height, float rotation = 0.f, Color color = {1.0f, 1.0f, 1.0f, 1.0f},
	bool flip_x = false)
{
	Quad result;
		
	float st = sinf(rotation);
	float ct = cosf(rotation);
    
	f32 hw = width / 2;
	f32 hh = height / 2;
    
	f32 uvl = 0.0f;
	f32 uvr = 1.0f;
	f32 uvt = 0.0f;
	f32 uvb = 1.0f;
    
	if (flip_x)
	{
		f32 temp = uvl;
		uvl = uvr;
		uvr = temp;
	}
	    
	result.verts[0] = { { (-hw * ct + hh * st) + pos_x, (-hw * -st + hh * ct) + pos_y, 0.0f }, { uvl, uvt }, color };//0 - lt
	result.verts[1] = { { (hw * ct + hh * st) + pos_x, (hw * -st + hh * ct) + pos_y, 0.0f }, { uvr, uvt }, color };//1 - rt
	result.verts[2] = { { (hw * ct + -hh * st) + pos_x, (hw * -st + -hh * ct) + pos_y, 0.0f }, { uvr, uvb }, color };//2 - rb
	result.verts[3] = { { (-hw * ct + -hh * st) + pos_x, (-hw * -st + -hh * ct) + pos_y, 0.0f }, { uvl, uvb }, color };//3 - lb
    
	return result;
}

static Quad make_quad_from_entity(Entity entity)
{
	return make_quad(entity.pos.x, entity.pos.y, 
	                 entity.width, entity.height, entity.rotation, entity.color);
}

static void update_sprite(Gameplay_Data * data, 
                          Sprite * sprite, 
                          f32 delta_time_s,
                          bool should_repeat = true)
{
	sprite->frame_time += delta_time_s;
    
	if (sprite->frame_time > sprite->frame_duration)
	{
		sprite->frame_time -= sprite->frame_duration;
		sprite->current_frame += 1;
		if (sprite->current_frame >= sprite->frame_count)
		{
			if(should_repeat)
			{
				sprite->current_frame = 0;
			}
			else
			{
				sprite->current_frame = sprite->frame_count-1;
			}            
		}
	}
}

#define DRAG_FACTOR 4.0f
#define TANK_SPEED 10.0f
#define TANK_ROTATION_SPEED 0.015f;
#define TURRET_ROTATION_SPEED 0.03f;

extern "C" void UpdateGamePlay(platform_api *PlatformAPI, Game_Memory *memory, Input_State *Input, f32 dt)
{    
    if(WasPressed(Input->Back))
    {
        PlatformAPI->QuitRequested = true;
    }
    
	platform_api Platform = *PlatformAPI;
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;
    
	//TODO(shutton) : maybe put this somewhere else? Are we clearing memory when the .dll is reloaded? Maybe move memory stuff to the platform layer?
    if(!data->IsInitialized)
    {		
		//read_file_result MusicFile = Platform.ReadEntireFile("../assets/music.wav");
		//data->MusicSound = load_wav_from_memory(MusicFile.data);

		data->character_texture = Platform.LoadTexture("../assets/tank_base.png");
		data->turret_texture = Platform.LoadTexture("../assets/tank_turret.png");
		data->block_texture = Platform.LoadTexture("../assets/block.png");
		data->map_tex = Platform.LoadTextureData("../assets/map.bmp");
		data->bullet_texture = Platform.LoadTexture("../assets/bullet.png");
		
		InitGameObjecets(memory);
		//AddPlaySound(&data->MusicSound, true);

        data->IsInitialized = true;
    }

	Vector2 acceleration = {};
	Entity * player = &data->Tank;
	
	if (Input->MoveLeft.ended_down)
	{
		player->rotation -= TANK_ROTATION_SPEED;
	}
	else if (Input->MoveRight.ended_down)
	{
		player->rotation += TANK_ROTATION_SPEED;
	}

	if (Input->ActionLeft.ended_down)
	{
		data->turret_rotation -= TURRET_ROTATION_SPEED;
	}
	else if (Input->ActionRight.ended_down)
	{
		data->turret_rotation += TURRET_ROTATION_SPEED;
	}

	Vector2 north = { 0.f, 1.f };
	Vector2 forward = Rotate(north, player->rotation);

	if (Input->MoveDown.ended_down)
	{
		acceleration = -TANK_SPEED * forward;
	}
	else if (Input->MoveUp.ended_down)
	{
		acceleration = TANK_SPEED * forward;
	}
	else acceleration = { 0.0f, 0.f };
    
	f32 dragX = -DRAG_FACTOR * data->Tank.velocity.x;
	acceleration.x += dragX;

	f32 dragY = -DRAG_FACTOR * data->Tank.velocity.y;
	acceleration.y += dragY;
	
	data->Tank.velocity.x += acceleration.x * dt;
	data->Tank.velocity.y += acceleration.y * dt;
    
	//sweapt phase collision detection - adjust velocity to prevent collision
	Collision collision = {};
	Collision cols[NUM_BLOCKS_MAP];
	int num_cols = 0;
	for (int i = 0; i < NUM_BLOCKS_MAP; i++)
	{
		if (Is_Collision(data->Tank, data->blocks[i], collision, dt))
		{
			cols[num_cols] = collision;
			num_cols++;
		}
	}
	resolve_swept_collisions_with_terrain(&data->Tank, cols, num_cols);
		       
	//update character position	
	data->Tank.pos.x += data->Tank.velocity.x * dt;
	data->Tank.pos.y += data->Tank.velocity.y * dt;

	//penetration phase collision detection
	Penetration pen;
	Penetration pens[20];
	int num_pens = 0;
	for (int i = 0; i < NUM_BLOCKS_MAP; i++)
	{
		if (Is_Penetration(data->Tank, data->blocks[i], pen))
		{
			pens[num_pens] = pen;
			num_pens++;
		}
	}

	int worst_pen_index = get_worst_pen_index(pens, num_pens);
	int loop_count = 0;
	while (worst_pen_index != -1)
	{
		pen = pens[worst_pen_index];		
		data->Tank.pos = data->Tank.pos + (pen.depth * pen.normal);
		if (pen.normal.x == 1)
		{
			data->Tank.velocity.x = maxf(data->Tank.velocity.x, 0.0f);
		}
		else if (pen.normal.x == -1)
		{
			data->Tank.velocity.x = minf(data->Tank.velocity.x, 0.0f);
		}
		
		if (pen.normal.y == 1)
		{
			data->Tank.velocity.y = maxf(data->Tank.velocity.y, 0.0f);
		}
		else if (pen.normal.y == -1)
		{
			data->Tank.velocity.y = minf(data->Tank.velocity.y, 0.0f);
		}
        
		num_pens = 0;
		for (uint32 i = 0; i < NUM_BLOCKS_MAP; i++)
		{
			if (Is_Penetration(data->Tank, data->blocks[i], pen))
			{
				pens[num_pens] = pen;
				num_pens++;
			}
		}
		worst_pen_index = get_worst_pen_index(pens, num_pens);
		loop_count++;
        
		if (loop_count >= 100)
		{
			data->Tank.pos.y += 0.1f;            
			break;
		}
	}

	if (WasPressed(Input->ActionDown))
	{
		//fire bullet
		for (int i = 0; i < NUM_BULLETS; i++)
		{
			if (data->bullets[i].is_active) continue;

			data->bullets[i].is_active = true;
			data->bullets[i].pos = data->Tank.pos;
			Vector2 up = { 0.f, 1.f };
			Vector2 bearing = Rotate(up, data->Tank.rotation + data->turret_rotation);
			data->bullets[i].velocity = (bearing * 10.0f);
			data->bullets[i].velocity += data->Tank.velocity;
			break;
		}
	}

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (!data->bullets[i].is_active) continue;
		for (int j = 0; j < data->block_count; j++)
		{
			if (Is_Penetration_Naive(data->bullets[i], data->blocks[j]))
			{
				data->bullets[i].is_active = false;
			}
		}

		data->bullets[i].pos += (data->bullets[i].velocity * dt);		
	}
}

extern "C" void RenderGameplay(platform_api *PlatformAPI, Game_Memory *memory)
{
	platform_api Platform = *PlatformAPI;
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;
    
	Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Tank), data->character_texture.handle);
	Platform.AddQuadToRenderBuffer(make_quad(data->Tank.pos.x,
	                                         data->Tank.pos.y, 1.0f, 1.0f,
	                                         data->turret_rotation + data->Tank.rotation), data->turret_texture.handle);
	for (int i = 0; i < data->block_count; i++)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->blocks[i]), data->block_texture.handle);
	}

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (!data->bullets[i].is_active) continue;
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->bullets[i]), data->bullet_texture.handle);
	}

	Platform.SetCameraPos(data->Camera_Pos);
}