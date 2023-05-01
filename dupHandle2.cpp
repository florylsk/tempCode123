HANDLE hOriginalFile = CreateFileW(L"originalFile.dmp", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
if (hOriginalFile == INVALID_HANDLE_VALUE) {
    // handle error
}

// write data to original file
MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hOriginalFile, MiniDumpNormal, NULL, NULL, NULL);

HANDLE hNewFile = CreateFileW(L"newFile.dmp", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
if (hNewFile == INVALID_HANDLE_VALUE) {
    // handle error
}

// set file pointer to beginning of original file
SetFilePointer(hOriginalFile, 0, NULL, FILE_BEGIN);

BYTE buffer[1024];
DWORD bytesRead;

// read data from original file, modify it, and write to new file
while (ReadFile(hOriginalFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
    // modify data with custom function
    // ...

    // set file pointer to beginning of new file
    SetFilePointer(hNewFile, 0, NULL, FILE_BEGIN);

    // write modified data to new file
    DWORD bytesWritten;
    if (!WriteFile(hNewFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead) {
        // handle error
    }

    // set file pointer to end of new file
    SetFilePointer(hNewFile, 0, NULL, FILE_END);
}

CloseHandle(hOriginalFile);
CloseHandle(hNewFile);
