#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*-----------------
	FileUtils
------------------*/

// C++17에서 추가가 된 애,
// 예전에는 표준에 입출력함수가없어서
// windows였으면 FILE*로 윈도우 의존을 했었음
// --> 지금은 어느정도 표준에
//==> 서버코어 속성에서 C/C++ > 언어 > 17표준적용
namespace fs = std::filesystem;

// 경로를 받아서 표준 입출력함수로 파일을 긁어옴
Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	Vector<BYTE> ret;

	// directory를 긁어오는 편리한 기능이 많음
	fs::path filePath{ path };

	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath));
	ret.resize(fileSize);

	// 경로를 받아서 그 경로를 input으로 한다음에 긁어서
	basic_ifstream<BYTE> inputStream{ filePath };
	inputStream.read(&ret[0], fileSize);

	// 뱉어주기
	return ret;
}

// utf-8로 받아온 애를 utf-16으로 바꾸기
String FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	String ret;
	if (srcLen == 0)
		return ret;

							// 윈도우즈 함수
	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
