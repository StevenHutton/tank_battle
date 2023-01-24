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

Entity screen_space;

static void InitGameObjecets(Game_Memory * memory)
{
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;

	data->player1.ent.width = 1.0f;
	data->player1.ent.height = 1.0f;
	data->player1.ent.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->player1.ent.pos = {5.f, 14.f};
	data->player1.ent.health = 100;
	data->player1.ent.is_active = true;
	data->player1.fire_timer = 0.0f;
		
	data->player2.ent.width = 1.0f;
	data->player2.ent.height = 1.0f;
	data->player2.ent.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->player2.ent.pos = {45.f, 14.f};
	data->player2.ent.health = 100;
	data->player2.ent.is_active = true; 
	data->player2.fire_timer = 0.0f;

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
				data->blocks[count].is_active = true;
				data->blocks[count].health = 200;
				count++;
			}
		}
	}
	data->block_count = count;
	data->Camera_Pos = { 24.0f, 14.5f };

	screen_space.pos = { 24.0f, 14.5f };
	screen_space.width = 55.0f;
	screen_space.height = 35.0f;
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
#define TANK_ROTATION_SPEED 0.015f
#define TURRET_ROTATION_SPEED 0.03f
#define FIRE_COOLDOWN 0.5f

void update_player(Gameplay_Data * data, Tank * tank, Input_State Input, f32 dt)
{	
	if (!tank->ent.is_active) return;

	if (tank->fire_timer >= 0.0f)
		tank->fire_timer -= dt;

	Entity * player = &tank->ent;	
	Vector2 acceleration = {};
		
	if (Input.MoveLeft.ended_down)
	{
		player->rotation -= TANK_ROTATION_SPEED;
	}
	else if (Input.MoveRight.ended_down)
	{
		player->rotation += TANK_ROTATION_SPEED;
	}

	if (Input.ActionLeft.ended_down)
	{
		player->t_rot -= TURRET_ROTATION_SPEED;
	}
	else if (Input.ActionRight.ended_down)
	{
		player->t_rot += TURRET_ROTATION_SPEED;
	}

	Vector2 north = { 0.f, 1.f };
	Vector2 forward = Rotate(north, player->rotation);

	if (Input.MoveDown.ended_down)
	{
		acceleration = -TANK_SPEED * forward;
	}
	else if (Input.MoveUp.ended_down)
	{
		acceleration = TANK_SPEED * forward;
	}
	else acceleration = { 0.0f, 0.f };
    
	f32 dragX = -DRAG_FACTOR * player->velocity.x;
	acceleration.x += dragX;

	f32 dragY = -DRAG_FACTOR * player->velocity.y;
	acceleration.y += dragY;
	
	player->velocity.x += acceleration.x * dt;
	player->velocity.y += acceleration.y * dt;
		
	if (WasPressed(Input.ActionDown) && tank->fire_timer <= 0.0f)
	{
		//fire bullet
		for (int i = 0; i < NUM_BULLETS; i++)
		{
			if (data->bullets[i].is_active) continue;

			data->bullets[i].is_active = true;
			data->bullets[i].pos = player->pos;
			Vector2 up = { 0.f, 1.f };
			Vector2 bearing = Rotate(up, player->rotation + player->t_rot);
			data->bullets[i].pos += bearing * 1.1f;
			data->bullets[i].velocity = (bearing * 15.0f);
			data->bullets[i].velocity += player->velocity;
			break;
		}

		tank->fire_timer += FIRE_COOLDOWN;
	}
		    
	//sweapt phase collision detection - adjust velocity to prevent collision
	Collision collision = {};
	Collision cols[NUM_BLOCKS_MAP];
	int num_cols = 0;
	for (int i = 0; i < NUM_BLOCKS_MAP; i++)
	{
		if (Is_Collision(*player, data->blocks[i], collision, dt))
		{
			cols[num_cols] = collision;
			num_cols++;
		}
	}
	resolve_swept_collisions_with_terrain(player, cols, num_cols);
		       
	//update character position	
	player->pos.x += player->velocity.x * dt;
	player->pos.y += player->velocity.y * dt;

	//penetration phase collision detection
	Penetration pen;
	Penetration pens[20];
	int num_pens = 0;
	for (int i = 0; i < NUM_BLOCKS_MAP; i++)
	{
		if (Is_Penetration(*player, data->blocks[i], pen))
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
		player->pos = player->pos + (pen.depth * pen.normal);
		if (pen.normal.x == 1)
		{
			player->velocity.x = maxf(player->velocity.x, 0.0f);
		}
		else if (pen.normal.x == -1)
		{
			player->velocity.x = minf(player->velocity.x, 0.0f);
		}
		
		if (pen.normal.y == 1)
		{
			player->velocity.y = maxf(player->velocity.y, 0.0f);
		}
		else if (pen.normal.y == -1)
		{
			player->velocity.y = minf(player->velocity.y, 0.0f);
		}
        
		num_pens = 0;
		for (uint32 i = 0; i < NUM_BLOCKS_MAP; i++)
		{
			if (Is_Penetration(*player, data->blocks[i], pen))
			{
				pens[num_pens] = pen;
				num_pens++;
			}
		}
		worst_pen_index = get_worst_pen_index(pens, num_pens);
		loop_count++;
        
		if (loop_count >= 100)
		{
			player->pos.y += 0.1f;            
			break;
		}
	}

	if(player->health <= 0)
		player->is_active = false;
}

extern "C" void UpdateGamePlay(platform_api *PlatformAPI, Game_Memory *memory, Input_State Input, Input_State Input2, f32 dt)
{    
    if(WasPressed(Input.Back))
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

	update_player(data, &data->player1, Input, dt);
	update_player(data, &data->player2, Input2, dt);

	//update bullets
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (!data->bullets[i].is_active) continue;
		for (int j = 0; j < data->block_count; j++)
		{
			if (Is_Penetration_Naive(data->bullets[i], data->blocks[j]) && data->blocks[j].is_active)
			{
				data->bullets[i].is_active = false;
				data->blocks[j].health -= 10;
				if(data->blocks[j].health <= 0)
					data->blocks[j].is_active = false;
			}
			
			if (Is_Penetration_Naive(data->bullets[i], data->player1.ent) && data->player1.ent.is_active)
			{
				data->bullets[i].is_active = false;
				data->player1.ent.health -= 10;
			}

			if (Is_Penetration_Naive(data->bullets[i], data->player2.ent) && data->player2.ent.is_active)
			{
				data->bullets[i].is_active = false;
				data->player2.ent.health -= 10;
			}

			if (!Is_Penetration_Naive(data->bullets[i], screen_space))
				data->bullets[i].is_active = false;
		}

		data->bullets[i].pos += (data->bullets[i].velocity * dt);		
	}
}

extern "C" void RenderGameplay(platform_api *PlatformAPI, Game_Memory *memory)
{
	platform_api Platform = *PlatformAPI;
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;
	
	if (data->player1.ent.is_active)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->player1.ent), data->tank_texture.handle);
		Platform.AddQuadToRenderBuffer(make_quad(data->player1.ent.pos.x, data->player1.ent.pos.y, 1.0f, 1.0f,
		                                         data->player1.ent.t_rot + data->player1.ent.rotation), data->turret_texture.handle);
	}
	if (data->player2.ent.is_active)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->player2.ent), data->tank_texture.handle);
		Platform.AddQuadToRenderBuffer(make_quad(data->player2.ent.pos.x, data->player2.ent.pos.y, 1.0f, 1.0f,
		                                         data->player2.ent.t_rot + data->player2.ent.rotation), data->turret_texture.handle);
	}
	for (int i = 0; i < data->block_count; i++)
	{
		if (!data->blocks[i].is_active) continue;
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->blocks[i]), data->block_texture.handle);
	}

	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (!data->bullets[i].is_active) continue;
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->bullets[i]), data->bullet_texture.handle);
	}

	Platform.SetCameraPos(data->Camera_Pos);
}