#pragma once
#include "Types.h"
#include "Container.h"
#include "rapidxml.hpp"
using namespace rapidxml;

/*-------------
	XmlNode
--------------*/

// 타입 정의, WCHAR를 사용할 것, rapidXML에서 지원해주는 애
using XmlNodeType = xml_node<WCHAR>;
using XmlDocumentType = xml_document<WCHAR>;
using XmlAttributeType = xml_attribute<WCHAR>;

class XmlNode
{
public:
	XmlNode(XmlNodeType* node = nullptr) : _node(node) { }

	bool				IsValid() { return _node != nullptr; }
						
						// 이 노드에서 key 값에 해당하는 attr이 있는지찾아봄,
						// 있다고 하면 문자열 비교를 통해서 true라고 하면 true
						// attr는 name="id", type="int", notnull="true" 등등,
						// value는 <> 사이에 들어가는 값, <Column>123</Column>에서 123
	bool				GetBoolAttr(const WCHAR* key, bool defaultValue = false);
	int8				GetInt8Attr(const WCHAR* key, int8 defaultValue = 0);
	int16				GetInt16Attr(const WCHAR* key, int16 defaultValue = 0);
						// 노드에 key값이 있으면 string일테니까 걔를 int로 wtoi 함수로 작업
	int32				GetInt32Attr(const WCHAR* key, int32 defaultValue = 0);
	int64				GetInt64Attr(const WCHAR* key, int64 defaultValue = 0);
	float				GetFloatAttr(const WCHAR* key, float defaultValue = 0.0f);
	double				GetDoubleAttr(const WCHAR* key, double defaultValue = 0.0);
	const WCHAR*		GetStringAttr(const WCHAR* key, const WCHAR* defaultValue = L"");

	bool				GetBoolValue(bool defaultValue = false);
	int8				GetInt8Value(int8 defaultValue = 0);
	int16				GetInt16Value(int16 defaultValue = 0);
	int32				GetInt32Value(int32 defaultValue = 0);
	int64				GetInt64Value(int64 defaultValue = 0);
	float				GetFloatValue(float defaultValue = 0.0f);
	double				GetDoubleValue(double defaultValue = 0.0);
	const WCHAR* GetStringValue(const WCHAR* defaultValue = L"");

	XmlNode				FindChild(const WCHAR* key);
	Vector<XmlNode>		FindChildren(const WCHAR* key);

private:
	XmlNodeType* _node = nullptr;
};

/*---------------
	XmlParser
----------------*/

class XmlParser
{
public:
	bool ParseFromFile(const WCHAR* path, OUT XmlNode& root);

private:
	shared_ptr<XmlDocumentType>		_document = nullptr;
	String							_data;
};

