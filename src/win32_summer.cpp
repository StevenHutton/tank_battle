
inline uint32
SafeTruncateUInt64(uint64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

static void 
win32_free_file_memory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

read_file_result win32_read_file(char *fileName)
{
	read_file_result result = {};
	
    if((fileName[0] == '.') &&
       (fileName[1] == '.'))
    {
        char Path[MAX_PATH] = {};
        size_t PathLength = strlen(win32State_.ExeFilePath);
        
        Copy(PathLength, win32State_.ExeFilePath, Path);
        Path[PathLength] = '\\';
        AppendCString(Path + PathLength + 1, fileName);
        fileName = Path;
    }
    
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	
	Assert(fileHandle != INVALID_HANDLE_VALUE);
    
    LARGE_INTEGER FileSize;
	if(GetFileSizeEx(fileHandle, &FileSize))
	{
		uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
		result.data = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(result.data)
		{
			DWORD BytesRead;
			if(ReadFile(fileHandle, result.data, FileSize32, &BytesRead, 0) &&
			   (FileSize32 == BytesRead))
			{
				result.dataSize = FileSize32;
			}
			else
			{
				win32_free_file_memory(result.data);
				result.data = 0;
			}
		}
	}
	return result;
}

//read file and append null char
read_file_result win32_read_file_to_ntchar(char *fileName)
{
	read_file_result result = {};
	
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	
	Assert(fileHandle != INVALID_HANDLE_VALUE)
		
		LARGE_INTEGER FileSize;
	if(GetFileSizeEx(fileHandle, &FileSize))
	{
		uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
		result.data = VirtualAlloc(0, FileSize32+1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(result.data)
		{
			DWORD BytesRead;
			if(ReadFile(fileHandle, result.data, FileSize32, &BytesRead, 0) &&
			   (FileSize32 == BytesRead))
			{
				result.dataSize = FileSize32;
			}
			else
			{
				win32_free_file_memory(result.data);
				result.data = 0;
			}
		}
		char* text = (char*)result.data;
		text += sizeof(char) * FileSize32;
		*text = '\0';
	}
	
	return result;
}

typedef HRESULT WINAPI direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);

static LPDIRECTSOUNDBUFFER Win32SoundBuffer;
static f32 GlobalFrequencyCounter;

static void Win32InitAudio(game_sound_buffer *SoundBuffer, HWND Window)
{
	HMODULE DSoundDll = LoadLibraryA("dsound.dll");
	if(DSoundDll)
	{
		direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundDll, "DirectSoundCreate");
		if(DirectSoundCreate)
		{
			LPDIRECTSOUND DirectSound = 0;
			if(SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
			{
				if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
				{
					// NOTE(kstandbridge): Sound spec
					SoundBuffer->ChannelCount = 2;
					SoundBuffer->SamplesPerSecond = 44100;
					SoundBuffer->BytesPerSample = SoundBuffer->ChannelCount*sizeof(int16);
					SoundBuffer->SizeInBytes = SoundBuffer->SamplesPerSecond*SoundBuffer->BytesPerSample;
                    
					DSBUFFERDESC BufferDescription = {0};
					BufferDescription.dwSize = sizeof(BufferDescription);
					BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                    
					LPDIRECTSOUNDBUFFER PrimaryBuffer;
					DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0);
                    
					WAVEFORMATEX WaveFormat = {0};
					WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
					WaveFormat.nChannels = (WORD)SoundBuffer->ChannelCount;
					WaveFormat.nSamplesPerSec = SoundBuffer->SamplesPerSecond;
					WaveFormat.wBitsPerSample = 16;
					WaveFormat.nBlockAlign = WaveFormat.nChannels*WaveFormat.wBitsPerSample/8;
					WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
                    
					BufferDescription = {0};
					BufferDescription.dwSize = sizeof(BufferDescription);
					BufferDescription.dwFlags = DSBCAPS_GLOBALFOCUS;
					BufferDescription.dwBufferBytes = SoundBuffer->SizeInBytes;
					BufferDescription.lpwfxFormat = &WaveFormat;
                    
                    
					if(!SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &Win32SoundBuffer, 0)))
					{
						// TODO(kstandbridge): Error CreateSoundBuffer failed
						InvalidCodePath;
					}
				}
				else
				{
					// TODO(kstandbridge): Error SetCooperativeLevel failed
					InvalidCodePath;
				}
			}
			else
			{
				// TODO(kstandbridge): Error DirectSoundCreate failed
				InvalidCodePath;
			}
		}
		else
		{
			// TODO(kstandbridge): Error DirectSoundCreate not found
			InvalidCodePath;
		}
        
        
	}
	else
	{
		// TODO(kstandbridge): Error dsound not found
		InvalidCodePath;
	}
}

static FILETIME
Win32GetLastWriteTime(char *filename)
{
	FILETIME lastWriteTime = {0};
    
	WIN32_FILE_ATTRIBUTE_DATA data;
	if(GetFileAttributesExA(filename, GetFileExInfoStandard, &data))
	{
		lastWriteTime = data.ftLastWriteTime;
	}
	else
	{
		// TODO(kstandbridge): Error failed to GetFileAttributesExA
	}
    
	return lastWriteTime;
}

static void
Win32GetFilePaths(Win32_State *win32State)
{
	char Filename[MAX_PATH] = {};
	GetModuleFileName(0, Filename, MAX_PATH);
	size_t Length = strlen(Filename);
    
	char *At = Filename;
	if(*At == '\"')
	{
		++At;
		Length -= 2;
	}
	char *lastSlash = At + Length;
	while(*lastSlash != '\\')
	{
		--Length;
		--lastSlash;
	}
	Copy(Length, At, win32State->ExeFilePath);
	win32State->ExeFilePath[Length] = '\0';
    
	Copy(Length, win32State->ExeFilePath, win32State->DllFullFilePath);
	AppendCString(win32State->DllFullFilePath + Length, "\\game.dll");
    
	Copy(Length, win32State->ExeFilePath, win32State->TempDllFullFilePath);
	AppendCString(win32State->TempDllFullFilePath + Length, "\\game_temp.dll");
    
	Copy(Length, win32State->ExeFilePath, win32State->LockFullFilePath);
	AppendCString(win32State->LockFullFilePath + Length, "\\lock.tmp");
		
	Copy(Length, win32State->ExeFilePath, win32State->BotDll1FilePath);
	AppendCString(win32State->BotDll1FilePath + Length, "\\player1.dll");

	Copy(Length, win32State->ExeFilePath, win32State->BotDll2FilePath);
	AppendCString(win32State->BotDll2FilePath + Length, "\\player2.dll");
}