/* This program illustrates pointer aliasing.
 * Move() cannot use old value fetched from &B because Move() might
 * have been called with the same value for &D and &B.
 *
 * Move() and MoveWithoutAliasing() both work.
 * The point is about compiler optimizations.
 * Pointer aliasing means the compiler cannot optimize away the
 * second load from &B.
 * */

#include <windows.h>
//OutputDebugString()

int X = 1;
int Y = 2;
int Z = 3;
int W = 4;

void
MoveWithoutAliasing(int *A, int *B, int *C)
{
    *A = *B; // 1) LOAD FROM B, 2) WRITE TO A
    *C = *B; // 3) WRITE TO C SAME VALUE ALREADY FETCHED FROM B
}

void
Move(int *A, int *B, int *C, int *D)
{
    *A = *B; // 1) LOAD FROM B, 2) WRITE TO A
    *D = 5;  // 3)WRITE D
    *C = *B; // 4) MUST LOAD FROM B, 5) THEN WRITE TO C
}

int CALLBACK
WinMain( // Program entry point
    HINSTANCE Instance,     // Handle to application instance
    HINSTANCE PrevInstance, // not important
    LPSTR CommandLine,      // not important
    int ShowCode            // not important
    )
{
    Move(&X, &Y, &Z, &Y);
    return(0);
}
