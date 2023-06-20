#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*-----------------
	FileUtils
------------------*/

// C++17���� �߰��� �� ��,
// �������� ǥ�ؿ� ������Լ������
// windows������ FILE*�� ������ ������ �߾���
// --> ������ ������� ǥ�ؿ�
//==> �����ھ� �Ӽ����� C/C++ > ��� > 17ǥ������
namespace fs = std::filesystem;

// ��θ� �޾Ƽ� ǥ�� ������Լ��� ������ �ܾ��
Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	Vector<BYTE> ret;

	// directory�� �ܾ���� ���� ����� ����
	fs::path filePath{ path };

	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath));
	ret.resize(fileSize);

	// ��θ� �޾Ƽ� �� ��θ� input���� �Ѵ����� �ܾ
	basic_ifstream<BYTE> inputStream{ filePath };
	inputStream.read(&ret[0], fileSize);

	// ����ֱ�
	return ret;
}

// utf-8�� �޾ƿ� �ָ� utf-16���� �ٲٱ�
String FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size());

	String ret;
	if (srcLen == 0)
		return ret;

							// �������� �Լ�
	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
