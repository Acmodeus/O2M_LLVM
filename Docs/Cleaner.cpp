#include <iostream.h>


///////////////////////////
//информация об 1 указателе
struct CPointerInfo {
	void* p;
	int count;
	CPointerInfo* next;
};

///////////////////////////////////
//начало списка эл-тов CPointerInfo
static CPointerInfo* O2M_CLEANER_ROOT;


//-----------------------------------------------------------------------------
//увеличение счетчика ссылок для заданного указателя
void O2M_CLEANER_INC(void* p)
{
	//проверка наличия ненулевого указателя (нулевые ук. не учитываются)
	if (p) {
		//поиск указателя в списке
		CPointerInfo* pi;
		for (pi = O2M_CLEANER_ROOT; pi; pi = pi->next)
			if (pi->p == p) {
				//увеличение счетчика ссылок
				pi->count++;
				return;
			}
		//ук. в списке не найден - создание нового эл-та списка
		pi = new CPointerInfo;
		pi->p = p;
		pi->count = 1;
		//вставка нового эл-та в начало списка
		pi->next = O2M_CLEANER_ROOT ? O2M_CLEANER_ROOT : 0;
		O2M_CLEANER_ROOT = pi;
	}//if
}


//-----------------------------------------------------------------------------
//уменьшение счетчика ссылок для заданного указателя
//возврат - признак обнуления счетчика ссылок
bool O2M_CLEANER_DEC(void* p)
{
	//проверка наличия ненулевого указателя (нулевые ук. не учитываются)
	if (p) {
		//поиск указателя в списке
		CPointerInfo* pi = O2M_CLEANER_ROOT;
		CPointerInfo* prev_pi = 0;
		while (pi) {
			if (pi->p == p) {
				//уменьшение счетчика ссылок
				pi->count--;
				//проверка необходимости удаления указателя
				if (0 >= pi->count) {
					//удаление указателя из списка
					if (prev_pi)
						prev_pi->next = pi->next;
					else
						O2M_CLEANER_ROOT = pi->next;
					delete pi;
					//ссылок на ук. больше нет, указатель нужно удалять
					return true;
				} else //на указатель еще есть ссылки
					return false;
			}
			//переход к следующему эл-ту списка
			prev_pi = pi;
			pi = pi->next;
		}//while
		//возможно, в программе произошло присвоение неинициализированному указателю
		return false;
	}
	//нулевой ук. удалять не требуется, поэтому возвращаем false
	return false;
}


//////////////////////////////////
//пример объявления типа указатель

// VAR TRec = RECORD END;
struct TRec {
};

// VAR pA, pB : POINTER TO TRec;
static TRec* pA;
static TRec* pB;


//-----------------------------------------------------------------------------
//данная процедура создается компилятором
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
//отображение списка указателей (DEBUG)
void O2M_CLEANER_DebugPrint(const char* comment)
{
	cout << comment << endl;
	if (!O2M_CLEANER_ROOT) {
		cout << "\tList empty" << endl;
		return;
	}
	int i = 1;
	for (CPointerInfo* pi = O2M_CLEANER_ROOT; pi; pi = pi->next, i++)
		cout << "\t" << "ь" << i << ": (" << pi->p << ", " << pi->count << ")" << endl;
}


//-----------------------------------------------------------------------------
//проверка работы через процедуру
//PROCEDURE TestProc(p : POINTER TO TRec);
void TestProc(TRec* p)
{
	//для не VAR параметров подразумевается неявное присваивание значения формального
	//параметра => требуется увеличение счетчика ссылок
	O2M_CLEANER_INC(p);

	//VAR pC : POINTER TO TRec;
	TRec* pC = NULL;

	//NEW(pC);
	O2M_CLEAN_TRec(pC);
	pC = new(TRec);
	O2M_CLEANER_INC(pC);

	//содержимое списка
	O2M_CLEANER_DebugPrint("TestProc(pA): NEW(pC);");

	//при выходе из области видимости необходимо очищать локальные переменные
	O2M_CLEAN_TRec(pC);

	//при выходе из области видимости необходимо уменьшать счетчик ссылок
	//(только для не VAR параметров)
	O2M_CLEAN_TRec(p);
}


//-----------------------------------------------------------------------------
//тестирование механизма сборщика мусора
int main()
{
	//NEW(pA);
	O2M_CLEAN_TRec(pA);
	pA = new(TRec);
	O2M_CLEANER_INC(pA);

	//содержимое списка
	O2M_CLEANER_DebugPrint("NEW(pA);");

	TestProc(pA);

	//содержимое списка
	O2M_CLEANER_DebugPrint("TestProc(pA); (* after call *)");

	//pB := pA;
	{
		void* O2M_TMP_POINTER = pB;
		pB = pA;
		O2M_CLEANER_INC(pA);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//содержимое списка
	O2M_CLEANER_DebugPrint("pB := pA;");

	//pA := pB;
	{
		void* O2M_TMP_POINTER = pA;
		pA = pB;
		O2M_CLEANER_INC(pB);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//содержимое списка
	O2M_CLEANER_DebugPrint("pA := pB;");

	//NEW(pB);
	O2M_CLEAN_TRec(pB);
	pB = new(TRec);
	O2M_CLEANER_INC(pB);

	//содержимое списка
	O2M_CLEANER_DebugPrint("NEW(pB);");

	//pA := NIL;
	{
		void* O2M_TMP_POINTER = pA;
		pA = 0;
		O2M_CLEANER_INC(0);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//содержимое списка
	O2M_CLEANER_DebugPrint("pA := NIL;");

	//pB := NIL;
	{
		void* O2M_TMP_POINTER = pB;
		pB = 0;
		O2M_CLEANER_INC(0);
		O2M_CLEAN_TRec(O2M_TMP_POINTER);
	}

	//содержимое списка
	O2M_CLEANER_DebugPrint("pB := NIL;");

	return 0;
}