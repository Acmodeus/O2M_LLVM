#include "iostream.h"

/* TYPE TArr : ARRAY */


typedef int TI;


const int O2M_ARR_0_a = 5;
const int O2M_ARR_1_a = 15;
TI a[O2M_ARR_0_a][O2M_ARR_1_a];

const int O2M_ARR_0_b = 5;
const int O2M_ARR_1_b = 10;
TI b[O2M_ARR_0_b][O2M_ARR_1_b];

const int O2M_ARR_0_c = 2;
const int O2M_ARR_1_c = 3;
const int O2M_ARR_2_c = 4;
TI c[O2M_ARR_0_c][O2M_ARR_1_c][O2M_ARR_2_c];


void writeArray2(const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI *array)
{
	for(int i = 0; i < O2M_ARR_0_array; i++) {
		for(int j = 0; j < O2M_ARR_1_array; j++)
			cout << ((array[i+j*O2M_ARR_0_array] >= 10) ? "  " : "   ") << array[i+j*O2M_ARR_0_array];
		cout << endl;
	}
}


void writeArray3(const int O2M_ARR_0_array, const int O2M_ARR_1_array, const int O2M_ARR_2_array, TI *array)
{
	for(int i = 0; i < O2M_ARR_0_array; i++) {
		for(int j = 0; j < O2M_ARR_1_array; j++) {
			for(int k = 0; k < O2M_ARR_2_array; k++)
				cout << ((array[j+i*O2M_ARR_0_array+k*O2M_ARR_0_array*O2M_ARR_1_array] >= 10) ? "  " : "   ") << array[j+i*O2M_ARR_0_array+k*O2M_ARR_0_array*O2M_ARR_1_array] << ",";
			cout << "   ";
		}
			cout << endl;
	}
}


void proc2(const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI *array)
{
	for(int i = 0; i < O2M_ARR_0_array; i++)
		for(int j = 0; j < O2M_ARR_1_array; j++)
			array[i+j*O2M_ARR_0_array] = i + j;
}


void tst2(int x, int y, const int O2M_ARR_0_array, const int O2M_ARR_1_array, TI *O2M_ARR_array)
{
	TI *array = new TI[O2M_ARR_0_array * O2M_ARR_1_array];

	O2M_ARR_array[x * O2M_ARR_1_array + y] = 10;

	delete[] array;
}


void tst3(int x, int y, int z, const int O2M_ARR_0_array, const int O2M_ARR_1_array, const int O2M_ARR_2_array, TI *array)
{
	array[z * O2M_ARR_1_array * O2M_ARR_0_array + x * O2M_ARR_1_array + y] = 10;
}


void main ()
{
	tst2(0, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);
	tst2(1, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);
	tst2(2, 3, O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);

	cout << "----- b -----" << endl;
	writeArray2(O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);

	proc2(O2M_ARR_0_a, O2M_ARR_1_a, (TI *)a);
	proc2(O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);

	tst3(1, 0, 0, O2M_ARR_0_c, O2M_ARR_1_c, O2M_ARR_2_c, (TI *)c);

	cout << "----- a -----" << endl;
	writeArray2(O2M_ARR_0_a, O2M_ARR_1_a, (TI *)a);
	cout << "----- b -----" << endl;
	writeArray2(O2M_ARR_0_b, O2M_ARR_1_b, (TI *)b);
	cout << "----- c -----" << endl;
	writeArray3(O2M_ARR_0_c, O2M_ARR_1_c, O2M_ARR_2_c, (TI *)c);

	return;
}