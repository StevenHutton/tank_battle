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