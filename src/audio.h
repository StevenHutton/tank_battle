#ifndef AUDIO_H

#include "summerjam.h"

typedef struct Loaded_Sound
{
    int32 sample_count;
    int32 channel_count;
    
    int16* samples;
} Loaded_Sound;

#pragma pack(push, 1)
typedef struct Wav_Header
{
    uint32 riff_id;
    uint32 size;
    uint32 wave_id;
    
} Wav_Header;

#define RIFF_CODE(a, b, c, d) (((uint32)(a) << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))

enum
{
    Wav_chunk_id_fmt  = RIFF_CODE('f', 'm', 't', ' '),
    Wav_chunk_id_riff = RIFF_CODE('R', 'I', 'F', 'F'),
    Wav_chunk_id_wave = RIFF_CODE('W', 'A', 'V', 'E'),
    Wav_chunk_id_data = RIFF_CODE('d', 'a', 't', 'a'),
};

typedef struct
{
    uint32 id;
    uint32 size;
} Wav_Chunk;

typedef struct
{
    uint16 format_tag;
    uint16 num_channels;
    uint32 samples_per_second;
    uint32 avg_bytes_per_second;
    uint16 block_align;
    uint16 bits_per_sample;
    uint16 cb_size;
    uint16 valid_bits_sample;
    uint32 dw_channel_mask;
    uint8 sub_format[16];
    
} Wav_Fmt;
#pragma pack(pop)

typedef struct playing_sound
{
    bool32 Active;
    
    Loaded_Sound *Sound;
    f32 Position; // NOTE(kstandbridge): Sample position
    
    bool32 Looping;
    f32 Volume;
    f32 TargetVolume;
    f32 FadingSpeed;
    f32 Pan;
    f32 SpeedMultiplier;
    
    f32 *SourceX;
    
    struct playing_sound *SyncedSound;
    
    struct playing_sound *NextFree;
} playing_sound;

typedef struct
{
    int32 SizeInBytes;
    int32 ChannelCount;
    int32 SamplesPerSecond;
    int32 BytesPerSample;
    int32 RunningSampleIndex;
    
    int16 *Samples;
    int32 SamplesToWrite;
} game_sound_buffer;


#define AUDIO_H
#endif //AUDIO_H
