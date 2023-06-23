#include <windows.h>
//#include <synchapi.h>
#include<iostream>
#include <concurrent_queue.h>
#include "D3DAPI.h"

using namespace std;

void ShowMsgBox(LPCWSTR title, LPCWSTR msg)
{
#ifdef _DEBUG

	MessageBoxW(nullptr, msg, title, MB_OK | MB_ICONINFORMATION);

#endif // _DEBUG
}

void ShowMsgBox(LPCWSTR title, const char* msg)
{
#ifdef _DEBUG
	int size = MultiByteToWideChar(CP_UTF8, 0, msg, -1, nullptr, 0);

	// Allocate memory for the wchar_t* string
	wchar_t* wstr = new wchar_t[size];

	// Convert the char* string to wchar_t* string
	MultiByteToWideChar(CP_UTF8, 0, msg, -1, wstr, size);


	MessageBoxW(nullptr, wstr, title, MB_OK | MB_ICONINFORMATION);
	delete[] wstr;

#endif // _DEBUG
}

void ShowMsgBox(LPCWSTR title, int msg)
{
#ifdef _DEBUG
	WCHAR wChar[20];
	_itow_s(msg, wChar, 20, 10);
	MessageBoxW(nullptr, wChar, title, MB_OK | MB_ICONINFORMATION);

#endif // _DEBUG
}


//-----------------console debug---------------------

concurrency::concurrent_queue<LPWSTR> LogQueue;

void PrintDebug(int msg)
{
#ifdef _DEBUG
	WCHAR wChar[30];
	_itow_s(msg, wChar, 30, 10);
	PrintDebug(wChar);

#endif // _DEBUG
}

void PrintDebug(float msg)
{
#ifdef _DEBUG
	WCHAR wChar[30];
	swprintf(wChar, sizeof(msg) / sizeof(wchar_t), L"%f", msg);
	_itow_s(msg, wChar, 30, 10);
	PrintDebug(wChar);

#endif // _DEBUG
}

void PrintDebug(LPCWSTR msg)
{
#ifdef _DEBUG
	int length = wcslen(msg) + 1;

	LPWSTR dMsg = new WCHAR[length];
	std::copy(msg, msg + length, dMsg);
	LogQueue.push(dMsg);

#endif // _DEBUG
}

void PrintDebug(const char* str)
{
#ifdef _DEBUG
	auto wMsg = GetWideStringFromString(str);
	int length = wMsg.length();
	LPWSTR dMsg = new WCHAR[length];
	std::copy(wMsg.begin(), wMsg.end(), dMsg);
	LogQueue.push(dMsg);

#endif // _DEBUG
}

#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall TryGetLog(int& isGet, LPWSTR _msg);
#ifdef __cplusplus 
}
#endif

void __declspec(dllexport) __stdcall TryGetLog(int& isGet, LPWSTR _msg)
{
	LPWSTR msg;
	bool result = LogQueue.try_pop(msg);
	if (result) 
	{ 
		isGet = 1; 
		wcscpy_s(_msg, 200, msg);
		delete[] msg;
	}
	else { isGet = 0; }
}
