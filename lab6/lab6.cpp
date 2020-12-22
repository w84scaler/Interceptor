#include <iostream>
#include <windows.h>
using namespace std;

int main()
{
    char injectDLLName[] = ".\\InterceptorLib.dll";
    STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION procInfo;

    wstring CommandLine(L"TestApp.exe");

    LPWSTR lpwCmdLine = &CommandLine[0];

    ZeroMemory(&procInfo, sizeof(procInfo));

    startupInfo.cb = sizeof(startupInfo);

    if (!CreateProcess(NULL, lpwCmdLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &procInfo)) {
        cout << "CreateProcess() failed. Error: " << GetLastError() << endl;
        return -1;
    }


    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, procInfo.dwProcessId);
    if (process == NULL)
    {
        cout << "OpenProcess() failed. Error: " << GetLastError() << endl;
        return 1;
    }

    HMODULE hKernelModule = GetModuleHandle(TEXT("kernel32.dll"));
    if (hKernelModule == NULL)
    {
        cout << "GetModuleHandle() failed. Error: " << GetLastError() << endl;
        return 1;
    }

    FARPROC LoadLibraryAddress = GetProcAddress(hKernelModule, "LoadLibraryA");
    if (LoadLibraryAddress == NULL)
    {
        cout << "GetProcAddress() failed. Error: " << GetLastError() << endl;
        return 1;
    }

    PVOID rmMemory = VirtualAllocEx(process, NULL, strlen(injectDLLName) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (rmMemory == NULL)
    {
        cout << "VirtualAllocEx() failed. Error: " << GetLastError() << endl;
        return 1;
    }

    WriteProcessMemory(process, (LPVOID)rmMemory, injectDLLName, strlen(injectDLLName) + 1, NULL);
    HANDLE rmThread = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddress, (LPVOID)rmMemory, NULL, NULL);
    if (rmThread == NULL)
    {
        cout << "CreateRemoteThread() failed. Error: " << GetLastError() << endl;
        return 1;
    }
    else {
        cout << "Injection complete" << endl;
    }

    ResumeThread(procInfo.hThread);

    WaitForSingleObject(procInfo.hProcess, INFINITE);
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
    return 0;
};