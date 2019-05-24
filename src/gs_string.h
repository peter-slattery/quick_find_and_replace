////////////////////////////////////////////////////////////////
//        String 
////////////////////////////////////////////////////////////////

struct string
{
    char* Data;
    s32 Length;
    s32 Max;
};


////////////////////////////////////////////////////////////////
//        String Memory
////////////////////////////////////////////////////////////////

struct slot_header
{
    slot_header* Next;
    s32 Size;
};

struct slot_arena
{
    u8* Memory;
    s32 SlotSize;
    s32 SlotCount;
    slot_header* FreeList;
};

struct contiguous_slot_count_result
{
    s32 Count;
    slot_header* LastContiguousSlot;
};


////////////////////////////////////////////////////////////////
//        String Function Declarations
////////////////////////////////////////////////////////////////

// Utility
#if !defined GS_LANGUAGE_H

static void    GSZeroMemory (u8* Memory, s32 Size);
static s32     GSMin (s32 A, s32 B); 
static s32     GSAbs (s32 A); 
static float   GSAbsF (float A);
static float   GSPowF (float N, s32 Power);

#endif

// Setup
static void   InitializeString (string* String, char* Data, s32 DataSize);
static string InitializeString (char* Data, s32 DataSize);
static void   ClearString (string* String);

// Char/Char Array
static bool     IsSlash (char C);
static bool     IsNewline (char C);
static bool     IsWhitespace (char C);
static bool     IsAlpha (char C);
static bool     IsUpper (char C);
static bool     IsLower (char C);
static bool     IsNumeric (char C);
static bool     ToUpper (char C);
static bool     ToLower (char C);
static bool     IsAlphaNumeric (char C);
static u32      CharToUInt (char C);
static s32      CharArrayLength (char* CharArray);
static bool     CharArraysEqual (char* A, s32 ALength, char* B, s32 BLength);
static void     ReverseCharArray (char* Array, s32 Length);
#define         FirstIndexOfChar(array, find) IndexOfChar(array, 0, find)
static s32      IndexOfChar (char* Array, s32 Start, char Find);
#define         FastLastIndexOfChar(array, len, find) FastReverseIndexOfChar(array, len, 0, find)
static s32      FastReverseIndexOfChar (char* Array, s32 Length, s32 OffsetFromEnd, char Find);
#define         LastIndexOfChar(array, find) ReverseIndexOfChar(array, 0, find)
static s32      ReverseIndexOfChar (char* Array, s32 OffsetFromEnd, char Find);

// String
static bool    StringsEqual (string A, string B);
static bool    StringEqualsCharArray (string String, char* CharArray);
static s32     FindFirstChar (string String, char C);
static void    SetStringToChar (string* Dest, char C, s32 Count);
static void    SetStringToCharArray (string* Dest, char* Source);
static void    ConcatString (string* Dest, string Source);
static void    ConcatString (string* Dest, string Source, s32 Length);
static void    CopyStringTo (string Source, string* Dest);
static void    CopyCharArray (char* Source, char* Dest, s32 DestLength);
static void    InsertChar (string* String, char Char, s32 Index);
static void    RemoveCharAt (string* String, s32 Index);
static string  Substring (string* String, s32 Start, s32 End);

// Parsing
static u32   ParseUnsignedInt (char* String, s32 Length);
static s32   UIntToString (u32 Int, char* String, s32 Length);
static s32   ParseSignedInt (char* String, s32 Length);
static s32   IntToString (s32 Int, char* String, s32 Length);
static s32   IntToString (s32 Int, char* String, s32 Length, s32 Offset);
static float ParseFloat (char* String, s32 Length);
static s32   FloatToString(float Float, char *String, s32 Length, s32 AfterPoint);


////////////////////////////////////////////////////////////////
//        String Memory Function Declarations
////////////////////////////////////////////////////////////////

static s32                          CalculateSlotCountFromSize (s32 RequestedSize, s32 SlotSize);
static bool                         SlotsAreContiguous (slot_header* First, slot_header* Second);
static contiguous_slot_count_result CountContiguousSlots (slot_header* First);
static slot_header*                 GetSlotAtOffset(slot_header* First, s32 Offset);
static slot_header*                 InsertSlotIntoList (slot_header* NewSlot, slot_header* ListStart);
static void                         AllocStringFromStringArena (string* String, s32 Size, slot_arena* Storage);
static string                       AllocStringFromStringArena (s32 Size, slot_arena* Storage);
static void                         FreeToStringArena (string* String, slot_arena* Storage);
static void                         ReallocFromStringArena (string* String, s32 NewSize, slot_arena* Storage);

////////////////////////////////////////////////////////////////
//        String Utility Functions
////////////////////////////////////////////////////////////////

#if !defined GS_LANGUAGE_H

static void
GSZeroMemory (u8* Memory, s32 Size)
{
    for (int i = 0; i < Size; i++) { Memory[i] = 0; }
}

static s32
GSMin (s32 A, s32 B)
{
    return (A < B ? A : B);
}

static s32
GSAbs (s32 A)
{
    return (A < 0 ? -A : A);
}

static float
GSAbs (float A)
{
    return (A < 0 ? -A : A);
}

static float
GSPow (float N, s32 Power)
{
    float Result = N;
    for(s32 i = 1; i < Power; i++) { Result *= N; }
    return Result;
}

#endif

////////////////////////////////////////////////////////////////
//        Init and Clear
////////////////////////////////////////////////////////////////

static void
InitializeString (string* String, char* Data, s32 DataSize)
{
    String->Data = Data;
    String->Max = DataSize;
    String->Length = 0;
}

static string
InitializeString (char* Data, s32 DataSize)
{
    string Result = {};
    Result.Data = Data;
    Result.Max = DataSize;
    Result.Length = 0;
    return Result;
}

static void
ClearString (string* String)
{
    String->Data = 0;
    String->Max = 0;
    String->Length = 0;
}

////////////////////////////////////////////////////////////////
//        Basic Char Operations
////////////////////////////////////////////////////////////////

static bool IsSlash (char C) { return ((C == '\\') || (C == '/')); }
static bool IsNewline (char C) { return (C == '\n') || (C == '\r'); }
static bool IsWhitespace (char C) { return (C == ' ') || (C == '\t') || IsNewline(C); }
static bool IsAlpha (char C)
{
    // TODO(Peter): support UTF8 chars
    return ((C >= 'A') && (C <= 'Z')) || ((C >= 'a') && (C <= 'z'));
}
static bool IsUpper (char C)
{
    return ((C >= 'A') && (C <= 'Z'));
}
static bool IsLower (char C)
{
    return ((C >= 'a') && (C <= 'z'));
}
static bool IsNumeric (char C)
{
    return (C >= '0') && (C <= '9');
}
static bool IsAlphaNumeric (char C)
{
    return IsAlpha(C) || IsNumeric(C);
}

static u32 CharToUInt (char C) { 
    u32 Result = (C - '0');
    return Result; 
}

static s32
CharArrayLength (char* Array)
{
    char* C = Array;
    s32 Result = 0;
    while (*C)
    {
        *C++;
        Result++;
    }
    return Result;
}

static s32      
NullTerminatedCharArrayLength (char* CharArray)
{
    char* Iter = CharArray;
    while (*Iter)
    {
        *Iter++;
    }
    return (Iter - CharArray);
}

static bool
CharArraysEqual (char* A, s32 ALength, char* B, s32 BLength)
{
    bool Result = false;
    if (ALength == BLength)
    {
        Result = true;
        char* AIter = A;
        char* BIter = B;
        for (s32 i = 0; i < ALength; i++)
        {
            if(*AIter++ != *BIter++)
            {
                Result = false;
                break;
            }
        }
    }
    return Result;
}

static void
ReverseCharArray (char* Array, s32 Length)
{
    char* ForwardIter = Array;
    char* BackwardIter = Array + Length - 1;
    for (s32 i = 0; i < (Length / 2); i++)
    {
        char F = *ForwardIter;
        char B = *BackwardIter;
        *ForwardIter++ = B;
        *BackwardIter-- = F;
    }
}

static s32 
IndexOfChar (char* Array, s32 After, char Find)
{
    s32 Result = -1;
    
    s32 Counter = After;
    char* Iter = Array + After;
    while (*Iter)
    {
        if (*Iter == Find)
        {
            Result = Counter;
            break;
        }
        Counter++;
        *Iter++;
    }
    
    return Result;
}

static s32
FastReverseIndexOfChar (char* Array, s32 Length, s32 OffsetFromEnd, char Find)
{
    s32 Result = -1;
    
    s32 Counter = Length - OffsetFromEnd;
    char* Iter = Array + Length - OffsetFromEnd;
    for (int i = 0; i < (Length - OffsetFromEnd); i++)
    {
        if (*Iter == Find)
        {
            Result = Counter;
            break;
        }
        
        *Iter--;
        Counter--;
    }
    
    return Result;
}

static s32
ReverseIndexOfChar (char* Array, s32 OffsetFromEnd, char Find)
{
    s32 StringLength = NullTerminatedCharArrayLength(Array);
    return FastReverseIndexOfChar(Array, StringLength, OffsetFromEnd, Find);
}

////////////////////////////////////////////////////////////////
//        Basic String Operations
////////////////////////////////////////////////////////////////

static bool
StringsEqual (string A, string B)
{
    bool Result = false;
    
    if (A.Length == B.Length)
    {
        Result = true;
        char* AIter = A.Data;
        char* BIter = B.Data;
        for (s32 i = 0; i < A.Length; i++)
        {
            if (*AIter++ != *BIter++)
            {
                Result = false;
                break;
            }
        }
    }
    
    return Result;
}

#define MakeStringLiteral(array) MakeString((array), sizeof(array))
static string
MakeString (char* Array, s32 Length)
{
    string Result = {};
    Result.Data = Array;
    Result.Length = Length;
    Result.Max = Length;
    return Result;
}

static string
MakeString (char* Array)
{
    s32 Length = CharArrayLength (Array);
    return MakeString(Array, Length);
}

static bool
StringEqualsCharArray (string String, char* CharArray)
{
    bool Result = true;
    
    char* S = String.Data;
    char* C = CharArray;
    
    s32 Count = 0;
    while (*C && Count < String.Length)
    {
        if (*C++ != *S++)
        {
            Result = false;
            break;
        }
        Count++;
    }
    
    return Result;
}

static s32
FindFirstChar (string String, char C)
{
    s32 Result = -1;
    
    char* Iter = String.Data;
    for (int i = 0; i < String.Length; i++)
    {
        if (*Iter++ == C)
        {
            Result = i;
            break;
        }
    }
    
    return Result;
}

static void
SetStringToChar (string* Dest, char C, s32 Count)
{
    Assert(Count <= Dest->Max);
    
    char* Iter = Dest->Data;
    for (int i = 0; i < Count; i++)
    {
        *Iter++ = C;
    }
    Dest->Length = Count;
}

static void
SetStringToCharArray (string* Dest, char* Source)
{
    Dest->Length = 0;
    
    char* Src = Source;
    char* Dst = Dest->Data;
    while (*Src && Dest->Length < Dest->Max)
    {
        *Dst++ = *Src++;
        Dest->Length++;
    }
}

static void
ConcatString (string* Dest, string Source)
{
    Assert((Dest->Length + Source.Length) <= Dest->Max);
    
    char* Dst = Dest->Data + Dest->Length;
    char* Src = Source.Data;
    for (s32 i = 0; i < Source.Length; i++)
    {
        *Dst++ = *Src++;
        Dest->Length++;
    }
}

static void
ConcatString (string* Dest, string Source, s32 Length)
{
    Assert(Length < Source.Length);
    Assert((Dest->Length + Length) <= Dest->Max);
    
    char* Dst = Dest->Data + Dest->Length;
    char* Src = Source.Data;
    for (s32 i = 0; i < Length; i++)
    {
        *Dst++ = *Src++;
        Dest->Length++;
    }
}

static void
CopyStringTo (string Source, string* Dest)
{
    char* Src = Source.Data;
    char* Dst = Dest->Data;
    s32 CopyLength = GSMin(Source.Length, Dest->Max);
    for (int i = 0; i < CopyLength; i++)
    {
        *Dst++ = *Src++;
    }
    Dest->Length = Source.Length;
}

static void
CopyCharArray (char* Source, char* Dest, s32 DestLength)
{
    char* Src = Source;
    char* Dst = Dest;
    s32 i = 0; 
    while (*Src && i < DestLength)
    {
        *Dst++ = *Src++;
        i++;
    }
}

static void
InsertChar (string* String, char Char, s32 Index)
{
    Assert(Index >= 0 && Index < String->Max);
    Assert(String->Length < String->Max);
    
    char* Src = String->Data + String->Length;
    char* Dst = Src + 1;
    for (int i = String->Length - 1; i >= Index; i--)
    {
        *Dst-- = *Src--;
    }
    
    *(String->Data + Index) = Char;
    String->Length++;
}

static void
RemoveCharAt (string* String, s32 Index)
{
    Assert(Index >= 0 && Index < String->Max);
    
    char* Dst = String->Data + Index;
    char* Src = Dst + 1;
    for (int i = Index; i < String->Length; i++)
    {
        *Dst++ = *Src++;
    }
    *Dst = 0;
    String->Length--;
}

static string
Substring (string* String, s32 Start, s32 End)
{
    Assert(Start >= 0 && End > Start && End < String->Length);
    
    string Result = {};
    Result.Data = String->Data + Start;
    Result.Length = End - Start;
    return Result;
}

////////////////////////////////////////////////////////////////
//        String Parsing
////////////////////////////////////////////////////////////////

static u32
ParseUnsignedInt (char* String, s32 Length)
{
    Assert(IsNumeric(*String));
    
    char* Iter = String;
    u32 Result = 0;
    for (s32 i = 0; i < Length; i++)
    {
        Result = CharToUInt(*Iter++) + (Result * 10);
    }
    return Result;
}

static s32
UIntToString (u32 Int, char* String, s32 Length)
{
    s32 Remaining = Int;
    s32 CharsCopied = 0;
    char* Iter = String;
    while (Remaining > 0 && CharsCopied < Length)
    {
        *Iter++ = '0' + (Remaining % 10);
        Remaining /= 10;
        CharsCopied++;
    }
    ReverseCharArray(String, CharsCopied);
    return CharsCopied;
}

static s32
ParseSignedInt (char* String, s32 Length)
{
    Assert(Length > 0);
    
    s32 Negative = 1;
    s32 LengthRemaining = Length;
    s32 Result = 0;
    char* Iter = String;
    
    if (*Iter == '-') { 
        LengthRemaining--;
        *Iter++; 
        Negative = -1; 
    }
    
    for (s32 i = 0; i < LengthRemaining; i++)
    {
        Result = CharToUInt(*Iter++) + (Result * 10);
    }
    
    Result *= Negative;
    
    return Result;
}

static s32
IntToString (s32 Int, char* String, s32 Length)
{
    s32 Remaining = Int;
    s32 CharsCopied = 0;
    char* Iter = String;
    
    bool Negative = Remaining < 0;
    Remaining = GSAbs(Remaining);
    
    while (Remaining > 0 && CharsCopied < Length)
    {
        *Iter++ = '0' + (Remaining % 10);
        Remaining /= 10;
        CharsCopied++;
    }
    
    if (Negative)
    {
        *Iter++ = '-';
        CharsCopied++;
    }
    
    ReverseCharArray(String, CharsCopied);
    return CharsCopied;
}

static s32 
IntToString (s32 Int, char* String, s32 Length, s32 Offset)
{
    char* StringStart = String + Offset;
    s32 LengthWritten = IntToString(Int, StringStart, Length - Offset);
    return LengthWritten;
}

static float
ParseFloat (char* String, s32 Length)
{
    s32 Negative = 1;
    s32 LengthRemaining = Length;
    float Result = 0;
    char* Iter = String;
    
    if (*Iter == '-') { 
        LengthRemaining--;
        *Iter++; 
        Negative = -1; 
    }
    
    for (s32 i = 0; i < LengthRemaining; i++)
    {
        if (IsNumeric(*Iter))
        {
            Result = (float)CharToUInt(*Iter++) + (Result * 10);
        }
        else if (*Iter == '.' || *Iter == 0) 
        { 
            LengthRemaining -= i;
            break; 
        }
    }
    
    if (*Iter == '.')
    {
        *Iter++;
        float AfterPoint = 0;
        s32 PlacesAfterPoint = 0;
        
        for (s32 i = 0; i < LengthRemaining; i++)
        {
            if (IsNumeric(*Iter))
            {
                AfterPoint = (float)CharToUInt(*Iter++) + (AfterPoint * 10);
                PlacesAfterPoint++;
            }
            else
            {
                break;
            }
        }
        
        AfterPoint = AfterPoint / GSPow(10, PlacesAfterPoint);
        Result += AfterPoint;
    }
    
    Result *= Negative;
    
    return Result;
}

static s32
FloatToString(float Float, char *String, s32 Length, s32 AfterPoint)
{
    s32 IPart = (s32)Float;
    float FPart = GSAbs(Float - (float)IPart);
    
    s32 i = IntToString(IPart, String, Length);
    
    if (AfterPoint > 1)
    {
        String[i++] = '.';
        
        s32 FPartInt = FPart * GSPow(10, AfterPoint);
        i += IntToString(FPartInt, String, Length, i);
    }
    
    return i;
}

////////////////////////////////////////////////////////////////
//        PrintF
////////////////////////////////////////////////////////////////


static void
PrintF (string* Dest, char* Format, s32 FormatLength,  ...)
{
    
}


////////////////////////////////////////////////////////////////
//        String Memory Function Definitions
////////////////////////////////////////////////////////////////

static s32
CalculateSlotCountFromSize (s32 RequestedSize, s32 SlotSize)
{
    s32 SlotCount = RequestedSize / SlotSize;
    if (SlotCount * SlotSize < RequestedSize)
    {
        SlotCount += 1;
    }
    return SlotCount;
}

static bool
SlotsAreContiguous (slot_header* First, slot_header* Second)
{
    bool Result = false;
    u8* FirstSlotNextAddress = (u8*)First + First->Size;
    u8* SecondAddress = (u8*)Second;
    Result = FirstSlotNextAddress == SecondAddress;
    return Result;
}

static contiguous_slot_count_result
CountContiguousSlots (slot_header* First)
{
    Assert(First != 0);
    
    contiguous_slot_count_result Result = {};
    Result.Count = 1;
    
    slot_header* IterPrev = First;
    slot_header* Iter = First->Next;
    while (Iter && SlotsAreContiguous(IterPrev, Iter))
    {
        Result.Count++;
        IterPrev = Iter;
        Iter = Iter->Next;
    }
    
    Result.LastContiguousSlot = IterPrev;
    return Result;
}

static slot_header*
GetSlotAtOffset(slot_header* First, s32 Offset)
{
    slot_header* Iter = First;
    s32 Count = 0;
    while (Count < Offset && Iter)
    {
        Iter = Iter->Next;
        Count++;
    }
    return Iter;
}

static slot_header*
InsertSlotIntoList (slot_header* NewSlot, slot_header* ListStart)
{
    slot_header* List = ListStart;
    if (NewSlot < List)
    {
        NewSlot->Next = List;
        List = NewSlot;
    }
    else
    {
        slot_header* PrevIter = List;
        slot_header* Iter = List->Next;
        while (Iter && NewSlot > Iter)
        {
            PrevIter = Iter;
            Iter = Iter->Next;
        }
        
        Assert(PrevIter);
        if (PrevIter) 
        { 
            PrevIter->Next = NewSlot;
        }
        
        if (Iter)
        {
            NewSlot->Next = Iter;
        }
    }
    return List;
}

static void
AllocStringFromStringArena (string* String, s32 Size, slot_arena* Storage)
{
    s32 SlotCount = CalculateSlotCountFromSize(Size, Storage->SlotSize);
    slot_header* Slot = Storage->FreeList;
    slot_header* PrevSlot = 0;
    while (Slot)
    {
        contiguous_slot_count_result ContiguousSlots = CountContiguousSlots(Slot);
        if (ContiguousSlots.Count >= SlotCount)
        {
            slot_header* NextStartSlot = GetSlotAtOffset(Slot, SlotCount);
            if (PrevSlot)
            {
                PrevSlot->Next = NextStartSlot;
            }
            else
            {
                Storage->FreeList = NextStartSlot;
            }
            break;
        }
        else
        {
            PrevSlot = Slot;
            Slot = Slot->Next; 
        }
    }
    
    if (Slot)
    {
        String->Data = (char*)Slot;
        GSZeroMemory((u8*)String->Data, SlotCount * Storage->SlotSize);
        String->Max = SlotCount * Storage->SlotSize;
        String->Length = 0;
    }
}

static string
AllocStringFromStringArena (s32 Size, slot_arena* Storage)
{
    string Result = {0};
    AllocStringFromStringArena(&Result, Size, Storage);
    return Result;
}

static void
FreeToStringArena (string* String, slot_arena* Storage)
{
    u8* Base = (u8*)(String->Data);
    u8* End = Base + String->Max - 1;
    u8* MemoryEnd = Storage->Memory + (Storage->SlotSize * Storage->SlotCount);
    Assert((Base >= Storage->Memory) && (End < MemoryEnd));
    Assert((String->Max % Storage->SlotSize) == 0);
    
    s32 SizeReclaimed = 0;
    slot_header* Slot = (slot_header*)Base;
    while (SizeReclaimed < String->Max)
    {
        Slot->Size = Storage->SlotSize;
        Storage->FreeList = InsertSlotIntoList(Slot, Storage->FreeList);
        SizeReclaimed += Storage->SlotSize;
        Slot = (slot_header*)(Base + SizeReclaimed);
    }
    
    String->Data = 0;
    String->Length = 0;
    String->Max = 0;
}

static void
ReallocFromStringArena (string* String, s32 NewSize, slot_arena* Storage)
{
    string NewString = AllocStringFromStringArena(NewSize, Storage);
    CopyStringTo(*String, &NewString);
    FreeToStringArena(String, Storage);
    *String = NewString;
}

#if defined(DEBUG)

void DEBUGPrintChars (string* String, s32 Count)
{
    char* Iter = String->Data;
    for (int i = 0; i < Count; i++)
    {
        *Iter++ = (char)('A' + i);
    }
    String->Length = Count;
}

static void
TestStrings()
{
    
    slot_arena StringArena = {};
    
    s32 TestCount = 0;
    s32 SuccessCount = 0;
    
    DebugPrint("\n\n-------------------------------------------------\n  Begin Testing Strings\n\n\n");
    
    ////////////////////////////////////////////////////////////////
    //    Char Functions
    
    char ForwardArray[] = "Hello, Sailor";
    char BackwardArray[] = "roliaS ,olleH";
    TestClean(CharArraysEqual(ForwardArray, 13, ForwardArray, 13), "String Equality");
    TestClean(!CharArraysEqual(ForwardArray, 13, BackwardArray, 13), "String Equality");
    
    TestClean(IndexOfChar(ForwardArray, 0, ',') == 5, "Index Of Char");
    TestClean(IndexOfChar(ForwardArray, 5, 'l') == 10, "Index of Char (skipping first 5)");
    TestClean(FastReverseIndexOfChar(ForwardArray, 13, 0, 'o') == 11, "Fast Reverse Index Of Char");
    TestClean(ReverseIndexOfChar(ForwardArray, 0, 'o') == 11, "Reverse Index of Char");
    TestClean(ReverseIndexOfChar(ForwardArray, 3, 'o') == 4, "Reverse Index of Char (skipping last 3)");
    TestClean(LastIndexOfChar(ForwardArray, 'o') == 11, "Last Index of Char");
    
    ReverseCharArray(ForwardArray, 13);
    TestClean(CharArraysEqual(ForwardArray, 13, BackwardArray, 13), "Reversing Char Array");
    
    char UIntString[] = "1234";
    u32 UIntValue = 1234;
    u32 ParsedUInt = ParseUnsignedInt(UIntString, 4);
    TestClean((ParsedUInt == UIntValue), "String To U32");
    char StringifiedUInt[4] = {};
    UIntToString(UIntValue, StringifiedUInt, 4);
    TestClean(CharArraysEqual(UIntString, 4, StringifiedUInt, 4), "U32 To String");
    
    char IntString[] = "-1234";
    s32 IntValue = -1234;
    s32 ParsedInt = ParseSignedInt(IntString, 5);
    TestClean((ParsedInt == IntValue), "String To S32");
    char StringifiedInt[5] = {};
    IntToString(IntValue, StringifiedInt, 5);
    TestClean(CharArraysEqual(IntString, 5, StringifiedInt, 5), "S32 to String");
    
    char FloatString[] = "-1234.125";
    float FloatValue = -1234.125f;
    float ParsedFloat = ParseFloat(FloatString, 8);
    TestClean((ParsedFloat == FloatValue), "String To Float");
    char StringifiedFloat[10] = {};
    FloatToString(FloatValue, StringifiedFloat, 10, 3);
    TestClean(CharArraysEqual(FloatString, 8, StringifiedFloat, 8), "Float To String");
    
    
    ////////////////////////////////////////////////////////////////
    //
    
    StringArena.SlotSize = 256;
    StringArena.SlotCount = 32;
    StringArena.Memory = Allocate(StringArena.SlotSize * StringArena.SlotCount);
    slot_header* PrevSlotHeader = 0;
    for (int i = StringArena.SlotCount - 1; i >= 0; i--)
    {
        u8* SlotBase = StringArena.Memory + (i * StringArena.SlotSize);
        slot_header* SlotHeader = (slot_header*)SlotBase;
        SlotHeader->Size = StringArena.SlotSize;
        SlotHeader->Next = PrevSlotHeader;
        
        // TEST(peter): Should be true always, except on the first iteration, when there is no next slot
        bool Contiguity = SlotsAreContiguous(SlotHeader, PrevSlotHeader);
        TestClean((Contiguity || SlotHeader->Next == 0), "Contiguous Arenas");
        
        PrevSlotHeader = SlotHeader;
    }
    StringArena.FreeList = PrevSlotHeader;
    
    // TEST(peter): Count Should equal StringArena.SlotCount
    s32 ContiguousSlotsCountBefore = CountContiguousSlots(StringArena.FreeList).Count;
    TestClean((ContiguousSlotsCountBefore == StringArena.SlotCount), "Contiguous Arenas");
    
    // TEST(peter): Should be false
    bool Contiguity = SlotsAreContiguous(StringArena.FreeList, StringArena.FreeList->Next->Next);
    Contiguity = SlotsAreContiguous(StringArena.FreeList->Next->Next, StringArena.FreeList);
    TestClean(!Contiguity, "Non Contiguous Arenas");
    
    s32 Slots = CalculateSlotCountFromSize(10, 256);
    TestClean(Slots == 1, "Slot Sizing");
    Slots = CalculateSlotCountFromSize(256, 256);
    TestClean(Slots == 1, "Slot Sizing");
    Slots = CalculateSlotCountFromSize(345, 256);
    TestClean(Slots == 2, "Slot Sizing");
    Slots = CalculateSlotCountFromSize(1024, 256);
    TestClean(Slots == 4, "Slot Sizing");
    
    slot_header* HeaderTen = GetSlotAtOffset(StringArena.FreeList, 10);
    slot_header* HeaderThree = GetSlotAtOffset(StringArena.FreeList, 3);
    slot_header* HeaderFive = GetSlotAtOffset(StringArena.FreeList, 5);
    
    string StringA = AllocStringFromStringArena(10, &StringArena);
    string StringB = AllocStringFromStringArena(345, &StringArena);
    
#if 0
    // TEST(peter): Should TestClean
    u8* RandomMemory = (u8*)malloc(256);
    string RandomMemString = {};
    RandomMemString.Data = (char*)RandomMemory;
    RandomMemString.Max = 256;
    FreeToStringArena(&RandomMemString, &StringArena); 
#endif
    FreeToStringArena(&StringA, &StringArena);
    FreeToStringArena(&StringB, &StringArena);
    // TEST(peter): After freeing both allocations, ContiguousSlotCountBefore and ContiguousSlotCountAfter should be equal
    s32 ContiguousSlotCountAfter = CountContiguousSlots(StringArena.FreeList).Count;
    TestClean(ContiguousSlotCountAfter == ContiguousSlotsCountBefore, "Add and REmove Slots from Arena");
    
    // TEST(peter): Set up a free list where the first element is too small, so it has to traverse to find an appropriately
    // sized block
    // The slots will look list [256][used][256][256][256] etc..
    StringA = AllocStringFromStringArena(256, &StringArena);
    StringB = AllocStringFromStringArena(256, &StringArena);
    FreeToStringArena(&StringA, &StringArena);
    u32 Contiguous = CountContiguousSlots(StringArena.FreeList).Count; // Should = 1;
    string StringC = AllocStringFromStringArena(512, &StringArena);
    slot_header* HeaderC = (slot_header*)(StringC.Data);
    
    string ReallocTestString = AllocStringFromStringArena(256, &StringArena);
    DEBUGPrintChars(&ReallocTestString, 24);
    ReallocFromStringArena(&ReallocTestString, 512, &StringArena);
    
    
    string TestString = AllocStringFromStringArena(10, &StringArena);
    DEBUGPrintChars(&TestString, TestString.Max);
    ReallocFromStringArena(&TestString, 20, &StringArena);
    DEBUGPrintChars(&TestString, TestString.Max);
    ReallocFromStringArena(&TestString, 10, &StringArena);
    FreeToStringArena(&TestString, &StringArena);
    
    string EqualityStringA = AllocStringFromStringArena(345, &StringArena);
    string EqualityStringB = AllocStringFromStringArena(415, &StringArena);
    // Equality should succeed despite length differences
    string EqualityStringC = AllocStringFromStringArena(256, &StringArena); 
    string EqualityStringD = AllocStringFromStringArena(256, &StringArena); // Equality should fail
    string EqualityStringEmpty = {};
    
    DEBUGPrintChars(&EqualityStringA, 24);
    DEBUGPrintChars(&EqualityStringB, 24);
    DEBUGPrintChars(&EqualityStringC, 24);
    DEBUGPrintChars(&EqualityStringD, 12);
    
    bool ABEquality = StringsEqual(EqualityStringA, EqualityStringB); // Should Succeed
    bool ACEquality = StringsEqual(EqualityStringA, EqualityStringC); // Should Succeed
    bool ADEquality = StringsEqual(EqualityStringA, EqualityStringD); // Should Fail
    bool AEEquality = StringsEqual(EqualityStringA, EqualityStringEmpty); // Should Fail
    
    TestClean(ABEquality, "String Equality");
    TestClean(ACEquality, "String Equality");
    TestClean(!ADEquality, "String Equality");
    TestClean(!AEEquality, "String Equality");
    
    string CatStringA = AllocStringFromStringArena(256, &StringArena);
    SetStringToCharArray(&CatStringA, "Hello ");
    string CatStringB = AllocStringFromStringArena(512, &StringArena);
    SetStringToCharArray(&CatStringB, "Sailor!");
    string CatStringResult = AllocStringFromStringArena(512, &StringArena);
    SetStringToCharArray(&CatStringResult, "Hello Sailor!");
    ConcatString(&CatStringA, CatStringB);
    TestClean(StringsEqual(CatStringA, CatStringResult), "Cat Strings");
    
    s32 FirstSpaceIndex = FindFirstChar(CatStringA, ' ');
    TestClean(FirstSpaceIndex == 5, "First Index");
    
    SetStringToChar(&CatStringB, 'B', 5);
    TestClean(StringEqualsCharArray(CatStringB, "BBBBB"), "SetStringToChar");
    
    
    DebugPrint("Results: Passed %d / %d\n\n\n", SuccessCount, TestCount);
}
#endif // DEBUG
