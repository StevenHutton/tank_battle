#include "game.h"

void set_button(button_state &button, bool isPressed = true)
{
	if (isPressed)
	{
		button.ended_down = true;
		button.HalfTransitionCount = 1;
	}
	else
	{
		button.ended_down = false;
		button.HalfTransitionCount = 0;
	}
}

bool is_in_block(Gameplay_Data * data, Vector2 point)
{
	for (int i = 0; i < data->block_count; i++)
	{
		if (is_point_collision(data->blocks[i], point)) return true;
	}
	return false;
}

f32 get_bearing(const Vector2 from, const Vector2 to)
{
	Vector2 dir = from - to;
	//atan2 expects y-axis to be down not up or vice versa. Which ever way it is it's the other way to this game.
	f32 bearing = atan2f(-dir.y, dir.x);
	bearing -= 90.0f * ((f32)M_PI / 180.0f); //in Atan2 0 is in the direction of the positive X-axis. In this game 0 degrees is positive Y-axis.

	return bearing;
}

extern "C" Input_State UpdateBot(Gameplay_Data data, int player_number)
{
	Input_State input_state = {};
	Tank * player;
	Tank * enemy;
	if (player_number == 1)
	{
		player = &data.player1;
		enemy = &data.player2;
	}
	else
	{
		player = &data.player2;
		enemy = &data.player1;
	}

	return input_state;
}