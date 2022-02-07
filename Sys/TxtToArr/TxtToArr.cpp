//===============================================================
// ������������ ����� ���������� ����� � ������ ����� C++
//===============================================================

#include "stdio.h"

int main(int argc, char *argv[])
{
	//�������� ������� ����������
	if (argc <= 1) {
		printf("Usage: TxtToArr SrcFileName [ArrName] [ResFileName]\n");
		return 0;
	}

	//�������� ��������� ����� ��� ������
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		printf("Cannot open file \"%s\" for reading\n", argv[1]);
		return 0;
	}

	//����� �������� ������� (�������� 3-�� ���������)
	char *arr_name;
	if (argc >= 3) arr_name = argv[2];
	else arr_name = "arr_name";

	//����� ����� ��������� ����� (�������� 2-�� ���������)
	char *res_file_name;
	if (argc >= 4) res_file_name = argv[3];
	else res_file_name = "result.txt";

	//�������� ��������� ����� ��� ������
	FILE *ff = fopen(res_file_name, "w");
	if (!ff) {
		printf("Cannot open file \"%s\" for writing\n", res_file_name);
		fclose(f);
		return 0;
	}

	//������������� ������ � ����������
	const int b_size = 10000;
	const int bb_size = b_size * 2;
	char b[b_size];
	char bb[bb_size];
	int ii = 0;
	int b_count = 1;	//������ ���� �� ������� ���� ���� ������

	//������ ������ ������� + ����. ������� ������ ������
	fprintf (ff, "static char *%s[%s_size] =\n{\n\t\"", arr_name, arr_name);

	//���� ��������� �������� ����� � �����
	while (!feof(f)) {

		//�������� ���������� ����� � �����
		int curr_size = fread(&b, 1, b_size, f);

		//���� ��������� �������� ������
		for (int i = 0; i<curr_size; i++)
			switch (b[i]) {
			case '\n':	//�������� ����� ��������� ������
				bb[ii] = '\0';
				//������ ����� ������� ������ � �������� ����. ������
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
			default:	//����������� �������� ������� � ���. �����
				bb[ii++] = b[i];
			}

		//������ ������� ������ �� ����� ������
		bb[ii] = '\0';
		fprintf(ff, "%s", bb);
		ii = 0;

	}//while

	//������ ����. ������� ��������� ������ + ����� �������
	fprintf(ff, "\"\n};");

	//������ ������� ����������� ������� �����
	fprintf(ff, "\n%s_size = %i;\n", arr_name, b_count);

	fclose(f);
	fclose(ff);

	return 0;
}