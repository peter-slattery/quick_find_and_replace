#ifndef GS_LANGUAGE_H

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#include <windows.h>
// TODO(Peter): Get rid of stdio
#include <stdio.h>

#elif defined(__APPLE__) && defined(__MAC__)
// TODO(Peter): 

#else // Std lib
#include <stdlib.h>

#endif

#include <math.h>

#define internal static
#define local_persist static
#define global_variable static


#if !defined(GS_TYPES)

#define GSINT64(s) (s) ## L
#define GSUINT64(s) (s) ## UL

typedef signed char    b8;
typedef short int      b16;
typedef int            b32;
typedef long long int  b64;

typedef unsigned char          u8;
typedef unsigned short int     u16;
typedef unsigned int           u32;
typedef unsigned long long int u64;

typedef signed char   s8;
typedef short int     s16;
typedef int           s32;
typedef long long int s64;

typedef float  r32;
typedef double r64;

#define INT8_MIN   (-128)
#define INT16_MIN  (-32767-1)
#define INT32_MIN  (-2147483647-1)
#define INT64_MIN  (-GSINT64(9223372036854775807)-1)

#define INT8_MAX   (127)
#define INT16_MAX  (32767)
#define INT32_MAX  (2147483647)
#define INT64_MAX  (GSINT64(9223372036854775807))

#define UINT8_MAX  (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (GSUINT64(18446744073709551615))

#define FLOAT_MIN  (1.175494351e-38F)
#define FLOAT_MAX  (3.402823466e+38F)
#define DOUBLE_MIN (2.2250738585072014e-308)
#define DOUBLE_MAX (1.7976931348623158e+308)

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define M_PI  3.14159265359
#define PI_OVER_180 0.01745329251f

#define GS_TYPES
#endif


#ifdef DEBUG

static void DebugPrint(char* Format, ...);

#if !defined(Assert)
// NOTE(peter): this writes to address 0 which is always illegal and will cause a crash
#define Assert(expression) if(!(expression)){ *((int *)0) = 5; }
#endif

#define InvalidCodePath Assert(0);
#define STBI_ASSERT(x) Assert(x)

// TODO(Peter): this isn't great 
#define DEBUG_TRACK_SCOPE(a)


#define TestClean(v, c) SuccessCount += Test(v, c, &TestCount)
internal s32
Test(b32 Result, char* Description, s32* Count)
{
    char* Passed = (Result ? "Success" : "Failed");
    if (!Result)
        DebugPrint("%s:\n................................................%s\n\n", Description, Passed);
    
    *Count = *Count + 1;
    return (Result ? 1 : 0);
}

#else

#define Assert(expression)
#define InvalidCodePath
#define DEBUG_TRACK_SCOPE(a)

#endif // DEBUG

#ifndef GS_LANGUAGE_MATH

#define GSZeroMemory(mem, size) GSZeroMemory_((u8*)(mem), (size)) 
static void
GSZeroMemory_ (u8* Memory, s32 Size)
{
    for (int i = 0; i < Size; i++) { Memory[i] = 0; }
}

#define GSCopyMemory(from, to, size) GSCopyMemory_((u8*)from, (u8*)to, size)
static void
GSCopyMemory_ (u8* From, u8* To, s32 Size)
{
    for (int i = 0; i < Size; i++) { To[i] = From[i]; }
}

#define GSMinDef(type) static type GSMin(type A, type B) { return (A < B ? A : B); }
GSMinDef(s8)
GSMinDef(s16)
GSMinDef(s32)
GSMinDef(s64)
GSMinDef(u8)
GSMinDef(u16)
GSMinDef(u32)
GSMinDef(u64)
GSMinDef(r32)
GSMinDef(r64)
#undef GSMinDef

#define GSMaxDef(type) static type GSMax(type A, type B) { return (A > B ? A : B); }
GSMaxDef(s8)
GSMaxDef(s16)
GSMaxDef(s32)
GSMaxDef(s64)
GSMaxDef(u8)
GSMaxDef(u16)
GSMaxDef(u32)
GSMaxDef(u64)
GSMaxDef(r32)
GSMaxDef(r64)
#undef GSMaxDef

#define GSClampDef(type) static type GSClamp(type Min, type V, type Max) { \
        type Result = V; \
        if (V < Min) { Result = Min; } \
        if (V > Max) { Result = Max; } \
        return Result; \
}
GSClampDef(s8)
GSClampDef(s16)
GSClampDef(s32)
GSClampDef(s64)
GSClampDef(u8)
GSClampDef(u16)
GSClampDef(u32)
GSClampDef(u64)
GSClampDef(r32)
GSClampDef(r64)
#undef GSClampDef

#define GSClamp01Def(type) static type GSClamp01(type V) { \
        type Min = 0; type Max = 1; \
        type Result = V; \
        if (V < Min) { Result = Min; } \
        if (V > Max) { Result = Max; } \
        return Result; \
}
GSClamp01Def(r32)
GSClamp01Def(r64)
#undef GSClamp01Def

#define GSAbsDef(type) static type GSAbs(type A) { return (A < 0 ? -A : A); }
GSAbsDef(s8)
GSAbsDef(s16)
GSAbsDef(s32)
GSAbsDef(s64)
GSAbsDef(r32)
GSAbsDef(r64)
#undef GSAbsDef

#define GSPowDef(type) static type GSPow(type N, s32 Power) { \
        type Result = N; \
        for(s32 i = 1; i < Power; i++) { Result *= N; } \
        return Result; \
}
GSPowDef(s8)
GSPowDef(s16)
GSPowDef(s32)
GSPowDef(s64)
GSPowDef(u8)
GSPowDef(u16)
GSPowDef(u32)
GSPowDef(u64)
GSPowDef(r32)
GSPowDef(r64)
#undef GSPowDef


#define GSLerpDef(type) type GSLerp(type A, type B, type Percent) { return (A * (1.0f - Percent))+(B * Percent);}
GSLerpDef(r32)
GSLerpDef(r64)
#undef GSLerpDef

static r32 GSSqrt(r32 V) { return sqrtf(V); }
static r64 GSSqrt(r64 V) { return sqrt(V); }

static r32 DegreesToRadians (r32 Degrees) { return Degrees * PI_OVER_180; }
static r64 DegreesToRadians (r64 Degrees) { return Degrees * PI_OVER_180; }

#define GS_LANGUAGE_MATH
#endif // GS_LANGUAGE_MATH

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)

internal u8* Allocate (s32 Size) 
{
    return (u8*)VirtualAlloc(NULL, Size, 
                             MEM_COMMIT | MEM_RESERVE, 
                             PAGE_EXECUTE_READWRITE);
}

internal b32 Free (u8* Base, u32 Size)
{
    b32 Result = false;
    Result = VirtualFree(Base, Size, MEM_RELEASE);
    return Result;
}

#if defined(DEBUG)
internal void
DebugPrint (char* Format, ...)
{
    char Buffer[256];
    va_list Args;
    va_start(Args, Format);
    // TODO(Peter): get rid of this
    vsnprintf(Buffer, 256, Format, Args);
    OutputDebugStringA(Buffer);
    va_end(Args);
}
#endif

internal void
PrintF (char* Format, ...)
{
    char Buffer[256];
    va_list Args;
    va_start(Args, Format);
    // TODO(Peter): get rid of this
    printf(Buffer, 256, Format, Args);
    va_end(Args);
}

#elif defined(__APPLE__) && defined(__MAC__)


internal u8* Allocate (s32 Size) 
{
    // TODO(Peter): 
    return 0; 
}

internal void Free (u8* Base, u32 Size)
{
    // TODO(Peter): 
    b32 Result = false;
    return Result;
}

#else // Std lib

internal u8* Allocate (s32 Size) 
{
    return (u8*)malloc(Size); 
}

internal void Free (u8* Base, u32 Size)
{
    b32 Result = true;
    free(Base);
    return Result;
}

#endif



#define GS_LANGUAGE_H
#endif