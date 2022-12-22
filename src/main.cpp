#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION 1
#include "includes/stb_image.h"
#include "summerjam.h"
#include "audio.h"
#include "game.h"

static platform_api Platform;

typedef void Update_Gameplay(platform_api *PlatformAPI, Game_Memory *memory, struct Input_State *Input, f32 dt);
typedef void Update_GameAudio(game_sound_buffer *SoundBuffer, f32 DeltaTime);
typedef void Render_Gameplay(platform_api *PlatformAPI, Game_Memory *memory);

typedef struct Win32_State
{
    game_sound_buffer SoundBuffer;
    
    char ExeFilePath[MAX_PATH];
    char DllFullFilePath[MAX_PATH];
    char TempDllFullFilePath[MAX_PATH];
    char LockFullFilePath[MAX_PATH];
    FILETIME LastDLLWriteTime;
    HMODULE AppLibrary;
    Update_Gameplay *UpdateGamePlay;
    Update_GameAudio *UpdateGameAudio;
	Render_Gameplay *RenderGameplay;
} Win32_State;

Win32_State win32State_ = {0};

static bool GlobalRunning = true;

#include "useful_summerjam.cpp"
#include "vector_summerjam.cpp"
#include "matrix_summer.cpp"
#include "win32_summer.cpp"
#include "summer_opengl.cpp"
#include "win32_sound.cpp"
#include "win32_input.cpp"

LRESULT CALLBACK MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch(uMsg)
	{
		case WM_SIZE: 
		{
		} break;
		
		case WM_DESTROY: 
		{
			GlobalRunning = false;
		} break;
		
		case WM_CLOSE: 
		{
			GlobalRunning = false;
		} break;
		
		case WM_ACTIVATEAPP: 
		{
		} break;
		
		case WM_PAINT:
		{
			//if we resize the window we can deal with that here
			PAINTSTRUCT paint;
			BeginPaint(hwnd, &paint);
			EndPaint(hwnd, &paint);
		} break;
		
		default:
		{
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		} break;
	}
	return result;
}

static WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

static void ToggleFullscreen(HWND Window)
{
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static void Win32ProcessPendingMessages(Input_State &input_result)
{
	MSG message;
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{	
		switch(message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
			
			case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
				uint32 VKCode = (uint32)message.wParam;
                bool32 AltKeyWasDown = (message.lParam & (1 << 29));
                bool32 WasDown = ((message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    switch(VKCode)
                    {
                        case VK_F4:
                        {
                            if(IsDown && AltKeyWasDown)
                            {
                                GlobalRunning = false;
                            }
                        } break;
                        case VK_RETURN:
                        {
                            if(IsDown && AltKeyWasDown)
                            {
                                if(message.hwnd)
                                {
                                    ToggleFullscreen(message.hwnd);
                                }
                            }
                        } break;
                        
						case 'W':       { Win32ProcessKeyboardButton(&input_result.MoveUp, IsDown); } break;
						case 'A':       { Win32ProcessKeyboardButton(&input_result.MoveLeft, IsDown); } break;
						case 'S':       { Win32ProcessKeyboardButton(&input_result.MoveDown, IsDown); } break;
						case 'D':       { Win32ProcessKeyboardButton(&input_result.MoveRight, IsDown); } break;
						case 'Q':       { Win32ProcessKeyboardButton(&input_result.LeftShoulder, IsDown); } break;
						case 'E':       { Win32ProcessKeyboardButton(&input_result.RightShoulder, IsDown); } break;
						case VK_UP:     { Win32ProcessKeyboardButton(&input_result.ActionUp, IsDown); } break;
						case VK_LEFT:   { Win32ProcessKeyboardButton(&input_result.ActionLeft, IsDown); } break;
                        //case VK_DOWN:   { Win32ProcessKeyboardButton(&Controller.ActionDown, IsDown); } break;
						case VK_RIGHT:  { Win32ProcessKeyboardButton(&input_result.ActionRight, IsDown); } break;
                        case VK_ESCAPE: { Win32ProcessKeyboardButton(&input_result.Back, IsDown); } break;
                        //case VK_SPACE:  { Win32ProcessKeyboardButton(&Controller.Start, IsDown); } break;
                        
                        // NOTE(kstandbridge): We use space to jump so that would be A (button button) on controller
						case VK_SPACE:  { Win32ProcessKeyboardButton(&input_result.ActionDown, IsDown); } break;
                    }
                }
			} break;
			
			default:
            {
				TranslateMessage(&message);
				DispatchMessage(&message);
			} break;
		}
	}
}

static Texture load_image_with_stbi(char * file_name)
{	
	int channels;
	Texture tex;
    
	void * texture_data = (void *)stbi_load(file_name, &tex.width, &tex.height, &channels, 0);
	tex.handle = ogl_init_texture(tex.width, tex.height, channels, texture_data);
	stbi_image_free(texture_data);
    
	return tex;
}

static Texture Get_Texture(char * path)
{
	char * base_path = win32State_.ExeFilePath;
	char Path[MAX_PATH] = {};
	size_t PathLength = strlen(base_path);

	Copy(PathLength, base_path, Path);
	AppendCString(Path + PathLength, "\\");
	PathLength += 1;
	AppendCString(Path + PathLength, path);

	return load_image_with_stbi(Path);
}

static void Gameplay_dll_reload(Win32_State *win32State)
{
	FILETIME NewDLLWriteTime = Win32GetLastWriteTime(win32State->DllFullFilePath);
	if(CompareFileTime(&NewDLLWriteTime, &win32State->LastDLLWriteTime) != 0)
	{
		WIN32_FILE_ATTRIBUTE_DATA Ignored;
		if(!GetFileAttributesExA(win32State->LockFullFilePath, GetFileExInfoStandard, &Ignored))
		{            
			if(win32State->AppLibrary && !FreeLibrary(win32State->AppLibrary))
			{
				// TODO(kstandbridge): Error freeing app library
			}
			win32State->AppLibrary = 0;
			win32State->UpdateGamePlay = 0;
			win32State->UpdateGameAudio = 0;
			win32State->RenderGameplay = 0;
            
			if(CopyFileA(win32State->DllFullFilePath, win32State->TempDllFullFilePath, false))
			{
				win32State->AppLibrary = LoadLibraryA(win32State->TempDllFullFilePath);
				if(win32State->AppLibrary)
				{
					win32State->UpdateGamePlay = (Update_Gameplay *)GetProcAddress(win32State->AppLibrary, "UpdateGamePlay");
					win32State->UpdateGameAudio = (Update_GameAudio *)GetProcAddress(win32State->AppLibrary, "UpdateGameAudio");
					win32State->RenderGameplay = (Render_Gameplay *)GetProcAddress(win32State->AppLibrary, "RenderGameplay");
					if(!win32State->UpdateGamePlay || !win32State->UpdateGameAudio ||
                       !win32State->RenderGameplay)
					{
						// TODO(kstandbridge): Error AppUpdateFrame
						if(!FreeLibrary(win32State->AppLibrary))
						{
							// TODO(kstandbridge): Error freeing app library
						}
					}
					win32State->LastDLLWriteTime = NewDLLWriteTime;                        
				}
			}
			else
			{
				// TODO(kstandbridge): Error copying temp dll
			}
		}
	}
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) /
                  (f32)GlobalFrequencyCounter);
    return Result;
}

inline LARGE_INTEGER
Win32GetWallClock()
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow)
{
    Win32InitXInput();
    
    LARGE_INTEGER FrequencyCounterLarge;
    QueryPerformanceFrequency(&FrequencyCounterLarge);
    GlobalFrequencyCounter = (f32)FrequencyCounterLarge.QuadPart;
    
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

	Platform.ReadEntireFile = win32_read_file;
	Platform.LoadTexture = Get_Texture;
	Platform.AddQuadToRenderBuffer = add_quad_to_render_buffer;
	Platform.SetCameraPos = SetCameraPosition;
    
    WNDCLASS windowClass = {};
	
	windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	windowClass.lpfnWndProc = MainWindowCallback;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "SummerjamWindow";
	
	if (!RegisterClass(&windowClass))
		return 0;
    
	HWND window = CreateWindowEx(0, windowClass.lpszClassName,
                                 "Summer Jam Building Blocks Handmade",
                                 WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 0, 0, hInstance, 0);
    
	if(!window)
	{
		//todo - some error handling goes here
		return(0);
	}
    
    ToggleFullscreen(window);
    
    Game_Memory memory = {};
	memory.persistent_memory_size = Megabytes(16);
	memory.persistent_memory = VirtualAlloc(0, memory.persistent_memory_size,
                                            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);    
	
    Win32_State *win32State = &win32State_;    
    Win32GetFilePaths(win32State);    
	open_gl_init(win32State, window);
	
	//LOAD MAP	
	Gameplay_Data * game_data = (Gameplay_Data *)memory.persistent_memory;
	char Path[MAX_PATH] = {};
	size_t PathLength = strlen(win32State->ExeFilePath);
	Copy(PathLength, win32State->ExeFilePath, Path);
	AppendCString(Path + PathLength, "\\..\\assets\\map.bmp");
	int channels;
	game_data->map_data = (void *)stbi_load(Path, &game_data->map_width, &game_data->map_height, &channels, 0);    
	
	HDC windowDC = GetDC(window);
    LARGE_INTEGER LastPerformanceCounter = Win32GetWallClock();
    
    Win32InitAudio(&win32State->SoundBuffer, window);
    Win32ClearSoundBuffer(&win32State->SoundBuffer);
    if(!SUCCEEDED(Win32SoundBuffer->Play(0, 0, DSBPLAY_LOOPING)))
    {
        InvalidCodePath;
    }
    win32State->SoundBuffer.Samples = (int16 *)VirtualAlloc(0, win32State->SoundBuffer.SizeInBytes, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
	XINPUT_STATE XInputState = {};
	XINPUT_STATE Old_XInputState = {};
	Input_State input_state = {};
    
    // NOTE: Otherwise Sleep will be ignored for requests less than 50? citation needed
    UINT MinSleepPeriod = 1;
    timeBeginPeriod(MinSleepPeriod);
    f32 TargetSeconds = 1.0f / 60.0f;
    
    while(GlobalRunning)
	{
		Gameplay_dll_reload(win32State);   
		//input handling
		for (int i = 0; i < NUM_BUTTONS; i++)
		{
			input_state.Buttons[i].HalfTransitionCount = 0;
		}
		Win32ProcessPendingMessages(input_state);
		Old_XInputState = XInputState;
		DWORD xinput_result = Win32XInputGetState(0, &XInputState);
		if (xinput_result == ERROR_SUCCESS)
		{
			get_gamepad_input(XInputState, Old_XInputState, input_state);
		}
        if(win32State->UpdateGamePlay && win32State->RenderGameplay)
        {
			win32State->UpdateGamePlay(&Platform, &memory, &input_state, TargetSeconds);
			win32State->RenderGameplay(&Platform, &memory);
        }
        
		win32_ogl_render(windowDC, &global_render_buffer);
		reset_quad_buffers(&global_render_buffer);
        
		Win32UpdateAudioThread(win32State);
        
        if(Platform.QuitRequested)
        {
            break;
        }        
        
        f32 FrameSeconds = Win32GetSecondsElapsed(LastPerformanceCounter, Win32GetWallClock());
        
        if(FrameSeconds < TargetSeconds)
        {
            DWORD Miliseconds = (DWORD)(1000.0f * (TargetSeconds - FrameSeconds));
            if(Miliseconds > 0)
            {
                Sleep(Miliseconds);
            }
            
            FrameSeconds = Win32GetSecondsElapsed(LastPerformanceCounter, Win32GetWallClock());
            
            while(FrameSeconds < TargetSeconds)
            {
                FrameSeconds = Win32GetSecondsElapsed(LastPerformanceCounter, Win32GetWallClock());
                _mm_pause();
            }
        }
        
        LastPerformanceCounter = Win32GetWallClock();
	}
	ReleaseDC(window, windowDC);
    
    return 0;
}