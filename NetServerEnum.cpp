#ifndef UNICODE
#define UNICODE
#endif
#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <lm.h>

int wmain(int argc, wchar_t * argv[])
{
    LPSERVER_INFO_101 pBuf = NULL;
    LPSERVER_INFO_101 pTmpBuf;
    DWORD dwLevel = 101;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwTotalCount = 0;
    DWORD dwServerType = SV_TYPE_SERVER;        // all servers
    DWORD dwResumeHandle = 0;
    NET_API_STATUS nStatus;
    LPWSTR pszServerName = NULL;
    LPWSTR pszDomainName = NULL;
    DWORD i;

    if (argc > 2) 
    {
        fwprintf(stderr, L"Usage: %s [DomainName]\n", argv[0]);
        exit(1);
    }
    // The request is not for the primary domain.
    //
    if (argc == 2)
        pszDomainName = argv[1];
    //
    // Call the NetServerEnum function to retrieve information
    //  for all servers, specifying information level 101.
    //
    nStatus = NetServerEnum(pszServerName,
                            dwLevel,
                            (LPBYTE *) & pBuf,
                            dwPrefMaxLen,
                            &dwEntriesRead,
                            &dwTotalEntries,
                            dwServerType, 
                            pszDomainName, 
                            &dwResumeHandle);
    //
    // If the call succeeds,
    //
    if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
        if ((pTmpBuf = pBuf) != NULL) {
            //
            // Loop through the entries and 
            //  print the data for all server types.
            //
            for (i = 0; i < dwEntriesRead; i++) {
                assert(pTmpBuf != NULL);

                if (pTmpBuf == NULL) {
                    fprintf(stderr, "An access violation has occurred\n");
                    break;
                }

                printf("\tPlatform: %d\n", pTmpBuf->sv101_platform_id);
                wprintf(L"\tName:     %s\n", pTmpBuf->sv101_name);
                printf("\tVersion:  %d.%d\n",
                       pTmpBuf->sv101_version_major,
                       pTmpBuf->sv101_version_minor);
                printf("\tType:     %d", pTmpBuf->sv101_type);
                //
                // Check to see if the server is a domain controller;
                //  if so, identify it as a PDC or a BDC.
                //
                if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_CTRL)
                    wprintf(L" (PDC)");
                else if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
                    wprintf(L" (BDC)");

                printf("\n");
                //
                // Also print the comment associated with the server.
                //
                wprintf(L"\tComment:  %s\n\n", pTmpBuf->sv101_comment);

                pTmpBuf++;
                dwTotalCount++;
            }
            // Display a warning if all available entries were
            //  not enumerated, print the number actually 
            //  enumerated, and the total number available.

            if (nStatus == ERROR_MORE_DATA) {
                fprintf(stderr, "\nMore entries available!!!\n");
                fprintf(stderr, "Total entries: %d", dwTotalEntries);
            }

            printf("\nEntries enumerated: %d\n", dwTotalCount);

        } else {
            printf("No servers were found\n");
            printf("The buffer (bufptr) returned was NULL\n");
            printf("  entriesread: %d\n", dwEntriesRead);
            printf("  totalentries: %d\n", dwEntriesRead);
        }

    } else
        fprintf(stderr, "NetServerEnum failed with error: %d\n", nStatus);
    //
    // Free the allocated buffer.
    //
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);

    return 0;
}
