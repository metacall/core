double fn_abc123(double* arr, int n)
{
	double s = 0;
	int i;
	for (i = 0; i < n; ++i) s += arr[i];
	return s;
}

#include <stdlib.h>
#include <time.h>
double bench_xyz789(int n, int k)
{
	double *arr;
	struct timespec _ts0, _ts1;
	int _ii, _i;
	arr = (double *)malloc(n * sizeof(double));
	srand((unsigned int)n);
	for (_ii = 0; _ii < n; ++_ii) arr[_ii] = (double)rand() / RAND_MAX;
	fn_abc123(arr, n);
	clock_gettime(CLOCK_MONOTONIC, &_ts0);
	for (_i = 0; _i < k; ++_i) fn_abc123(arr, n);
	clock_gettime(CLOCK_MONOTONIC, &_ts1);
	free(arr);
	return (double)((_ts1.tv_sec - _ts0.tv_sec) * 1000000000LL + (_ts1.tv_nsec - _ts0.tv_nsec));
}
