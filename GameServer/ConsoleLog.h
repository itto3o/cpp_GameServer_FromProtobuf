#pragma once

/*---------------
	ConsoleLog
----------------*/
// 색상을 선택해서 로그를 찌긍ㄹ 수 있다. CoreGlobal.h에 추가
enum class Color
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class ConsoleLog
{
	enum { BUFFER_SIZE = 4096 };

public:
	ConsoleLog();
	~ConsoleLog();

public:
	void		WriteStdOut(Color color, const WCHAR* str, ...);
	void		WriteStdErr(Color color, const WCHAR* str, ...);

protected:
	void		SetColor(bool stdOut, Color color);

private:
	HANDLE		_stdOut;
	HANDLE		_stdErr;
};