#include <math.h>

#include "audio.h"
#include "game.h"
#include "useful_summerjam.cpp"
#include "vector_summerjam.cpp"
#include "matrix_summer.cpp"
#include "audio.cpp"
#include "collision.cpp"

static bool32 WasPressed(button_state State)
{
	bool32 Result = ((State.HalfTransitionCount > 1) ||
		(State.HalfTransitionCount == 1) && State.ended_down);
	return Result;
}

#define PLAYER_GROUND_ACCELERATION 0.8f
#define PLAYER_DRAG 3.0f
#define MAX_VELOCITY 0.8f

static void InitGameObjecets(Game_Memory * memory)
{
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;

	data->Tank.width = 1.0f;
	data->Tank.height = 1.0f;
	data->Tank.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Tank.pos = {5.f, 14.f};
		
	data->Tank2.width = 1.0f;
	data->Tank2.height = 1.0f;
	data->Tank2.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Tank2.pos = {45.f, 14.f};

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
		data->tank_texture = Platform.LoadTexture("../assets/tank_base.png");
		data->turret_texture = Platform.LoadTexture("../assets/tank_turret.png");
		data->block_texture = Platform.LoadTexture("../assets/block.png");
		data->map_tex = Platform.LoadTextureData("../assets/map.bmp");
		data->bullet_texture = Platform.LoadTexture("../assets/bullet.png");
		
		InitGameObjecets(memory);
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
			data->bullets[i].pos += bearing * 0.8f;
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
    
	Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Tank), data->tank_texture.handle);
	Platform.AddQuadToRenderBuffer(make_quad(data->Tank.pos.x, data->Tank.pos.y, 1.0f, 1.0f,
	                                         data->turret_rotation + data->Tank.rotation), data->turret_texture.handle);
	
	Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Tank2), data->tank_texture.handle);
	Platform.AddQuadToRenderBuffer(make_quad(data->Tank2.pos.x, data->Tank2.pos.y, 1.0f, 1.0f,
	                                         data->turret_rotation2 + data->Tank2.rotation), data->turret_texture.handle);
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