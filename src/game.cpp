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

#define BLOCK_SIZE 0.1f

#define CASTLE_SIZE 0.4f
#define MOUNTAIN_WIDTH 6.4f
#define MOUNTAIN_HEIGHT 3.2f
#define CLOUDS_WIDTH 6.4f
#define CLOUDS_HEIGHT 1.6f
#define TREES_WIDTH 3.2f
#define TREES_HEIGHT 3.2f
#define UNDERWOOD_WIDTH 6.4f
#define UNDERWOOD_HEIGHT 1.6f
#define TITLE_HEIGHT 0.8f
#define TITLE_WIDTH 1.6f

static bool Is_On_Ground = false;
static bool Is_Ghost_Mode = false;
static bool Is_Smashing = false;

#define GRAVITY -2.0f
#define PLAYER_GROUND_ACCELERATION 0.8f
#define PLAYER_DRAG 3.0f
#define PLAYER_AIR_ACCELERATION 0.8f
#define JUMP_IMPULSE 1.0f
#define ENEMY_VELOCITY 0.1f
#define MAX_VELOCITY 0.8f

static void InitParallaxObjects(Entity *backgrounds, Vector2 pos, f32 width, f32 height)
{
	backgrounds[0].pos.x = pos.x;
	backgrounds[0].pos.y = pos.y;
	backgrounds[0].width = width;
	backgrounds[0].height = height;
	backgrounds[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	backgrounds[0].is_facing_right = true;
    
	backgrounds[1].pos.x = pos.x + width;
	backgrounds[1].pos.y = pos.y;
	backgrounds[1].width = width;
	backgrounds[1].height = height;
	backgrounds[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	backgrounds[1].is_facing_right = true;
}

static int global_map_blocks = 0;

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

	data->kid_sprite = create_sprite(data->kid_texture, 1, 0.075f, 0.15f, 0.f, 0.035f);
	data->kid_walk_sprite = create_sprite(data->kid_walk_texture, 4, 0.075f, 0.15f, 0.f, 0.035f);
	data->smash_sprite = create_sprite(data->smash_texture, 2, 0.075f, 0.15f, 0.f, 0.035f);
	data->block_place_sprite = create_sprite(data->block_place_texture, 5, 0.1f, 0.1f, 0.f, 0.f);

	data->Character.width = 0.05f;
	data->Character.height = 0.08f;
	data->Character.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Character.sprite = data->kid_walk_sprite;
	data->Camera_Pos = { 0.3f, 0.0f };

	Vector2 pos = { 0.0f, (data->map_height * BLOCK_SIZE) - 1.0f };
	Vector2 castle_pos = {};
	uint8 * map = (uint8*)data->map_data;
	int block_count = 0;
	int enemy_count = 0;

	for (int i = 0; i < data->map_height; i++)
	{
		for (int j = 0; j < data->map_width; j++)
		{
			uint8 r, g, b = 0;
			r = *map++;
			g = *(map++);
			b = *(map++);
			
			if (r == 0 && g == 0 && b == 0)
			{
				data->Blocks[block_count].pos = pos;
				data->Blocks[block_count].width = BLOCK_SIZE;
				data->Blocks[block_count].height = BLOCK_SIZE;
				data->Blocks[block_count].color = { 1.0f, 1.0f, 1.0f, 1.0f };
                
				if(pos.x > castle_pos.x)
					castle_pos = pos;
                
				block_count++;
			}
            
			if (r == 255 && g == 0 && b == 0)
			{
				data->SideEnemies[enemy_count].entity.pos = pos;
				data->SideEnemies[enemy_count].entity.width = BLOCK_SIZE * 0.8f;
				data->SideEnemies[enemy_count].entity.height = BLOCK_SIZE * 0.8f;
				data->SideEnemies[enemy_count].entity.velocity.x = -ENEMY_VELOCITY;
				data->SideEnemies[enemy_count].initial_pos = data->SideEnemies[enemy_count].entity.pos;
				data->SideEnemies[enemy_count].entity.color = { 1.0f, 1.0f, 1.0f, 1.0f };
                
				enemy_count++;
			}
            
			if (r == 0 && g == 255 && b == 0)
			{
				data->starting_block = pos;
				data->starting_pos = pos;
				data->Character.pos = pos;
				data->starting_pos.y += 2 * BLOCK_SIZE;
				data->Character.pos.y += 2 * BLOCK_SIZE;
			}
            
			pos.x += BLOCK_SIZE;
		}
		pos.y -= BLOCK_SIZE;
		pos.x = 0.0f;
	}
	
	data->Castle.pos = castle_pos;
	data->Castle.pos.x -= ((CASTLE_SIZE / 2) - (BLOCK_SIZE / 2));
	data->Castle.pos.y += ((CASTLE_SIZE / 2) + (BLOCK_SIZE / 2));
	data->Castle.width = CASTLE_SIZE;
	data->Castle.height = CASTLE_SIZE;
	data->Castle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
	pos.x = data->Camera_Pos.x;
	pos.y = data->Camera_Pos.y + 0.3f;
	data->Title.pos = pos;
	data->Title.width = TITLE_WIDTH;
	data->Title.height = TITLE_HEIGHT;
	data->Title.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Title.is_facing_right = true;
    
	data->EndScreen.width = BLOCK_SIZE * 24;
	data->EndScreen.height = BLOCK_SIZE * 12;
	data->EndScreen.color = { 1.0f, 1.0f, 1.0f, 0.0f };
	data->EndScreen.is_facing_right = true;
	
	pos.x = data->Camera_Pos.x;
	pos.y = data->Camera_Pos.y + -0.3f;
	InitParallaxObjects(data->Mountains, pos, MOUNTAIN_WIDTH, MOUNTAIN_HEIGHT);
    
	pos.x = data->Camera_Pos.x;
	pos.y = data->Camera_Pos.y + 0.1f;
	InitParallaxObjects(data->Clouds, pos, CLOUDS_WIDTH, CLOUDS_HEIGHT);
    
	pos.x = data->Camera_Pos.x;
	pos.y = 0.0f;
	InitParallaxObjects(data->Trees, pos, TREES_WIDTH, TREES_HEIGHT);
    
	pos.x = data->Camera_Pos.x + 3.7f;
	pos.y = data->Camera_Pos.y - 0.3f;
	InitParallaxObjects(data->Underwood, pos, UNDERWOOD_WIDTH, UNDERWOOD_HEIGHT);
    
	for(int i = 0; i < 3; ++i)
	{
		data->Stars[i].width = BLOCK_SIZE * 0.3f;
		data->Stars[i].height = BLOCK_SIZE * 0.3f;
		data->Stars[i].color = {1.0f, 1.0f, 1.0f, 0.0f};
	}
	data->draw_title = true;
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
	r.top = r.top + halfHeight - modifier;
	r.bottom = r.bottom - halfHeight + modifier;
	
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

static void get_entity_penetrations_with_terrain(Gameplay_Data * data, Entity * ent,  Penetration pens[], int &num_pens, bool is_enemy = false)
{
	for (uint32 i = 0; i < data->GlobalBlockCount; i++)
	{
		Penetration pen;
		if (Is_Penetration(*ent, data->Blocks[i], pen))
		{
			pens[num_pens] = pen;
			num_pens++;
			if(is_enemy && i >= NUM_BLOCKS_MAP && data->Spikes[i].width <= 0.0f)
			{
				data->Spikes[i] = data->Blocks[i];
				data->Blocks[i] = {};
			}
		}
	}
}

static void resolve_entity_penetrations_with_terrain(Gameplay_Data * data,
                                                     Entity * entity, 
                                                     Penetration pens[], int num_pens)
{
	int worst_pen_index = get_worst_pen_index(pens, num_pens);
	int loop_count = 0;
	while (worst_pen_index != -1)
	{
		Penetration pen = pens[worst_pen_index];			
		entity->pos = entity->pos + (pen.depth * pen.normal);
		if (pen.normal.x == 1)
		{
			entity->velocity.x = maxf(entity->velocity.x, 0.0f);
		}
		else if (pen.normal.x == -1)
		{
			entity->velocity.x = minf(entity->velocity.x, 0.0f);
		}
		
		if (pen.normal.y == 1)
		{
			entity->velocity.y = maxf(entity->velocity.y, 0.0f);
		}
		else if (pen.normal.y == -1)
		{
			entity->velocity.y = minf(entity->velocity.y, 0.0f);
		}
        
		num_pens = 0;
		for (uint32 i = 0; i < data->GlobalBlockCount; i++)
		{
			if (Is_Penetration(*entity, data->Blocks[i], pen))
			{
				pens[num_pens] = pen;
				num_pens++;
			}
		}
		worst_pen_index = get_worst_pen_index(pens, num_pens);
		loop_count++;
        
		if (loop_count >= 100)
		{
			entity->pos.y += BLOCK_SIZE;
            
			break;
		}
	}
}

static bool Is_Collision(Entity e1, Entity e2, Collision& col, f32 dt)
{
	rect r = GetExpandedRect(e2, e1.width / 4.0f, e1.height / 4.0f);
	
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

static void get_swept_collisions_with_terrain(Gameplay_Data * data,
                                              Entity * entity, 
                                              int * num_cols,
                                              f32 dt,
                                              Collision cols[],
                                              bool is_enemy = false)
{
	Collision collision = {};
	*num_cols = 0;
    
	for (uint32 i = 0; i < data->GlobalBlockCount; i++)
	{
		if (Is_Collision(*entity, data->Blocks[i], collision, dt))
		{
			cols[*num_cols] = collision;
			(*num_cols)++;
			if(is_enemy && i >= NUM_BLOCKS_MAP && data->Spikes[i].width <= 0.0f)
			{
				data->Spikes[i] = data->Blocks[i];
				data->Blocks[i] = {};
			}
		}
	}
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

static bool Is_Collision_Blocks(Entity e1, Entity e2)
{
	rect r1 = GetEntityRect(e1);
    
	if(e2.pos.x > r1.left && e2.pos.x < r1.right && 
		e2.pos.y > r1.bottom && e2.pos.y < r1.top)
	{
		return true;
	}
	return false;
}

static void update_parallax(Entity *backgrounds, Vector2 old_camera_pos, Vector2 new_camera_pos, Vector2 velocity, f32 width, uint8 &current_background)
{
	backgrounds[current_background].pos.x += ((new_camera_pos.x - old_camera_pos.x) * velocity.x);
	backgrounds[current_background].pos.y += ((new_camera_pos.y - old_camera_pos.y) * velocity.y);
    
	int8 Signal = 1;
	if(new_camera_pos.x < backgrounds[current_background].pos.x)
	{
		Signal = -1;
	}
	backgrounds[1-current_background].pos.x = backgrounds[current_background].pos.x + (width * Signal);
	backgrounds[1-current_background].pos.y = backgrounds[current_background].pos.y;
    
	if((new_camera_pos.x > (backgrounds[current_background].pos.x + (width/2))) ||
		(new_camera_pos.x < backgrounds[current_background].pos.x - (width/2)))
	{
		current_background = 1 - current_background;
	}
}

static void update_parallax(Entity *entity, Vector2 old_camera_pos, Vector2 new_camera_pos, Vector2 velocity)
{
	entity->pos.x += ((new_camera_pos.x - old_camera_pos.x) * velocity.x);
	entity->pos.y += ((new_camera_pos.y - old_camera_pos.y) * velocity.y);
}

static Vector2 Global_Camera_Offset = {};

static void update_camera(Gameplay_Data * data, f32 dt, bool button_up_pressed, bool button_down_pressed)
{
	Vector2 old_camera_pos;
	old_camera_pos.x = data->Camera_Pos.x;
	old_camera_pos.y = data->Camera_Pos.y;
    
	data->Camera_Pos.x = data->Character.pos.x + 0.2f;
	data->Camera_Pos.y = data->Character.pos.y + Global_Camera_Offset.y + 0.15f;
    
	// Look up and down
	if(abs(data->Character.velocity.x) < 0.01f && abs(data->Character.velocity.y) < 0.01f)
	{
		if(button_up_pressed)
		{
			if(Global_Camera_Offset.y < 0.25f)
			{
				data->camera_velocity.y = 0.5f;
			}
			else
			{
				data->camera_velocity.y = 0.0f;
			}
		}
		if(button_down_pressed)
		{
			if(Global_Camera_Offset.y > -0.55f)
			{
				data->camera_velocity.y = -0.5f;
			}
			else
			{
				data->camera_velocity.y = 0.0f;
			}
		}
		if(!button_up_pressed && !button_down_pressed)
		{
			if(Global_Camera_Offset.y > 0.01f)
			{
				data->camera_velocity.y = -0.5f;
			}
			else if(Global_Camera_Offset.y < -0.01f)
			{
				data->camera_velocity.y = 0.5f;
			}
			else
			{
				data->camera_velocity.y = 0.0f;
			}
		}
		Global_Camera_Offset.y += data->camera_velocity.y * dt;
	}
    
	static uint8 current_mountain = 0;
	Vector2 velocity = {0.8f, 0.8f};
	update_parallax(data->Mountains, old_camera_pos, data->Camera_Pos, velocity, MOUNTAIN_WIDTH, current_mountain);
    
	static uint8 current_cloud = 0;
	velocity = {0.7f, 0.7f};
	update_parallax(data->Clouds, old_camera_pos, data->Camera_Pos, velocity, CLOUDS_WIDTH, current_cloud);
    
	static uint8 current_tree = 0;
	velocity = {0.6f, 0.6f};
	update_parallax(data->Trees, old_camera_pos, data->Camera_Pos, velocity, TREES_WIDTH, current_tree);
    
	static uint8 current_underwood = 0;
	velocity = {-0.2f, -0.25};
	update_parallax(data->Underwood, old_camera_pos, data->Camera_Pos, velocity, UNDERWOOD_WIDTH, current_underwood);
    
	velocity = {-3.0f, 0.0f};
	update_parallax(&data->Title, old_camera_pos, data->Camera_Pos, velocity);
}

static Quad make_quad(f32 pos_x, f32 pos_y, f32 width, f32 height, Color color = {1.0f, 1.0f, 1.0f, 1.0f},
	bool flip_x = false)
{
	Quad result;
    
	f32 hw = width / 2;
	f32 hh = height / 2;
    
	f32 left = pos_x - hw;
	f32 right = pos_x + hw;
	f32 top = pos_y + hh;
	f32 bottom = pos_y - hh;
    
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
    
	result.verts[0] = { { left, top, 0.0f }, { uvl, uvt }, color };//0 - lt
	result.verts[1] = { { right, top, 0.0f }, { uvr, uvt }, color };//1 - rt
	result.verts[2] = { { right, bottom, 0.0f }, { uvr, uvb }, color };//2 - rb
	result.verts[3] = { { left, bottom, 0.0f }, { uvl, uvb }, color };//3 - lb
    
	return result;
}

static Quad make_quad_from_entity(Entity entity)
{
	return make_quad(entity.pos.x, entity.pos.y, 
	                 entity.width, entity.height, entity.color, !entity.is_facing_right);
}

static Quad make_quad_from_entity_sprite(Entity entity)
{
	Quad result;
	Sprite sprite = entity.sprite;
	Texture tex = entity.sprite.tex;
	Vector2 pos = entity.pos + sprite.offset;
    
	f32 hw = sprite.size.x / 2;
	f32 hh = sprite.size.y / 2;
    
	f32 left = pos.x - hw;
	f32 right = pos.x + hw;
	f32 top = pos.y + hh;
	f32 bottom = pos.y - hh;
    
	f32 uvl = 0.0f;
	f32 uvr = 1.0f;
	f32 uvt = 0.0f;
	f32 uvb = 1.0f;
    
	uvl = (sprite.current_frame * sprite.frame_width) / (float)tex.width;
	uvr = ((sprite.current_frame+1) * sprite.frame_width) / (float)tex.width;
	
	if (!entity.is_facing_right)
	{
		f32 temp = uvl;
		uvl = uvr;
		uvr = temp;
	}
    
	result.verts[0] = { { left, top, 0.0f }, { uvl, uvt }, entity.color };//0 - lt
	result.verts[1] = { { right, top, 0.0f }, { uvr, uvt }, entity.color };//1 - rt
	result.verts[2] = { { right, bottom, 0.0f }, { uvr, uvb }, entity.color };//2 - rb
	result.verts[3] = { { left, bottom, 0.0f }, { uvl, uvb }, entity.color };//3 - lb
    
	return result;
}

static void PlaceBlock(Gameplay_Data * data, Vector2 pos)
{
	++data->GlobalBlockCount;
	Assert(data->GlobalBlockCount < NUM_BLOCKS_MAX);
    
	data->Blocks[data->GlobalBlockCount-1].pos = pos;
	data->Blocks[data->GlobalBlockCount-1].width = BLOCK_SIZE;
	data->Blocks[data->GlobalBlockCount-1].height = BLOCK_SIZE;
	data->Blocks[data->GlobalBlockCount-1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data->Blocks[data->GlobalBlockCount-1].sprite = data->block_place_sprite;
	data->Blocks[data->GlobalBlockCount-1].is_facing_right = data->Character.is_facing_right;
    
	data->Character.pos.y = pos.y + (BLOCK_SIZE/1.5f);
	if(pos.x < data->Character.pos.x)
	{
		data->Character.pos.x = pos.x + (BLOCK_SIZE/4);
	}
	else
	{
		data->Character.pos.x = pos.x - (BLOCK_SIZE/4);
	}
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
            
			if(sprite->tex.handle == data->smash_sprite.tex.handle)
			{
				Is_Smashing = false;
			}
		}
	}
}

static void SpawnPlayer(Gameplay_Data *data)
{
	data->starting_block.x -= BLOCK_SIZE;
	PlaceBlock(data, data->starting_block);
	data->starting_pos.x -= BLOCK_SIZE;
	data->Character.pos = data->starting_pos;
	data->Character.velocity.x = 0.0f;
	data->Character.velocity.y = 0.0f;
    
	// Reset enemies
	uint8 * map = (uint8*)data->map_data;
	int enemy_count = 0;
	for (int i = 0; i < data->map_height; i++)
	{
		for (int j = 0; j < data->map_width; j++)
		{
			uint8 r, g, b = 0;
			r = *map++;
			g = *(map++);
			b = *(map++);
			
			if (r == 255 && g == 0 && b == 0)
			{
				data->SideEnemies[enemy_count].entity.pos = data->SideEnemies[enemy_count].initial_pos;
				data->SideEnemies[enemy_count].entity.velocity.x = -ENEMY_VELOCITY;
				data->SideEnemies[enemy_count].entity.velocity.y = 0.0f;
				data->SideEnemies[enemy_count].entity.is_facing_right = false;
				data->SideEnemies[enemy_count].entity.color = { 1.0f, 1.0f, 1.0f, 1.0f };
                
				enemy_count++;
			}
            
		}
	}
}

static void update_enemy(Gameplay_Data *data, Enemy *enemy, f32 dt)
{
	enemy->entity.velocity.y += GRAVITY * dt;
        
	//sweapt phase collision detection - adjust velocity to prevent collision
	Collision collision = {};
	Collision cols[NUM_BLOCKS_MAX];
	int num_cols = 0;
	get_swept_collisions_with_terrain(data, &enemy->entity, &num_cols, dt, cols, true);
	resolve_swept_collisions_with_terrain(&enemy->entity, cols, num_cols);
    
	//update character position	
	enemy->entity.pos.x += enemy->entity.velocity.x * dt;
	enemy->entity.pos.y += enemy->entity.velocity.y * dt;
    
	//penetration phase collision check - resolve any collisions not caught by swept phase
	Penetration pens[20];
	int num_pens = 0;
	get_entity_penetrations_with_terrain(data, &enemy->entity, pens, num_pens, true);
	resolve_entity_penetrations_with_terrain(data, &enemy->entity, pens, num_pens);
    
	if(abs(enemy->entity.velocity.x) < 0.001f)
	{
		enemy->entity.is_facing_right = !enemy->entity.is_facing_right;
		if(enemy->entity.is_facing_right)
		{
			enemy->entity.velocity.x = ENEMY_VELOCITY;
		}
		else
		{
			enemy->entity.velocity.x = -ENEMY_VELOCITY;
		}
	}
}

static void SomeExampleThread(void *Data)
{
    Gameplay_Data *GameData = (Gameplay_Data *)Data;
    GameData;
}


extern "C" void UpdateGamePlay(platform_api *PlatformAPI, Game_Memory *memory, Input_State *Input, f32 dt)
{
    
    if(WasPressed(Input->Back))
    {
        PlatformAPI->QuitRequested = true;
    }
    
	platform_api Platform = *PlatformAPI;
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;
#if 0
    // NOTE(kstandbridge): This is how you would run something in a seperate thread,
    // the game will stall until all these threads finish each frame
    // use Platform.BackgroundWorkQueue if the thread is not frame dependant
    Platform.AddWorkEntry(Platform.PerFrameWorkQueue, SomeExampleThread, data);
    
    // NOTE(kstandbridge): You can block this thread and wait for all threads to complete by calling CompleteAllWork and specifying which queue
    Platform.CompleteAllWork(Platform.PerFrameWorkQueue);
#endif
    
    if(!data->IsInitialized)
    {
        data->GlobalBlockCount = NUM_BLOCKS_MAP;
        
		read_file_result JumpFile = Platform.ReadEntireFile("../assets/jump_explosion.wav");
		data->JumpSound = load_wav_from_memory(JumpFile.data);
		
		read_file_result MusicFile = Platform.ReadEntireFile("../assets/music.wav");
		data->MusicSound = load_wav_from_memory(MusicFile.data);

		read_file_result deathFile = Platform.ReadEntireFile("../assets/player_ouch.wav");
		data->DeathSound = load_wav_from_memory(deathFile.data);
		
		read_file_result deathFile2 = Platform.ReadEntireFile("../assets/bug_ouch.wav");
		data->DeathSound2 = load_wav_from_memory(deathFile2.data);

		data->grass_tex = Platform.LoadTexture("../assets/grass.png");
		data->castle_tex = Platform.LoadTexture("../assets/castle.png");
    
		data->kid_texture = Platform.LoadTexture("../assets/kid.png");
		data->kid_walk_texture = Platform.LoadTexture("../assets/kid_walk.png");
		data->smash_texture = Platform.LoadTexture("../assets/smash_animation.png");
		data->block_place_texture = Platform.LoadTexture("../assets/block_placement_animation.png");
	
		data->mountains_tex = Platform.LoadTexture("../assets/mountains_back.png");
		data->clouds_tex = Platform.LoadTexture("../assets/clouds_back.png");    
		data->trees_tex = Platform.LoadTexture("../assets/trees.png");    
		data->underwood_tex = Platform.LoadTexture("../assets/underwood_a.png");
		data->block_tex = Platform.LoadTexture("../assets/block.png");
		data->enemy_tex = Platform.LoadTexture("../assets/enemy.png");
		data->star_tex = Platform.LoadTexture("../assets/star.png");
		data->spike_tex = Platform.LoadTexture("../assets/spike.png");
		data->title_tex = Platform.LoadTexture("../assets/title.png");
		data->end_tex = Platform.LoadTexture("../assets/end.png");
		
		InitGameObjecets(memory);

		AddPlaySound(&data->MusicSound, true);
        SpawnPlayer(data);
        data->IsInitialized = true;
    }
    
	if(WasPressed(Input->LeftShoulder))
	{
        SpawnPlayer(data);
	}
    
	if(WasPressed(Input->RightShoulder))
	{
		data->GlobalBlockCount = NUM_BLOCKS_MAP;
	}
    
	if(WasPressed(Input->ActionUp))
	{
		Is_Ghost_Mode = !Is_Ghost_Mode;
	}
	Vector2 acceleration = {};
	if (Is_Ghost_Mode)
	{
		if (Input->MoveLeft.ended_down)
		{
			acceleration.x = -5.0f;
		}
		if (Input->MoveRight.ended_down)
		{
			acceleration.x = 5.0f;
		}
		
		if (Input->MoveUp.ended_down)
		{
			acceleration.y = 5.0f;
		}
		if (Input->MoveDown.ended_down)
		{
			acceleration.y = -5.0f;
		}			
        
		f32 DragX = -PLAYER_DRAG*data->Character.velocity.x;
		acceleration.x += DragX;
		data->Character.velocity.x += acceleration.x * dt;
        
		f32 DragY = -PLAYER_DRAG*data->Character.velocity.y;
		acceleration.y += DragY;
		data->Character.velocity.y += acceleration.y * dt;
		
		data->Character.pos.x += data->Character.velocity.x * dt;
		data->Character.pos.y += data->Character.velocity.y * dt;
		update_camera(data, dt, false, false);
		return;
	}
    
	if (Input->MoveLeft.ended_down)
	{
		acceleration.x = -1.0f;
		data->Character.is_facing_right = false;
	}
	else if (Input->MoveRight.ended_down)
	{
		acceleration.x = 1.0f;
		data->Character.is_facing_right = true;
	}
	else acceleration.x = 0.0f;
    
	if (abs(data->Character.velocity.x) <= 0.01f)
		data->Character.velocity.x = 0.0f;
	
	if (Is_On_Ground && WasPressed(Input->ActionDown))
	{
		data->Character.velocity.y = JUMP_IMPULSE;
		Vector2 pos;
		pos.x = round_to_dp(data->Character.pos.x, 1);
		pos.y = round_to_dp(data->Character.pos.y, 1);
		PlaceBlock(data, pos);
        data->Character.sprite = data->smash_sprite;
        Is_Smashing = true;
		AddPlaySound(&data->JumpSound, false);
	}
	
	data->Character.velocity.y += GRAVITY * dt;
	f32 Drag = -PLAYER_DRAG * data->Character.velocity.x;
	acceleration.x += Drag;
	data->Character.velocity.x += acceleration.x * dt;
    
	//sweapt phase collision detection - adjust velocity to prevent collision
	Collision collision = {};
	Collision cols[NUM_BLOCKS_MAX];
	int num_cols = 0;
	get_swept_collisions_with_terrain(data, &data->Character, &num_cols, dt, cols);
	resolve_swept_collisions_with_terrain(&data->Character, cols, num_cols);
    
    // Prevent character to remain still when bumping its head
    for (int i = 0; i < num_cols; i++)
	{
		Collision col = cols[i];
        if(col.normal.y == 1.f)
        {
            data->Character.velocity.y = 0.0f;
            break;
        }
    }
    
    // Limit y velocity
    if(data->Character.velocity.y < -MAX_VELOCITY)
    {
        data->Character.velocity.y = -MAX_VELOCITY;
    }
    
	//update character position	
	data->Character.pos.x += data->Character.velocity.x * dt;
	data->Character.pos.y += data->Character.velocity.y * dt;
    
	//penetration phase collision check - resolve any collisions not caught by swept phase
	Penetration pens[20];
	int num_pens = 0;
	get_entity_penetrations_with_terrain(data, &data->Character, pens, num_pens);
	if (num_pens == 0)
		Is_On_Ground = false;
	else
	{
		for (int i = 0; i < num_pens; i++)
		{
			if (pens[i].normal.y > 0.0f)
				Is_On_Ground = true;
		}
	}
	
	resolve_entity_penetrations_with_terrain(data, &data->Character, pens, num_pens);
    
    // Prevent character to remain still when bumping its head
    for (int i = 0; i < num_pens; i++)
	{
		Penetration pen = pens[i];
        if(pen.normal.y == -1)
        {
            data->Character.velocity.y = 0.0f;
            break;
        }
    }
	
	// Enemy collision check
	Penetration penetration = {};
    for(int i = 0; i < NUM_ENEMIES; i++)
    {        
        if (Is_Penetration(data->Character, data->SideEnemies[i].entity, penetration))
        {
            SpawnPlayer(data);			
			AddPlaySound(&data->DeathSound2, false);
        }
    }
    
    // Spikes collision check
    for(int i = 0; i < NUM_BLOCKS_MAX; i++)
    {
        if (Is_Penetration(data->Character, data->Spikes[i], penetration))
        {
            SpawnPlayer(data);			
			AddPlaySound(&data->DeathSound, false);
        }
    }
	
	// Castle collision check
	if (Is_Penetration(data->Character, data->Castle, penetration))
	{
		if(data->StarsCount < 2)
        {
            SpawnPlayer(data);
			data->StarsCount++;
        }
        else
        {
            data->EndScreen.color = {1.0f, 1.0f, 1.0f, 1.0f};
        }
	}
        
    // Die if fall too low
    if(data->Character.pos.y < -0.5f)
    {
        SpawnPlayer(data);
		AddPlaySound(&data->DeathSound, false);
    }
    
    // Placing stars
    for(int i = 0; i < 3; i++)
    {
        data->Stars[i].color = { 1.0f, 1.0f, 1.0f, 0.0f };
        data->Stars[i].pos.x = data->Character.pos.x + ((i-1) * BLOCK_SIZE / 3);
        data->Stars[i].pos.y = data->Character.pos.y + (f32)(BLOCK_SIZE / 0.9f);
    }
    for(int i = 0; i < data->StarsCount; i++)
    {
        data->Stars[i].color = {1.0f, 1.0f, 1.0f, 1.0f};
    }
    
	update_sprite(data, &data->Character.sprite, dt);
    if(!Is_Smashing)
    {
        // Change to stading still texture
        if(abs(data->Character.velocity.x) < 0.01f)
        {
            data->Character.sprite.current_frame = 0;
            data->Character.sprite.tex = data->kid_sprite.tex;
        }
        else
        {
            data->Character.sprite.tex = data->kid_walk_sprite.tex;
        }
        Is_Smashing = false;
    }
    
    for(uint32 i = NUM_BLOCKS_MAP + 1; i < data->GlobalBlockCount; i++)
    {
        update_sprite(data, &data->Blocks[i].sprite, dt, false);
    }
    
	update_camera(data, dt, Input->MoveUp.ended_down, Input->MoveDown.ended_down);
    for(int i = 0; i < NUM_ENEMIES; i++)
    {
        update_enemy(data, &data->SideEnemies[i], dt);
    }
    data->EndScreen.pos = data->Camera_Pos;

	if (data->Camera_Pos.x > 2.0f)
	{
		data->draw_title = false;
	}
}

extern "C" void RenderGameplay(platform_api *PlatformAPI, Game_Memory *memory)
{
	platform_api Platform = *PlatformAPI;
	Gameplay_Data * data = (Gameplay_Data *)memory->persistent_memory;
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Mountains[0]), data->mountains_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Mountains[1]), data->mountains_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Clouds[0]), data->clouds_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Clouds[1]), data->clouds_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Trees[0]), data->trees_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Trees[1]), data->trees_tex.handle);
    
    for(int i = 0; i < NUM_ENEMIES; i++)
    {
        if(data->SideEnemies[i].entity.width > 0.0f)
        {
            Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->SideEnemies[i].entity), data->enemy_tex.handle);
        }
    }
	
	for (uint32 i = 0; i < 3; i++)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Stars[i]), data->star_tex.handle);
	}
	
	for (uint32 i = 0; i < NUM_BLOCKS_MAP + 1; i++)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Blocks[i]), data->grass_tex.handle);
	}

	for (uint32 i = NUM_BLOCKS_MAP + 1; i < data->GlobalBlockCount; i++)
	{
		Platform.AddQuadToRenderBuffer(make_quad_from_entity_sprite(data->Blocks[i]), data->Blocks[i].sprite.tex.handle);
	}
	
	if (data->draw_title)
		Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Title), data->title_tex.handle);

    Platform.AddQuadToRenderBuffer(make_quad_from_entity_sprite(data->Character), data->Character.sprite.tex.handle);
    
    for (uint32 i = 0; i < NUM_BLOCKS_MAX; i++)
	{
        if(data->Spikes[i].width > 0.0f)
        {
            Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Spikes[i]), data->spike_tex.handle);
        }
	}
    
	Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Castle), data->castle_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Underwood[0]), data->underwood_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->Underwood[1]), data->underwood_tex.handle);
    Platform.AddQuadToRenderBuffer(make_quad_from_entity(data->EndScreen), data->end_tex.handle);
}