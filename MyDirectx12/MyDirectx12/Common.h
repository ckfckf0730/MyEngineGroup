#pragma once

#include<string>
#include <wtypes.h>

void ShowMsgBox(LPCWSTR title, LPCWSTR msg);
void ShowMsgBox(LPCWSTR title, const char* msg);
void ShowMsgBox(LPCWSTR title, int msg);

void PrintDebug(LPCWSTR msg);
void PrintDebug(const char* msg);
void PrintDebug(int msg);
void PrintDebug(float msg);

std::wstring GetWideStringFromString(const std::string& str);
std::wstring GetWideStringFromString(const char* str);

std::string GetExtension(const std::string& path);
std::pair<std::string, std::string> SplitFileName(
	const std::string& path, const char splitter = '*');
std::string GetFileName(const std::string& filePath);
std::string GetDirectoryName(const std::string& filePath);