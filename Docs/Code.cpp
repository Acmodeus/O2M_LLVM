#include "iostream.h"

/* TYPE TArr : ARRAY */

/* VAR arr : TArr */
const int O2M_ARR_0_TArr = 5;
const int O2M_ARR_1_TArr = 10;
int *arr[O2M_ARR_0_TArr];

/* VAR sm_arr : ARRAY */
const int O2M_ARR_0_sm_arr = 2;
const int O2M_ARR_1_sm_arr = 5;
int *sm_arr[O2M_ARR_0_sm_arr];

/* initialisation of arrays */
struct O2M_ARR_INIT_DECL_testModule {
	O2M_ARR_INIT_DECL_testModule() {
		int i;
		for (i = 0; i < O2M_ARR_0_TArr; i++)
			arr[i] = new int[O2M_ARR_1_TArr];
		for (i = 0; i < O2M_ARR_0_sm_arr; i++)
			sm_arr[i] = new int[O2M_ARR_1_sm_arr];
	};
	~O2M_ARR_INIT_DECL_testModule() {
		int i;
		for (i = 0; i < O2M_ARR_0_TArr; i++)
			delete[] arr[i];
		for (i = 0; i < O2M_ARR_0_sm_arr; i++)
			delete[] sm_arr[i];
	};
} O2M_ARR_INIT_testModule;


typedef int TI;


void proc(const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI **array)
{
	for(int i = 0; i < O2M_ARR_0_array; i++)
		for(int j = 0; j < O2M_ARR_1_array; j++)
			array[i][j] = i + j;
}


void writeArray(const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI **array)
{
	for(int i = 0; i < O2M_ARR_0_array; i++) {
		for(int j = 0; j < O2M_ARR_1_array; j++)
			cout << ((array[i][j] >= 10) ? "  " : "   ") << array[i][j];
		cout << endl;
	}
}

const int O2M_ARR_0_a = 10;
const int O2M_ARR_1_a = 20;
TI a[O2M_ARR_0_a][O2M_ARR_1_a];

const int O2M_ARR_0_b = 5;
const int O2M_ARR_1_b = 10;
TI b[O2M_ARR_0_b][O2M_ARR_1_b];


void tst(int x, int y, const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI *O2M_ARR_array)
{
	TI *array = new TI[O2M_ARR_0_array * O2M_ARR_1_array];

	struct TT {
		TI *array;
		TT(int sz, TI *arr) {
			array = new TI[sz];
			arr = array;
		};
		~TT() {
			delete[] array;
		};
	} tt(O2M_ARR_0_array * O2M_ARR_1_array, array);

	O2M_ARR_array[x * O2M_ARR_1_array + y] = 10;
}


void main ()
{
	tst(0, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);
	tst(1, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);
	tst(2, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);

	for (int i = 0; i < O2M_ARR_0_b; i++) {
		for(int j = 0; j < O2M_ARR_1_b; j++)
			cout << ((b[i][j] < 10) ? "   " : "  ") << b[i][j];
		cout << endl;
	}

	return;

	proc(O2M_ARR_0_a, O2M_ARR_1_a, (TI **)a);
	proc(O2M_ARR_0_b, O2M_ARR_1_b, (TI **)b);

	cout << "----- a -----" << endl;
	writeArray(O2M_ARR_0_a, O2M_ARR_1_a, (TI **)a);
	cout << "----- b -----" << endl;
	writeArray(O2M_ARR_0_b, O2M_ARR_1_b, (TI **)b);

	return;
}