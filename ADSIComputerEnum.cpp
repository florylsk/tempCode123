// ConsoleApplication3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <objbase.h>
#include <wchar.h>
#include <activeds.h>
#include <sddl.h>
#include <comutil.h>
#include <string.h>
#include <stdio.h>


HRESULT FindComputers(IDirectorySearch *pContainerToSearch);  //  IDirectorySearch pointer to the container to search.
	


//  Entry point for the application.
void wmain(int argc, wchar_t *argv[])
{
	//  Initialize COM.
	CoInitialize(NULL);
	HRESULT hr = S_OK;
	//  Get rootDSE and the current user domain container distinguished name.
	IADs *pObject = NULL;
	IDirectorySearch *pContainerToSearch = NULL;
	LPOLESTR szPath = new OLECHAR[MAX_PATH];
	BOOL bReturnVerbose = FALSE;
	DWORD dwLength = MAX_PATH * 2;
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
		hr = pObject->Get(_bstr_t("defaultNamingContext"), &var);
		if (SUCCEEDED(hr))
		{
			//wprintf(L"bstrVal: %s\n", var.bstrVal);
			
			//  Build path to the domain container.
		   // wcsncpy_s(szPath, L"LDAP://", MAX_PATH);
		  // wcsncat_s(szPath, var.bstrVal, MAX_PATH - wcslen(szPath));
			
		   
		   //hr = ADsOpenObject(szPath,
		   hr = ADsOpenObject(L"LDAP://OU=IA Computers,OU=MyDept,DC=Test,Dc=com",
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION, //  Use Secure Authentication.
				IID_IDirectorySearch,
				(void**)&pContainerToSearch);

			if (SUCCEEDED(hr))
			{
				hr = FindComputers(pContainerToSearch); //  IDirectorySearch pointer to domainDNS container.
					
				if (SUCCEEDED(hr))
				{
					if (S_FALSE == hr)
						wprintf(L"Computer object cannot be found.\n");
				}
				else if (E_ADS_INVALID_FILTER == hr)
					wprintf(L"Cannot execute query. Invalid filter was specified.\n");
				else
					wprintf(L"Query failed to run. HRESULT: %x\n", hr);

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
	delete[] szPath;

	getchar();
	
}


HRESULT FindComputers(IDirectorySearch *pContainerToSearch)  //  IDirectorySearch pointer to the container to search.
{
	if (!pContainerToSearch)
		return E_POINTER;
	DWORD dwLength = (MAX_PATH * 2);
	// Create search filter.
	LPOLESTR pszSearchFilter = new OLECHAR[dwLength];

	//  Add the filter.
	pszSearchFilter = L"((objectCategory=computer))";
	
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

	LPOLESTR pszNonVerboseList[] = { L"name", L"distinguishedName" };

	LPOLESTR szName = new OLECHAR[MAX_PATH];
	LPOLESTR szDN = new OLECHAR[MAX_PATH];
		
	int iCount = 0;
	DWORD x = 0L;
	

	//  Return non-verbose list properties only.
	hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
		pszNonVerboseList,
		sizeof(pszNonVerboseList) / sizeof(LPOLESTR),
		&hSearch
		);

	if (SUCCEEDED(hr))
	{
		//  Call IDirectorySearch::GetNextRow() to retrieve the next data row.
		hr = pContainerToSearch->GetFirstRow(hSearch);
		if (SUCCEEDED(hr))
		{
			while (hr != S_ADS_NOMORE_ROWS)
			{
				//  Keep track of count.
				iCount++;
				
				//  Loop through the array of passed column names,
				//  print the data for each column.

				while (pContainerToSearch->GetNextColumnName(hSearch, &pszColumn) != S_ADS_NOMORE_COLUMNS)
				{
					hr = pContainerToSearch->GetColumn(hSearch, pszColumn, &col);
					if (SUCCEEDED(hr))
					{
						//  Verbose handles only the two single-valued attributes: cn and ldapdisplayname,
						//  so this is a special case.
						if (0 == wcscmp(L"name", pszColumn))
						{
							//wcscpy_s(szName, col.pADsValues->CaseIgnoreString);
							szName = col.pADsValues->CaseIgnoreString;
						}
						if (0 == wcscmp(L"distinguishedName", pszColumn))
						{
							//wcscpy_s(szDN, col.pADsValues->CaseIgnoreString);
							szDN = col.pADsValues->CaseIgnoreString;
						}
			
						pContainerToSearch->FreeColumn(&col);
					}
					FreeADsMem(pszColumn);
				}
				
				wprintf(L"%s\n  DN: %s\n\n", szName, szDN);

				//  Get the next row.
				hr = pContainerToSearch->GetNextRow(hSearch);
			}

		}
		//  Close the search handle to cleanup.
		pContainerToSearch->CloseSearchHandle(hSearch);
	}
	if (SUCCEEDED(hr) && 0 == iCount)
		hr = S_FALSE;

	delete[] szName;
	delete[] szDN;
	delete[] pszSearchFilter;
	return hr;
}
