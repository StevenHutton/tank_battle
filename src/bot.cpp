#include "game.h"



extern "C" Input_State UpdateBot(Gameplay_Data data, int player_number)
{
	Input_State input_state = {};
	Entity * player;
	Entity * enemy;
	if (player_number == 1)
	{
		player = &data.Tank;
		enemy = &data.Tank2;
	}
	else
	{
		player = &data.Tank2;
		enemy = &data.Tank;
	}

	input_state.MoveUp.ended_down = true;
	input_state.MoveUp.HalfTransitionCount = 1;

	
	input_state.ActionLeft.ended_down = true;
	input_state.ActionLeft.HalfTransitionCount = 1;

	return input_state;
}