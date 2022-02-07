#include <iostream.h>


///////////////////////////
//���������� �� 1 ���������
struct CPointerInfo {
	void* p;
	int count;
	CPointerInfo* next;
};

///////////////////////////////////
//������ ������ ��-��� CPointerInfo
static CPointerInfo* O2M_CLEANER_ROOT;


//-----------------------------------------------------------------------------
//���������� �������� ������ ��� ��������� ���������
void O2M_CLEANER_INC(void* p)
{
	//�������� ������� ���������� ��������� (������� ��. �� �����������)
	if (p) {
		//����� ��������� � ������
		CPointerInfo* pi;
		for (pi = O2M_CLEANER_ROOT; pi; pi = pi->next)
			if (pi->p == p) {
				//���������� �������� ������
				pi->count++;
				return;
			}
		//��. � ������ �� ������ - �������� ������ ��-�� ������
		pi = new CPointerInfo;
		pi->p = p;
		pi->count = 1;
		//������� ������ ��-�� � ������ ������
		pi->next = O2M_CLEANER_ROOT ? O2M_CLEANER_ROOT : 0;
		O2M_CLEANER_ROOT = pi;
	}//if
}


//-----------------------------------------------------------------------------
//���������� �������� ������ ��� ��������� ���������
//������� - ������� ��������� �������� ������
bool O2M_CLEANER_DEC(void* p)
{
	//�������� ������� ���������� ��������� (������� ��. �� �����������)
	if (p) {
		//����� ��������� � ������
		CPointerInfo* pi = O2M_CLEANER_ROOT;
		CPointerInfo* prev_pi = 0;
		while (pi) {
			if (pi->p == p) {
				//���������� �������� ������
				pi->count--;
				//�������� ������������� �������� ���������
				if (0 >= pi->count) {
					//�������� ��������� �� ������
					if (prev_pi)
						prev_pi->next = pi->next;
					else
						O2M_CLEANER_ROOT = pi->next;
					delete pi;
					//������ �� ��. ������ ���, ��������� ����� �������
					return true;
				} else //�� ��������� ��� ���� ������
					return false;
			}
			//������� � ���������� ��-�� ������
			prev_pi = pi;
			pi = pi->next;
		}//while
		//��������, � ��������� ��������� ���������� ��������������������� ���������
		return false;
	}
	//������� ��. ������� �� ���������, ������� ���������� false
	return false;
}


//////////////////////////////////
//������ ���������� ���� ���������

// VAR TRec = RECORD END;
struct TRec {
};

// VAR pA, pB : POINTER TO TRec;
static TRec* pA;
static TRec* pB;


//-----------------------------------------------------------------------------
//������ ��������� ��������� ������������
bool O2M_CLEAN_TRec(void* p)
{
	if (O2M_CLEANER_DEC(p)) {
		delete static_cast<TRec*>(p);
		p = 0;
		return true;
	} else
		return false;
}


//-----------------------------------------------------------------------------
//����������� ������ ���������� (DEBUG)
void O2M_CLEANER_DebugPrint(const char* comment)
{
	cout << comment << endl;
	if (!O2M_CLEANER_ROOT) {
		cout << "\tList empty" << endl;
		return;
	}
	int i = 1;
	for (CPointerInfo* pi = O2M_CLEANER_ROOT; pi; pi = pi->next, i++)
		cout << "\t" << "�" << i << ": (" << pi->p << ", " << pi->count << ")" << endl;
}


//-----------------------------------------------------------------------------
//�������� ������ ����� ���������
//PROCEDURE TestProc(p : POINTER TO TRec);
void TestProc(TRec* p)
{
	//��� �� VAR ���������� ��������������� ������� ������������ �������� �����������
	//��������� => ��������� ���������� �������� ������
	O2M_CLEANER_INC(p);

	//VAR pC : POINTER TO TRec;
	TRec* pC = NULL;

	//NEW(pC);
	O2M_CLEAN_TRec(pC);
	pC = new(TRec);
	O2M_CLEANER_INC(pC);

	//���������� ������
	O2M_CLEANER_DebugPrint("TestProc(pA): NEW(pC);");

	//��� ������ �� ������� ��������� ���������� ������� ��������� ����������
	O2M_CLEAN_TRec(pC);

	//��� ������ �� ������� ��������� ���������� ��������� ������� ������
	//(������ ��� �� VAR ����������)
	O2M_CLEAN_TRec(p);
}


//-----------------------------------------------------------------------------
//������������ ��������� �������� ������
int main()
{
	//NEW(pA);
	O2M_CLEAN_TRec(pA);
	pA = new(TRec);
	O2M_CLEANER_INC(pA);

	//���������� ������
	O2M_CLEANER_DebugPrint("NEW(pA);");

	TestProc(pA);

	//���������� ������
	O2M_CLEANER_DebugPrint("TestProc(pA); (* after call *)");

	//pB := pA;
	{
		void* O2M_TMP_POINTER = pB;
		pB = pA;
		O2M_CLEANER_INC(pA);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//���������� ������
	O2M_CLEANER_DebugPrint("pB := pA;");

	//pA := pB;
	{
		void* O2M_TMP_POINTER = pA;
		pA = pB;
		O2M_CLEANER_INC(pB);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//���������� ������
	O2M_CLEANER_DebugPrint("pA := pB;");

	//NEW(pB);
	O2M_CLEAN_TRec(pB);
	pB = new(TRec);
	O2M_CLEANER_INC(pB);

	//���������� ������
	O2M_CLEANER_DebugPrint("NEW(pB);");

	//pA := NIL;
	{
		void* O2M_TMP_POINTER = pA;
		pA = 0;
		O2M_CLEANER_INC(0);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//���������� ������
	O2M_CLEANER_DebugPrint("pA := NIL;");

	//pB := NIL;
	{
		void* O2M_TMP_POINTER = pB;
		pB = 0;
		O2M_CLEANER_INC(0);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//���������� ������
	O2M_CLEANER_DebugPrint("pB := NIL;");

	return 0;
}