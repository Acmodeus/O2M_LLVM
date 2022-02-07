///////////////////////////////////////////////////////
// ����� ������� ��� ������� ����� � ������� XML 1.0 //
// ����� ������� Link2M                              //
// ������ ����������: 1.0.003 �� 2004.08.26          //
///////////////////////////////////////////////////////

#ifndef xml_lib_h_file
#define xml_lib_h_file

#include <ctype.h>
#include <stdio.h>
#include <vector>
#include <cstring>

namespace xml_lib {

//������ ������ ��� ������ �� ����� ����� ���������� (����� ����, ���������, ��������)
const int BufSize = 1000;

//������ �� ���������� ����� ������� � ��������� ���������� ��������
bool freadc(FILE* f, char& ch);
//������ �� ���������� ����� ����� (�����/�����) � ��������� ���������� ��������,
//�������� ����� ������ �� ���������� ������ �� ���������������
bool freads(FILE* f, char* buf, char& ch);
//������ �� ����� � ��������� ���������� ��������� ������-�� ��������
bool freadb(FILE* f, const char* st);
//������� ���� ����������� (������� "<!" ������ ���� ��� �������)
bool SkipComment(FILE* f);
//�������� ���������� ��������� (������ '"' (��� '>') ������ ���� ��� ������)
bool ReadText(FILE* f, char* buf, char& ch);
//�������������� ����������� � ����� ����� � ������ (������������ ������� ������������)
bool ReadNumber(FILE* f, char& ch);

}//namespace


//-----------------------------------------------------------------------------
//���� ������
enum EXMLErrors {
	EXMLNoError,
	//��� ������ �������� xml �����
	EXMLOpen,
	//��� ������ �������� xml �����
	EXMLClose,
	//��� ������ - �������� ������ �����
	EXMLFormat
};


//-----------------------------------------------------------------------------
//��� ���� ��������-�������� ������ xml ��-��
class CXMLAttribute {
public:
	CXMLAttribute(const char* newName, const char* newValue);
	~CXMLAttribute() {
		delete[] Name;
		delete[] Value;
	};
	//�������� ���������
	char* Name;
	//�������� ��������� (����� �������������)
	char* Value;
};


//-----------------------------------------------------------------------------
//����� ��� ��������� ������ xml ��-��
class CXMLEntry {
	void AddContent(const char* buf);
public:
	void AddExtContent(const char* buf);
	//��������� �������� �������� (��� NULL) �� �����
	const char* AttribValue(const char* AttribName) const;
	//�����������
	CXMLEntry() : Content(NULL), ExtContent(NULL), TagName(NULL) {};
	~CXMLEntry();
	bool Init(FILE* f, char& ch);
	//��� ������ ��� ��������-��������
	typedef std::vector<CXMLAttribute*> TAttribStore;
	//������ ��� ��������-��������
	TAttribStore AttribStore;
	//����� ������ ���� (�� ������� ���������� ����)
	char* Content;
	//�����, ��������� ����� ���� (����������� ������������ �����)
	char* ExtContent;
	//��� xml ����
	char* TagName;
	//��� ������ xml ��-���
	typedef std::vector<CXMLEntry*> TXMLEntryStore;
	//������ xml ��-���
	TXMLEntryStore XMLEntryStore;
};


//-----------------------------------------------------------------------------
//����� ��� ��������� ������ xml �����
class CXMLFile {
public:
	CXMLFile() : XMLEntry(NULL) {};
	~CXMLFile() {
		Clear();
	};
	//������� ����������� �������, ���������� � ���������� ������ ������ Load
	void Clear() {
		delete XMLEntry;
		XMLEntry = NULL;
	};
	//�������� ����� � ������� XML
	EXMLErrors Load(const char* fname);
	//������� ��� ���������
	CXMLEntry* XMLEntry;
};


#endif	//xml_lib_h_file
