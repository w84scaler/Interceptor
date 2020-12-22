#include "pch.h"
#include "detours.h"
#include <iostream>
#include <fstream>
#include <sstream>
#pragma comment(lib,"detours.lib")
using namespace std;

wofstream outputFile;

HANDLE(WINAPI* pCreateFile) (LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = CreateFile;
BOOL(WINAPI* pReadFile) (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) = ReadFile;
LSTATUS(WINAPI* pRegSetValue) (HKEY, LPCWSTR, DWORD, LPCWSTR, DWORD) = RegSetValue;
LONG(WINAPI* pRegOpenKey) (HKEY, LPCWSTR, PHKEY) = RegOpenKey;
LSTATUS(WINAPI* pRegCloseKey) (HKEY) = RegCloseKey;
BOOL(WINAPI* pDeleteFile) (LPCWSTR) = DeleteFile;
LSTATUS(WINAPI* pRegDeleteKey) (HKEY hKey, LPCWSTR lpSubKey) = RegDeleteKey;
LSTATUS(WINAPI* pRegCreateKey) (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult) = RegCreateKey;

VOID writeToLogFile(wstring);

HANDLE WINAPI interceptionCreateFile(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
BOOL WINAPI interceptionReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL WINAPI interceptionDeleteFile(LPCWSTR);
LSTATUS WINAPI interceptionRegSetValue(HKEY, LPCWSTR, DWORD, LPCWSTR, DWORD);
LONG WINAPI interceptionRegOpenKey(HKEY, LPCWSTR, PHKEY);
LSTATUS WINAPI interceptionRegCloseKey(HKEY);
LSTATUS WINAPI interceptionRegDeleteKey(HKEY, LPCWSTR);
LSTATUS WINAPI interceptionRegCreateKey(HKEY, LPCWSTR, PHKEY);

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            TCHAR str[MAX_PATH] = { 0 };
            GetModuleFileName(hModule, str, MAX_PATH);
            WCHAR sep = '/';

            #ifdef _WIN32
            sep = '\\';
            #endif

            std::wstring::size_type pos = std::wstring(str).find_last_of(sep);
            wstring path = wstring(str).substr(0, pos);
            outputFile.open(path + L"\\log.txt");

            DisableThreadLibraryCalls(hModule);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)pCreateFile, interceptionCreateFile);
            DetourAttach(&(PVOID&)pReadFile, interceptionReadFile);
            DetourAttach(&(PVOID&)pDeleteFile, interceptionDeleteFile);
            DetourAttach(&(PVOID&)pRegSetValue, interceptionRegSetValue);
            DetourAttach(&(PVOID&)pRegOpenKey, interceptionRegOpenKey);
            DetourAttach(&(PVOID&)pRegCloseKey, interceptionRegCloseKey);
            DetourAttach(&(PVOID&)pRegCreateKey, interceptionRegCreateKey);

            DetourTransactionCommit();
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourDetach(&(PVOID&)pCreateFile, interceptionCreateFile);
            DetourDetach(&(PVOID&)pReadFile, interceptionReadFile);
            DetourDetach(&(PVOID&)pDeleteFile, interceptionDeleteFile);
            DetourDetach(&(PVOID&)pRegSetValue, interceptionRegSetValue);
            DetourDetach(&(PVOID&)pRegOpenKey, interceptionRegOpenKey);
            DetourDetach(&(PVOID&)pRegCloseKey, interceptionRegCloseKey);
            DetourDetach(&(PVOID&)pRegCreateKey, interceptionRegCreateKey);

            DetourTransactionCommit();
        }
        break;
    }
    return TRUE;
}

HANDLE WINAPI interceptionCreateFile(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    wostringstream message;
    message << L"CreateFile() was called. File name: " << (wstring)lpFileName << L"\n";
    writeToLogFile(message.str());
    return pCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI interceptionReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    wostringstream message;
    message << L"ReadFile() was called. Bytes to read: " << nNumberOfBytesToRead << L"\n";
    writeToLogFile(message.str());
    return pReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

LSTATUS WINAPI interceptionRegSetValue(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData) {
    wostringstream message;
    message << L"RegSetValue() was called. Subkey: " << (wstring)lpSubKey << L". Data: " << (wstring)lpData << L"\n";
    writeToLogFile(message.str());
    return pRegSetValue(hKey, lpSubKey, dwType, lpData, cbData);
}

LONG WINAPI interceptionRegOpenKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult) {
    wostringstream message;
    message << L"RegOpenKey() was called. Subkey: " << (wstring)lpSubKey << L"\n";
    writeToLogFile(message.str());
    return pRegOpenKey(hKey, lpSubKey, phkResult);
}

LSTATUS WINAPI interceptionRegCloseKey(HKEY hKey) {
    wostringstream message;
    message << L"RegCloseKey() was called.\n";
    writeToLogFile(message.str());
    return pRegCloseKey(hKey);
}

BOOL WINAPI interceptionDeleteFile(LPCWSTR lpFileName)
{
    wostringstream message;
    message << L"DeleteFile() was called. File name: " << (wstring)lpFileName << L"\n";
    writeToLogFile(message.str());
    return pDeleteFile(lpFileName);
}

LSTATUS WINAPI interceptionRegDeleteKey(HKEY hKey, LPCWSTR lpSubKey)
{
    wostringstream message;
    message << L"RegDeleteKey() was called. Subkey: " << (wstring)lpSubKey << L"\n";
    writeToLogFile(message.str());
    return pRegDeleteKey(hKey, lpSubKey);
}

LSTATUS WINAPI interceptionRegCreateKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
    wostringstream message;
    message << L"RegCreateKey() was called. Subkey: " << (wstring)lpSubKey << L"\n";
    writeToLogFile(message.str());
    return pRegCreateKey(hKey, lpSubKey, phkResult);
}

VOID writeToLogFile(wstring message) {
    if (outputFile.is_open()) {
        outputFile << message << endl;
        outputFile.flush();
    }
}