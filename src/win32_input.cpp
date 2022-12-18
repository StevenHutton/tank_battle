//win32_input.cpp

typedef DWORD x_input_get_state(DWORD dwUserIndex, XINPUT_STATE *pState);
static x_input_get_state *Win32XInputGetState;

static void Win32InitXInput()
{
    HMODULE Library = LoadLibraryA("xinput1_4.dll");
    if(Library)
    {
        Win32XInputGetState = (x_input_get_state *)GetProcAddress(Library, "XInputGetState");
        if(!Win32XInputGetState)
        {
            // TODO(kstandbridge): Error loading XInputSetState
            InvalidCodePath;
        }
    }
    else
    {
        // TODO(kstandbridge): Error xinput dll not found
        InvalidCodePath;
    }
}

static f32 Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZone)
{
    f32 Result = 0.0f;
    if(Value < -DeadZone)
    {
        Result = (Value + DeadZone) / (32767.0f - DeadZone);
    }
    else if(Value > DeadZone)
    {
        Result = (Value - DeadZone) / (32767.0f - DeadZone);
    }
    return Result;
}

static inline void Win32ProcessKeyboardButton(button_state *State, bool IsDown)
{
	State->ended_down = IsDown;
	++State->HalfTransitionCount;
}

static void Win32ProcessXInputButton(DWORD XInputButtonState,
                                     button_state *OldState, button_state *NewState,
                                     DWORD ButtonBit)
{
    NewState->ended_down = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->ended_down != NewState->ended_down) ? 1 : 0;
}

static inline bool is_button_pressed(DWORD buttons, DWORD ButtonBit)
{
	return (buttons & ButtonBit) == ButtonBit;
}

static void win32_button_check(XINPUT_GAMEPAD pad,
							   XINPUT_GAMEPAD old_pad,
							   button_state * buttonState,
							   DWORD ButtonBit)
{
	if (is_button_pressed(pad.wButtons, ButtonBit))
	{
		buttonState->ended_down = true;
	}

	if ((is_button_pressed(pad.wButtons, ButtonBit)) != (is_button_pressed(old_pad.wButtons, ButtonBit)))
	{
		buttonState->HalfTransitionCount++;
		buttonState->ended_down = is_button_pressed(pad.wButtons, ButtonBit);
	}
}

static void get_gamepad_input(XINPUT_STATE &XInputState, XINPUT_STATE &Old_XInputState, Input_State &input_result)
{
	XINPUT_GAMEPAD *pad = &XInputState.Gamepad;
	XINPUT_GAMEPAD *old_pad = &Old_XInputState.Gamepad;

	f32 StickAverageX = Win32ProcessXInputStickValue(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	f32 StickAverageY = Win32ProcessXInputStickValue(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        		
	if(StickAverageX < 0.0f)
	{
		pad->wButtons = pad->wButtons | XINPUT_GAMEPAD_DPAD_LEFT;
	}
		
	if(StickAverageX > 0.0f)
	{
		pad->wButtons = pad->wButtons | XINPUT_GAMEPAD_DPAD_RIGHT;	
	}

	if(StickAverageY > 0.0f)
	{
		pad->wButtons = pad->wButtons | XINPUT_GAMEPAD_DPAD_UP;	
	}
		
	if(StickAverageY < 0.0f)
	{
		pad->wButtons = pad->wButtons | XINPUT_GAMEPAD_DPAD_DOWN;
	}

	win32_button_check(*pad, *old_pad, &input_result.MoveLeft, XINPUT_GAMEPAD_DPAD_LEFT);
	win32_button_check(*pad, *old_pad, &input_result.MoveRight, XINPUT_GAMEPAD_DPAD_RIGHT);
	win32_button_check(*pad, *old_pad, &input_result.MoveUp, XINPUT_GAMEPAD_DPAD_UP);
	win32_button_check(*pad, *old_pad, &input_result.MoveDown, XINPUT_GAMEPAD_DPAD_DOWN);

	win32_button_check(*pad, *old_pad, &input_result.ActionUp, XINPUT_GAMEPAD_Y);
	win32_button_check(*pad, *old_pad, &input_result.ActionLeft, XINPUT_GAMEPAD_X);
	win32_button_check(*pad, *old_pad, &input_result.ActionDown, XINPUT_GAMEPAD_A);
	win32_button_check(*pad, *old_pad, &input_result.ActionRight, XINPUT_GAMEPAD_B);
	win32_button_check(*pad, *old_pad, &input_result.LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
	win32_button_check(*pad, *old_pad, &input_result.RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	win32_button_check(*pad, *old_pad, &input_result.Back, XINPUT_GAMEPAD_BACK);
	win32_button_check(*pad, *old_pad, &input_result.Start, XINPUT_GAMEPAD_START);
}