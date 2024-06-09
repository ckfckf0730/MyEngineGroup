#include <string>
#include <windows.h>
#include <cassert>
#include <iostream>
#include "Common.h"

std::wstring GetWideStringFromString(const std::string& str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		nullptr,
		0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0],
		num1);

	assert(num1 == num2);
	return wstr;
}

std::wstring GetWideStringFromString(const char* str)
{
	auto num1 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str,
		-1,
		nullptr,
		0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str,
		-1,
		&wstr[0],
		num1);

	assert(num1 == num2);
	return wstr;
}

std::string GetExtension(const std::string& path)
{
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

std::pair<std::string, std::string> SplitFileName(
	const std::string& path, const char splitter )
{
	int idx = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx+1, path.length() - idx - 1);
	return ret;
}


std::string GetFileName(const std::string& filePath)
{
	const char* path = filePath.c_str();
	const char* fileNameUnix = strrchr(path, '/');
	const char* fileNameWindows = strrchr(path, '\\');

	const char* fileName = nullptr;

	if (fileNameUnix && fileNameWindows) 
	{
		fileName = (fileNameUnix > fileNameWindows) ? fileNameUnix : fileNameWindows;
	}
	else if (fileNameUnix) 
	{
		fileName = fileNameUnix;
	}
	else if (fileNameWindows) 
	{
		fileName = fileNameWindows;
	}

	if (fileName)
	{
		return std::string(fileName + 1);
	}
	else
	{
		return filePath;
	}
}

std::string GetDirectoryName(const std::string& filePath)
{
	const char* path = filePath.c_str();
	const char* lastSlashUnix = strrchr(path, '/');
	const char* lastSlashWindows = strrchr(path, '\\');

	const char* lastSlash = nullptr;

	if (lastSlashUnix && lastSlashWindows) 
	{
		lastSlash = (lastSlashUnix > lastSlashWindows) ? lastSlashUnix : lastSlashWindows;
	}
	else if (lastSlashUnix) 
	{
		lastSlash = lastSlashUnix;
	}
	else if (lastSlashWindows) 
	{
		lastSlash = lastSlashWindows;
	}

	if (lastSlash)
	{
		return std::string(path, lastSlash - path);  
	}
	else
	{
		return ""; 
	}
}

