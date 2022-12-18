

playing_sound PlayingSounds[32];
int32 NextPlayingSound;
playing_sound *FirstFreeSound;
int32 ImmutableSoundCount;

typedef struct Riff_Iterator
{
    uint8* at;
    uint8* stop;
} Riff_Iterator;

static Riff_Iterator
parse_chunk_at(void *at, void* stop) {
    
    Riff_Iterator iter;    
    iter.at = (uint8*)at;
    iter.stop = (uint8*)stop;    
    return iter;
}

static bool32
is_riff_iterator_valid(Riff_Iterator iter) {    
    return iter.at < iter.stop;
}

static Riff_Iterator
next_chunk(Riff_Iterator iter) {    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    uint32 size = (chunk->size+1) & ~1;
    iter.at += sizeof(Wav_Chunk) + size;
    return iter;
}

static void *
get_chunk_data(Riff_Iterator iter) {    
    return iter.at + sizeof(Wav_Chunk);
}

static uint32
get_type(Riff_Iterator iter) {    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    return chunk->id;
}

static uint32
get_chunk_data_size(Riff_Iterator iter) {    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    return chunk->size;
}

static Loaded_Sound
load_wav_from_memory(void *Data) {
    
    Loaded_Sound result = {0};    
    Wav_Header *header = (Wav_Header*)Data;
    Assert(header->riff_id == Wav_chunk_id_riff);
    Assert(header->wave_id == Wav_chunk_id_wave);
    
    uint32 channel_count = 0;
    uint32 sample_data_size = 0;
    int16 *sample_data = 0;
    for (Riff_Iterator iter = parse_chunk_at(header+1, (uint8*)(header+1) + header->size - 4);
		is_riff_iterator_valid(iter);
		iter = next_chunk(iter))
	{
        switch(get_type(iter))
		{            
            case Wav_chunk_id_fmt:
			{                
                Wav_Fmt *fmt = (Wav_Fmt*) get_chunk_data(iter);
                Assert(fmt->format_tag == 1); //pcm
                Assert(fmt->samples_per_second == 44100);
                Assert(fmt->bits_per_sample == 16);
                Assert(fmt->block_align == (sizeof(int16)*fmt->num_channels));
                Assert(fmt->num_channels == 1 || fmt->num_channels == 2);                
                channel_count = fmt->num_channels;                
            } break;            
            case Wav_chunk_id_data: 
			{                
                sample_data = (int16*)get_chunk_data(iter);
                sample_data_size = get_chunk_data_size(iter);
            } break;            
        }
    }
    
    Assert(channel_count && sample_data && sample_data_size);
    result.channel_count = channel_count;
    uint32 sample_count = sample_data_size / (channel_count*sizeof(int16));
    
    if (channel_count == 1)
	{        
        result.samples = sample_data;        
    } 
	else if (channel_count == 2)
	{        
        result.samples = sample_data;
    } 
	else 
	{
        Assert(!"Invalid channel count in Wav file");
    }
    
    result.channel_count = channel_count;
    result.sample_count = sample_count;
    
    return result;
}

static void
StopSound(playing_sound *Sound)
{
    Sound->Active = false;
    Sound->NextFree = FirstFreeSound;
    FirstFreeSound = Sound;
}

static playing_sound *
AddPlaySound(Loaded_Sound *Sound, bool32 Looping)
{
    playing_sound *Result = FirstFreeSound;
    
    if(Result)
    {
        FirstFreeSound = Result->NextFree;
    }
    else
    {
        Assert(NextPlayingSound >= 0 && NextPlayingSound < ArrayCount(PlayingSounds));
        Result = PlayingSounds + NextPlayingSound++;
        
        if(NextPlayingSound >= ArrayCount(PlayingSounds))
        {
            NextPlayingSound = ImmutableSoundCount;
        }
        Assert(NextPlayingSound >= 0 && NextPlayingSound < ArrayCount(PlayingSounds));
    }
    *Result = {0};
    Result->Active = true;
    Result->Volume = 1;
    Result->TargetVolume = 1;
    Result->FadingSpeed = 3;
    Result->Sound = Sound;
    Result->Position = 0;
    Result->Pan = 0;
    Result->Looping = Looping;
    Result->SpeedMultiplier = 1.0f;
    
    return Result;
}

extern "C" void UpdateGameAudio(game_sound_buffer *SoundBuffer, f32 DeltaTime)
{
    for(playing_sound *Sound = PlayingSounds;
        Sound != PlayingSounds + ArrayCount(PlayingSounds);
        ++Sound)
    {
        if(!Sound->Active) continue;
        
        Assert(Sound->Sound);        
        if(Sound->SyncedSound)
        {
            Sound->Position = Sound->SyncedSound->Position;
            Sound->SpeedMultiplier = Sound->SyncedSound->SpeedMultiplier;
        }
    }
    
    int16 *At = SoundBuffer->Samples;
    for(int32 Index = 0; Index < SoundBuffer->SamplesToWrite; ++Index)
    { // TODO(kstandbridge): To be more cache friendly, we should iterate over the samples inside each sound
        f32 LeftSample = 0;
        f32 RightSample = 0;
        
        for(playing_sound *Sound = PlayingSounds;
            Sound != PlayingSounds + ArrayCount(PlayingSounds);
            ++Sound)
        {
            if(!Sound->Active) continue;
            Assert(Sound->Sound);
            if(!Sound->Sound) continue;
            
            Sound->Volume = move_towards(Sound->Volume, Sound->TargetVolume, Sound->FadingSpeed/SoundBuffer->SamplesPerSecond);
            
            if(Sound->SourceX) Sound->Pan = move_towards(Sound->Pan, (*Sound->SourceX)*0.005f, 10.0f/SoundBuffer->SamplesPerSecond);
            
            if(Sound->Volume)
            {
                
                int32 Sample = clamp_min_max(0, (int32)(Sound->Position), Sound->Sound->sample_count - Sound->Sound->channel_count - 1);
                
                f32 Frac = Sound->Position - (f32)Sample;
                Sample *= Sound->Sound->channel_count;
                int32 NextSample = Sample + Sound->Sound->channel_count - 1 + 1;
                if(NextSample >= Sound->Sound->sample_count*Sound->Sound->channel_count)
                {
                    if(Sound->Looping) NextSample -= Sound->Sound->sample_count;
                    else NextSample = Sample;
                }
                
                f32 LeftSoundSample_1 = (f32)Sound->Sound->samples[Sample];
                f32 LeftSoundSample_2 = (f32)Sound->Sound->samples[NextSample];
                
                f32 RightSoundSample_1 = (f32)Sound->Sound->samples[Sample     + Sound->Sound->channel_count - 1];
                f32 RightSoundSample_2 = (f32)Sound->Sound->samples[NextSample + Sound->Sound->channel_count - 1];
                
                f32 LeftSoundSample = lerp(LeftSoundSample_1, Frac, LeftSoundSample_2)*Sound->Volume;
                f32 RightSoundSample = lerp(RightSoundSample_1, Frac, RightSoundSample_2)*Sound->Volume;
                
                LeftSample += (LeftSoundSample*clampf_min_max(0.0f, (1.0f-Sound->Pan), 1.0f) + 
                               RightSoundSample*clampf_min_max(0.0f, (-Sound->Pan), 1.0f));
                RightSample += (RightSoundSample*clampf_min_max(0.0f, (1.0f+Sound->Pan), 1.0f) + 
                                LeftSoundSample*clampf_min_max(0.0f, (Sound->Pan), 1.0f));
            }
            
            Sound->Position += Sound->SpeedMultiplier;
            if(Sound->Position >= Sound->Sound->sample_count)
            {
                if(Sound->Looping) Sound->Position -= Sound->Sound->sample_count;
                else StopSound(Sound);
            }
        }
        
        f32 Min = (f32)INT16_MIN;
        f32 Max = (f32)INT16_MAX;
        
        *At++ = (int16)clampf_min_max(Min, LeftSample*0.5f, Max);
        *At++ = (int16)clampf_min_max(Min, RightSample*0.5f, Max);
    }
}
