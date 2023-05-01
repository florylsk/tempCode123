#include <Windows.h>

int main()
{
    HANDLE hInputFile = /* your original file handle */;
    HANDLE hOutputFile = /* your new file handle */;

    // Get the size of the input file
    LARGE_INTEGER fileSize;
    GetFileSizeEx(hInputFile, &fileSize);

    // Set the file pointer to the beginning of the input file
    LARGE_INTEGER filePos;
    filePos.QuadPart = 0;
    SetFilePointerEx(hInputFile, filePos, NULL, FILE_BEGIN);

    // Define a buffer to read data from the input file handle
    const DWORD bufferSize = 1024;
    BYTE buffer[bufferSize];

    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;
    BOOL success = FALSE;

    // Read data from the input file handle and write it to the output file handle
    while (success = ReadFile(hInputFile, buffer, bufferSize, &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            // End of file
            break;
        }

        success = WriteFile(hOutputFile, buffer, bytesRead, &bytesWritten, NULL);
        if (!success || bytesRead != bytesWritten)
        {
            // Error writing data
            break;
        }
    }

    // Clean up handles
    CloseHandle(hInputFile);
    CloseHandle(hOutputFile);

    return success ? 0 : 1;
}
