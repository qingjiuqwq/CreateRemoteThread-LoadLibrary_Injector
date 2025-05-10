#include <Windows.h>
#include <stdio.h>  
#include <sys/stat.h> // fastest way to get file existens

#include <fcntl.h>
#include <io.h>

#define MAX_NAME 255
using namespace std;


void cutBNW(wchar_t* str) {
	size_t ln = wcslen(str) - 1;
	if (*str && str[ln] == '\n')
		str[ln] = '\0';
}
void cutBracketsW(wchar_t* str) {
	size_t ln = wcslen(str) - 1;
	if (*str && str[0] == '\"' && str[ln] == '\"')
	{
		for (size_t i = 0; i < ln - 1; i++)
		{
			str[i] = str[i + 1];
		}
		str[ln - 1] = '\0';
	}
}
int main()
{
	if (!_setmode(_fileno(stdout), _O_U16TEXT)) throw CREATE_UNICODE_ENVIRONMENT;
	if (!_setmode(_fileno(stdin), _O_U16TEXT)) throw CREATE_UNICODE_ENVIRONMENT;

	wprintf(L"This is the CreateRemoteThread+LoadLibrary injector [DirectX9, VisualC++ needed] \n");

findProcId:
	wprintf(L"Enter window name \n");

	wchar_t targetWindowName[MAX_NAME];
	fgetws(targetWindowName, MAX_NAME, stdin);
	cutBNW(targetWindowName);


	DWORD targetProcId = NULL;
	HWND targetHwnd = FindWindowW(NULL, targetWindowName);
	GetWindowThreadProcessId(targetHwnd, &targetProcId);
	wprintf(L"ProcID == %p \n", (void*)(targetProcId));

	if (targetProcId == NULL) {
		wprintf(L"No processes with %s window name found, try again \n", targetWindowName);
		goto findProcId;
	}


findDllPath:
	wprintf(L"Enter dll path \n");
	wchar_t targetDllPath[MAX_NAME];
	fgetws(targetDllPath, MAX_NAME, stdin);
	cutBNW(targetDllPath);
	cutBracketsW(targetDllPath);


	struct _stat buffer;
	if (_wstat(targetDllPath, &buffer) == 0) { // fastest way to get file existens
		wprintf(L"file %s actually exists \n", targetDllPath);
	}
	else {
		wprintf(L"No file like %s found, try again \n", targetDllPath);
		goto findDllPath;
	}

	unsigned short targetDllPathLength = sizeof(targetDllPath);

	HANDLE targetOpened = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcId);
	wprintf(L"OpenProcess(PROCESS_ALL_ACCESS): %p \n", targetDllPath);

	LPVOID allocatedMem = VirtualAllocEx(targetOpened, 0, targetDllPathLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	wprintf(L"VirtualAllocEx: %p \n", targetDllPath);
	if (allocatedMem == nullptr) throw FWP_E_NULL_POINTER;

	bool wpm = WriteProcessMemory(targetOpened, allocatedMem, targetDllPath, targetDllPathLength, 0);
	wprintf(L"WriteProcessMemory: %d \n", wpm);

	HANDLE rt = CreateRemoteThread(targetOpened, 0, 0, LPTHREAD_START_ROUTINE(LoadLibraryW), allocatedMem, 0, 0);
	wprintf(L"CreateRemoteThread: %p \n", rt);

	bool ch = CloseHandle(targetOpened);
	wprintf(L"CloseHandle: %d \n", ch);

	system("pause");
}