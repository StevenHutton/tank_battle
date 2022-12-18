//summerjam.h

#ifndef SUMMERJAM_H

#include <stdint.h>
#include <intrin.h>

#if SLOW
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#else
#define Assert(...)
#endif

#define InvalidDefaultCase default: { Assert(0); }
#define InvalidCodePath Assert(0);

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef float f32;
typedef int32 bool32;

#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

inline uint32 
AtomicCompareExchangeU32(uint32 volatile *Value, uint32 New, uint32 Expected)
{
    uint32 Result = _InterlockedCompareExchange((long volatile *)Value, New, Expected);
    
    return Result;
}

inline uint32
AtomicIncrementU32(uint32 volatile *Value)
{
    uint32 Result = _InterlockedIncrement((long volatile *)Value);
    
    return Result;
}

inline uint32
RoundF32ToUint32(f32 Value)
{
    uint32 Result = (uint32)_mm_cvtss_si32(_mm_set_ss(Value));
    return Result;
}

typedef struct Game_Memory
{
	uint64 persistent_memory_size;
	void * persistent_memory;
} Game_Memory;

#define SUMMERJAM_H
#endif //SUMMERJAM_H