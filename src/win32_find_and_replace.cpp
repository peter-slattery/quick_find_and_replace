#include <stdio.h>
#include "gs_language.h"
#include "gs_string.h"

internal char*
ReadEntireFileAndNullTerminate (char* Filename)
{
    char* Result = 0;
    
    printf("Opening File: %s\n", Filename);
    
    FILE* ReadFile = fopen(Filename, "r");
    if (ReadFile)
    {
        fseek(ReadFile, 0, SEEK_END);
        size_t FileSize = ftell(ReadFile);
        fseek(ReadFile, 0, SEEK_SET);
        
        Result = (char*)malloc(FileSize + 1);
        size_t ReadSize = fread(Result, 1, FileSize, ReadFile);
        Result[FileSize] = 0;
        
        fclose(ReadFile);
    }
    else
    {
        InvalidCodePath;
    }
    
    return Result;
}

internal void
WriteEntireFile (char* Filename, char* Contents, s32 Length)
{
    FILE* WriteFile = fopen(Filename, "w");
    if (WriteFile)
    {
        s32 WriteResult = fputs(Contents, WriteFile);
        if (WriteResult > 0)
        {
            fclose(WriteFile);
        }
    }
}

struct replace_pair
{
    char Find[256];
    s32 FindLength;
    char Replace[256];
    s32 ReplaceLength;
};

struct insert_result
{
    char* File;
    s32 Size;
    char* Cursor;
};

internal insert_result
InsertTokenInFile (char* CurrFileContents, s32 CurrFileSize, char* Cursor, replace_pair* Pair)
{
    printf("Replacing %s with %s", Pair->Find, Pair->Replace);
    
    char* File = CurrFileContents;
    s32 FileSize = CurrFileSize;
    s32 CursorPosition = Cursor - CurrFileContents;
    
    s32 DestFileSize = FileSize;
    char* DestFile = File;
    s32 SizeDifference = Pair->ReplaceLength - Pair->FindLength;
    
    if (SizeDifference > 0)
    {
        
        s32 NewFileSize = FileSize + SizeDifference + 1;
        char* NewFile = (char*)malloc(NewFileSize);
        
        CopyCharArray(File, NewFile, Cursor - CurrFileContents);
        DestFileSize = NewFileSize;
        DestFile = NewFile;
    }
    else
    {
        DestFileSize += SizeDifference;
    }
    
    CopyCharArray(Pair->Replace, DestFile + CursorPosition, Pair->ReplaceLength);
    CopyCharArray(File + CursorPosition + Pair->FindLength, 
                  DestFile + CursorPosition + Pair->ReplaceLength,
                  FileSize - (CursorPosition + Pair->FindLength));
    *(DestFile + DestFileSize) = 0;
    
    insert_result Result = {};
    Result.File = DestFile;
    Result.Size = DestFileSize;
    Result.Cursor = DestFile + CursorPosition + Pair->ReplaceLength;
    
    return Result;
}

int main(int ArgCount, char** Args)
{
    if (ArgCount <= 1) { return 0; }
    
    char* FileName = Args[1];
    char* FileContents = ReadEntireFileAndNullTerminate(FileName);
    s32   FileSize = CharArrayLength(FileContents);
    Assert(FileContents != 0);
    
    if (ArgCount > 2)
    {
        s32 PairsMax = ArgCount - 2;
        s32 PairsCount = 0;
        replace_pair* Pairs = (replace_pair*)malloc(sizeof(replace_pair) * PairsMax);
        
        for (int i = 2; i < ArgCount; i++)
        {
            char* Arg = Args[i];
            
            char* FindValue = Arg;
            s32 FindValueLength = FirstIndexOfChar(FindValue, '=');
            
            char* ReplaceValue = Arg + FindValueLength + 1;
            s32 ReplaceValueLength = CharArrayLength(Arg) - (FindValueLength + 1);
            
            replace_pair* CurrentPair = Pairs + PairsCount;
            GSZeroMemory(&CurrentPair->Find[0], 256);
            GSZeroMemory(&CurrentPair->Replace[0], 256);
            
            CopyCharArray(FindValue, CurrentPair->Find, FindValueLength);
            CurrentPair->FindLength = FindValueLength;
            CopyCharArray(ReplaceValue, CurrentPair->Replace, ReplaceValueLength);
            CurrentPair->ReplaceLength = ReplaceValueLength;
            
            PairsCount++;
        }
        
        char* FileIter = FileContents;
        while (*FileIter)
        {
            for (s32 i = 0; i < PairsCount; i++)
            {
                replace_pair* CurrentPair = Pairs + i;
                if (CharArraysEqual(FileIter, CurrentPair->FindLength,
                                    CurrentPair->Find, CurrentPair->FindLength))
                {
                    insert_result PostInsertFile = InsertTokenInFile (FileContents, FileSize, 
                                                                      FileIter, CurrentPair);
                    FileContents = PostInsertFile.File;
                    FileSize     = PostInsertFile.Size;
                    FileIter     = PostInsertFile.Cursor;
                }
            }
            
            FileIter++;
        }
        
        WriteEntireFile(FileName, FileContents, FileSize);
    }
    
    return 0;
}