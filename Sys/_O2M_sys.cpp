#include <math.h>

long MOD(const long x, const long y) {
	return x - long(floor(double(x) / y)) * y;
}

int ORD(const char ch) {
	return (ch < 0) ? (ch + 256) : ch;
}

void COPY(const char *x, const int x_size, char *v, const int v_size) {
	int min = x_size;
	if (v_size < x_size) min = v_size;
	for (int i = 0; i <= min-2; i++) v[i] = x[i];
	v[min-1] = 0;
}

int O2M_SET_RANGE(const int l, const int h) {
	int res = 0;
	for (int i = l; i <= h; i++) res |= 1 << i;
	return res;
}
