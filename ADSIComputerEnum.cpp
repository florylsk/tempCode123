#include "stdafx.h"
#include <objbase.h>
#include <wchar.h>
#include <activeds.h>
//  Define UNICODE.
//  Define version 5 for Windows 2000.
#define _WIN32_WINNT 0x0500
#include <sddl.h>

HRESULT FindUsers(IDirectorySearch *pContainerToSearch,  //  IDirectorySearch pointer to the container to search.
    LPOLESTR szFilter, //  Filter to find specific users.
    //  NULL returns all user objects.
    LPOLESTR *pszPropertiesToReturn, //  Properties to return for user objects found.
    //  NULL returns all set properties.
    BOOL bIsVerbose //  TRUE indicates that display all properties for the found objects.
    //  FALSE indicates that only the RDN.
    );

//  Entry point for the application.
void wmain(int argc, wchar_t *argv[ ])
{
#ifdef _MBCS
    //  Handle the command line arguments.
    DWORD dwLength = MAX_PATH*2;
    LPOLESTR pszBuffer = new OLECHAR[dwLength];
    wcsncpy_s(pszBuffer, L"",dwLength);
    BOOL bReturnVerbose = FALSE;


    for (int i = 1;i<argc;i++)
    {
        if (_wcsicmp(argv[i],L"/V") == 0)
        {
            bReturnVerbose = TRUE;
        }
        else if ((_wcsicmp(argv[i],L"/?") == 0)||
            (_wcsicmp(argv[i],L"-?") == 0))
        {
            wprintf(L"This application queries for users in the current user domain.\n");
            wprintf(L"Syntax: queryusers [/V][querystring]\n");
            wprintf(L"where /V specifies that all properties for the found users should be returned.\n");
            wprintf(L"querystring is the query criteria in ldap query format.\n");
            wprintf(L"Defaults: If no /V is specified, the query returns only the RDN and DN of the items found.\n");
            wprintf(L"If no querystring is specified, the query returns all users.\n");
            wprintf(L"Example: queryusers (sn=Smith)\n");
            wprintf(L"Returns all users with surname Smith.\n");
            return;
        }
        else
        {
            wcsncpy_s(pszBuffer,argv[i],dwLength-wcslen(pszBuffer));
        }
    }
    if (_wcsicmp(pszBuffer,L"") == 0)
        wprintf(L"\nFinding all user objects...\n\n");
    else
        wprintf(L"\nFinding user objects based on query: %s...\n\n", pszBuffer);

    //  Initialize COM.
    CoInitialize(NULL);
    HRESULT hr = S_OK;
    //  Get rootDSE and the current user domain container distinguished name.
    IADs *pObject = NULL;
    IDirectorySearch *pContainerToSearch = NULL;
    LPOLESTR szPath = new OLECHAR[MAX_PATH];
    VARIANT var;
    hr = ADsOpenObject(L"LDAP://rootDSE",
                       NULL,
                       NULL,
                       ADS_SECURE_AUTHENTICATION, // Use Secure Authentication.
                       IID_IADs,
                       (void**)&pObject);
    if (FAILED(hr))
    {
        wprintf(L"Cannot execute query. Cannot bind to LDAP://rootDSE.\n");
        if (pObject)
            pObject->Release();
        return;
    }
    if (SUCCEEDED(hr))
    {
        hr = pObject->Get(_bstr_t("defaultNamingContext"),&var);
        if (SUCCEEDED(hr))
        {
            //  Build path to the domain container.
            wcsncpy_s(szPath,L"LDAP://",MAX_PATH);
            wcsncat_s(szPath,var.bstrVal,MAX_PATH-wcslen(szPath));
            hr = ADsOpenObject(szPath,
                               NULL,
                               NULL,
                               ADS_SECURE_AUTHENTICATION, //  Use Secure Authentication.
                               IID_IDirectorySearch,
                               (void**)&pContainerToSearch);

            if (SUCCEEDED(hr))
            {
                hr = FindUsers(pContainerToSearch, //  IDirectorySearch pointer to domainDNS container.
                               pszBuffer,  
                               NULL, //  Return all properties.
                               bReturnVerbose 
                             );
                if (SUCCEEDED(hr))
                {
                    if (S_FALSE==hr)
                        wprintf(L"User object cannot be found.\n");
                }
                else if (E_ADS_INVALID_FILTER==hr)
                    wprintf(L"Cannot execute query. Invalid filter was specified.\n");
                else
                    wprintf(L"Query failed to run. HRESULT: %x\n",hr);
            }
            else
            {
                wprintf(L"Cannot execute query. Cannot bind to the container.\n");
            }
            if (pContainerToSearch)
                pContainerToSearch->Release();
        }
        VariantClear(&var);
    }
    if (pObject)
        pObject->Release();

    //  Uninitialize COM.
    CoUninitialize();
    delete [] szPath;
    delete [] pszBuffer;
    return;
#endif _MBCS
}

HRESULT FindUsers(IDirectorySearch *pContainerToSearch,  //  IDirectorySearch pointer to the container to search.
    LPOLESTR szFilter, //  Filter for finding specific users.
    //  NULL returns all user objects.
    LPOLESTR *pszPropertiesToReturn, //  Properties to return for user objects found.
    //  NULL returns all set properties.
    BOOL bIsVerbose    //  TRUE indicates that all properties for the found objects are displayed.
    //  FALSE indicates only the RDN.
    )
{
    if (!pContainerToSearch)
        return E_POINTER;
    DWORD dwLength = MAX_PATH*2;
    // Create search filter.
    LPOLESTR pszSearchFilter = new OLECHAR[dwLength];

    //  Add the filter.
    swprintf_s(pszSearchFilter, dwLength, L"(&(objectClass=user)(objectCategory=person)%s)",szFilter);

    //  Specify subtree search.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
    DWORD dwNumPrefs = 1;

    //  COL for iterations.
    LPOLESTR pszColumn = NULL;  
    ADS_SEARCH_COLUMN col;
    HRESULT hr = S_OK;

    //  Interface Pointers
    IADs  *pObj = NULL;
    IADs  * pIADs = NULL;

    //  Search handle.
    ADS_SEARCH_HANDLE hSearch = NULL;

    //  Set search preference.
    hr = pContainerToSearch->SetSearchPreference(&SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    LPOLESTR pszBool = NULL;
    DWORD dwBool = 0;
    PSID pObjectSID = NULL;
    LPOLESTR szSID = NULL;
    LPOLESTR szDSGUID = new WCHAR [39];
    LPGUID pObjectGUID = NULL;
    FILETIME filetime;
    SYSTEMTIME systemtime;
    DATE date;
    VARIANT varDate;
    LARGE_INTEGER liValue;
    LPOLESTR *pszPropertyList = NULL;
    LPOLESTR pszNonVerboseList[] = {L"name",L"distinguishedName"};

    LPOLESTR szName = new OLECHAR[MAX_PATH];
    LPOLESTR szDN = new OLECHAR[MAX_PATH];

    VariantInit(&varDate);

    int iCount = 0;
    DWORD x = 0L;



    if (!bIsVerbose)
    {
        //  Return non-verbose list properties only.
        hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
                                               pszNonVerboseList,
                                               sizeof(pszNonVerboseList)/sizeof(LPOLESTR),
                                               &hSearch
                                             );
    }
    else
    {
        if (!pszPropertiesToReturn)
        {
            //  Return all properties.
            hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
                                                   NULL,
                                                   (DWORD)-1,
                                                   &hSearch
                                                 );
        }
        else
        {
            //  Specified subset.
            pszPropertyList = pszPropertiesToReturn;
            //  Return specified properties.
            hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
                                                   pszPropertyList,
                                                   sizeof(pszPropertyList)/sizeof(LPOLESTR),
                                                   &hSearch
                                                 );
        }
    }
    if (SUCCEEDED(hr))
    {  
        //  Call IDirectorySearch::GetNextRow() to retrieve the next data row.
        hr = pContainerToSearch->GetFirstRow(hSearch);
        if (SUCCEEDED(hr))
        {
            while(hr != S_ADS_NOMORE_ROWS)
            {
                //  Keep track of count.
                iCount++;
                if (bIsVerbose)
                    wprintf(L"----------------------------------\n");
                //  Loop through the array of passed column names,
                //  print the data for each column.

                while(pContainerToSearch->GetNextColumnName(hSearch, &pszColumn) != S_ADS_NOMORE_COLUMNS)
                {
                    hr = pContainerToSearch->GetColumn(hSearch, pszColumn, &col);
                    if (SUCCEEDED(hr))
                    {
                        //  Print the data for the column and free the column.
                        if(bIsVerbose)
                        {
                            //  Get the data for this column.
                            wprintf(L"%s\n",col.pszAttrName);
                            switch (col.dwADsType)
                            {
                            case ADSTYPE_DN_STRING:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    wprintf(L"  %s\r\n",col.pADsValues[x].DNString);
                                }
                                break;
                            case ADSTYPE_CASE_EXACT_STRING:    
                            case ADSTYPE_CASE_IGNORE_STRING:    
                            case ADSTYPE_PRINTABLE_STRING:    
                            case ADSTYPE_NUMERIC_STRING:      
                            case ADSTYPE_TYPEDNAME:        
                            case ADSTYPE_FAXNUMBER:        
                            case ADSTYPE_PATH:          
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    wprintf(L"  %s\r\n",col.pADsValues[x].CaseIgnoreString);
                                }
                                break;
                            case ADSTYPE_BOOLEAN:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    dwBool = col.pADsValues[x].Boolean;
                                    pszBool = dwBool ? L"TRUE" : L"FALSE";
                                    wprintf(L"  %s\r\n",pszBool);
                                }
                                break;
                            case ADSTYPE_INTEGER:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    wprintf(L"  %d\r\n",col.pADsValues[x].Integer);
                                }
                                break;
                            case ADSTYPE_OCTET_STRING:
                                if (_wcsicmp(col.pszAttrName,L"objectSID") == 0)
                                {
                                    for (x = 0; x< col.dwNumValues; x++)
                                    {
                                        pObjectSID = (PSID)(col.pADsValues[x].OctetString.lpValue);
                                        //  Convert SID to string.
                                        ConvertSidToStringSid(pObjectSID, &szSID);
                                        wprintf(L"  %s\r\n",szSID);
                                        LocalFree(szSID);
                                    }
                                }
                                else if ((_wcsicmp(col.pszAttrName,L"objectGUID") == 0))
                                {
                                    for (x = 0; x< col.dwNumValues; x++)
                                    {
                                        //  Cast to LPGUID.
                                        pObjectGUID = (LPGUID)(col.pADsValues[x].OctetString.lpValue);
                                        //  Convert GUID to string.
                                        ::StringFromGUID2(*pObjectGUID, szDSGUID, 39); 
                                        //  Print the GUID.
                                        wprintf(L"  %s\r\n",szDSGUID);
                                    }
                                }
                                else
                                    wprintf(L"  Value of type Octet String. No Conversion.");
                                break;
                            case ADSTYPE_UTC_TIME:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    systemtime = col.pADsValues[x].UTCTime;
                                    if (SystemTimeToVariantTime(&systemtime,
                                        &date) != 0) 
                                    {
                                        //  Pack in variant.vt.
                                        varDate.vt = VT_DATE;
                                        varDate.date = date;
                                        VariantChangeType(&varDate,&varDate,VARIANT_NOVALUEPROP,VT_BSTR);
                                        wprintf(L"  %s\r\n",varDate.bstrVal);
                                        VariantClear(&varDate);
                                    }
                                    else
                                        wprintf(L"  Could not convert UTC-Time.\n",pszColumn);
                                }
                                break;
                            case ADSTYPE_LARGE_INTEGER:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    liValue = col.pADsValues[x].LargeInteger;
                                    filetime.dwLowDateTime = liValue.LowPart;
                                    filetime.dwHighDateTime = liValue.HighPart;
                                    if((filetime.dwHighDateTime==0) && (filetime.dwLowDateTime==0))
                                    {
                                        wprintf(L"  No value set.\n");
                                    }
                                    else
                                    {
                                        //  Verify properties of type LargeInteger that represent time.
                                        //  If TRUE, then convert to variant time.
                                        if ((0==wcscmp(L"accountExpires", col.pszAttrName))|
                                            (0==wcscmp(L"badPasswordTime", col.pszAttrName))||
                                            (0==wcscmp(L"lastLogon", col.pszAttrName))||
                                            (0==wcscmp(L"lastLogoff", col.pszAttrName))||
                                            (0==wcscmp(L"lockoutTime", col.pszAttrName))||
                                            (0==wcscmp(L"pwdLastSet", col.pszAttrName))
                                            )
                                        {
                                            //  Handle special case for Never Expires where low part is -1.
                                            if (filetime.dwLowDateTime==-1)
                                            {
                                                wprintf(L"  Never Expires.\n");
                                            }
                                            else
                                            {
                                                if (FileTimeToLocalFileTime(&filetime, &filetime) != 0) 
                                                {
                                                    if (FileTimeToSystemTime(&filetime,
                                                        &systemtime) != 0)
                                                    {
                                                        if (SystemTimeToVariantTime(&systemtime,
                                                            &date) != 0) 
                                                        {
                                                            //  Pack in variant.vt.
                                                            varDate.vt = VT_DATE;
                                                            varDate.date = date;
                                                            VariantChangeType(&varDate,&varDate,VARIANT_NOVALUEPROP,VT_BSTR);
                                                            wprintf(L"  %s\r\n",varDate.bstrVal);
                                                            VariantClear(&varDate);
                                                        }
                                                        else
                                                        {
                                                            wprintf(L"  FileTimeToVariantTime failed\n");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        wprintf(L"  FileTimeToSystemTime failed\n");
                                                    }

                                                }
                                                else
                                                {
                                                    wprintf(L"  FileTimeToLocalFileTime failed\n");
                                                }
                                            }
                                        }
                                        else
                                        {
                                            //  Print the LargeInteger.
                                            wprintf(L"  high: %d low: %d\r\n",filetime.dwHighDateTime, filetime.dwLowDateTime);
                                        }
                                    }
                                }
                                break;
                            case ADSTYPE_NT_SECURITY_DESCRIPTOR:
                                for (x = 0; x< col.dwNumValues; x++)
                                {
                                    wprintf(L"  Security descriptor.\n");
                                }
                                break;
                            default:
                                wprintf(L"Unknown type %d.\n",col.dwADsType);
                            }
                        }
                        else
                        {
#ifdef _MBCS
                            //  Verbose handles only the two single-valued attributes: cn and ldapdisplayname,
                            //  so this is a special case.
                            if (0==wcscmp(L"name", pszColumn))
                            {
                                wcscpy_s(szName,col.pADsValues->CaseIgnoreString);
                            }
                            if (0==wcscmp(L"distinguishedName", pszColumn))
                            {
                                wcscpy_s(szDN,col.pADsValues->CaseIgnoreString);
                            }
#endif _MBCS
                        }
                        pContainerToSearch->FreeColumn(&col);
                    }
                    FreeADsMem(pszColumn);
                }
                if (!bIsVerbose)
                    wprintf(L"%s\n  DN: %s\n\n",szName,szDN);
                //  Get the next row.
                hr = pContainerToSearch->GetNextRow(hSearch);
            }

        }
        //  Close the search handle to cleanup.
        pContainerToSearch->CloseSearchHandle(hSearch);
    } 
    if (SUCCEEDED(hr) && 0==iCount)
        hr = S_FALSE;

    delete [] szName;
    delete [] szDN;
    delete [] szDSGUID;
    delete [] pszSearchFilter;
    return hr;
}
