//===============================================================
// Конвертирует текст указанного файла в массив строк C++
//===============================================================

#include "stdio.h"

int main(int argc, char *argv[])
{
	//проверка наличия аргументов
	if (argc <= 1) {
		printf("Usage: TxtToArr SrcFileName [ArrName] [ResFileName]\n");
		return 0;
	}

	//открытие исходного файла для чтения
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		printf("Cannot open file \"%s\" for reading\n", argv[1]);
		return 0;
	}

	//выбор названия массива (проверка 3-го аргумента)
	char *arr_name;
	if (argc >= 3) arr_name = argv[2];
	else arr_name = "arr_name";

	//выбор имени конечного файла (проверка 2-го аргумента)
	char *res_file_name;
	if (argc >= 4) res_file_name = argv[3];
	else res_file_name = "result.txt";

	//открытие конечного файла для записи
	FILE *ff = fopen(res_file_name, "w");
	if (!ff) {
		printf("Cannot open file \"%s\" for writing\n", res_file_name);
		fclose(f);
		return 0;
	}

	//промежуточные буфера и переменные
	const int b_size = 10000;
	const int bb_size = b_size * 2;
	char b[b_size];
	char bb[bb_size];
	int ii = 0;
	int b_count = 1;	//должна быть по крайней мере одна строка

	//запись начала массива + откр. кавычку первой строки
	fprintf (ff, "static char *%s[%s_size] =\n{\n\t\"", arr_name, arr_name);

	//цикл поблочной загрузки файла в буфер
	while (!feof(f)) {

		//загрузка очередного блока в буфер
		int curr_size = fread(&b, 1, b_size, f);

		//цикл обработки текущего буфера
		for (int i = 0; i<curr_size; i++)
			switch (b[i]) {
			case '\n':	//проверка конца очередной строки
				bb[ii] = '\0';
				//запись конца текущей строки и отступов след. строки
				fprintf(ff, "%s\",\n\t\"", bb);
				ii = 0;
				b_count++;
				continue;
			case '\t':
				bb[ii++] = '\\';
				bb[ii++] = 't';
				continue;
			case '"':
			case '\\':
				bb[ii++] = '\\';
			default:	//копирование текущего символа в вых. буфер
				bb[ii++] = b[i];
			}

		//запись остатка строки до конца буфера
		bb[ii] = '\0';
		fprintf(ff, "%s", bb);
		ii = 0;

	}//while

	//запись закр. кавычки последней строки + конец массива
	fprintf(ff, "\"\n};");

	//запись размера полученного массива строк
	fprintf(ff, "\n%s_size = %i;\n", arr_name, b_count);

	fclose(f);
	fclose(ff);

	return 0;
}