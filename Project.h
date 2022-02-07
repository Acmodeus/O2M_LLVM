///////////////////////////////////////////////////////////////////////////
// ����� ��� ������ � ����������� ������ �������� O2M, ����� ������� O2M //
///////////////////////////////////////////////////////////////////////////

#ifndef O2M_Project_h
#define O2M_Project_h

#include "Common.h"
#include "xml_lib.h"


//-----------------------------------------------------------------------------
//����� ��� �������� �������� �������
class CProject {
public:
	CProject();
	~CProject();
	//�������� (����������) O2M makefile
	bool Create2mk();
	//�������� ��� ������ AddMakeFile � ������ ������������� ������� ���� � �����
	void AddFullPathMakeFile(const char *FullPath, const char *ModuleName);
	//���������� ����� � ������ ������ � ��������� ���������� ������������
	void AddMakeFile(const char *FilePath, const char *ModuleName);
	//�������� ������������ �� ���������� �����
	int Init(const char *f_name);
	//��������, �������� �� ��������� ���� ������� ������ �������
	bool IsMainFile(const char* file_name);
	//�������� O2M makefile (���� ����)
	bool Load2mk();
	//��������� ����� O2M makefile ��� ������������ (Link2M) � �������� ������� (Make2M)
	bool Write2mk(const bool is_main, const char* ModuleName);
	//������������ ����� ���� (��� ������)
	int PathMaxLen;
	//������ ���������� � dfn ��������
	typedef StringVector Paths_type;
	Paths_type Paths;
	//������ ���������
	int TabSize;
private:
	//��� ������������ ���� ������� (��� .exe)
	char *EXEC;
	//������� ���� �������
	char *MAIN;
	//������ ��� ��������� O2M makefile
	CXMLFile xmlFile;
};


#endif //O2M_Project_h
