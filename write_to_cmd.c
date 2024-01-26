#include <Windows.h>
#include <cstdio>

int main()
{
    // Attach to the parent process's console
    AttachConsole(ATTACH_PARENT_PROCESS);

    // Write to the console
    const char* message = "Hello, world!";
    DWORD bytes_written;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), message, strlen(message), &bytes_written, nullptr);

    // Detach from the console
    FreeConsole();

    return 0;
}
