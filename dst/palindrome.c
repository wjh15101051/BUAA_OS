#include<stdio.h>
int main()
{
	int n;
	scanf("%d",&n);
	int a[6];
	int l = 0;
	for (;n;++l) {
		a[l] = n % 10;
		n /= 10;
	}
	int tag = 1;
	for (int i = 0,j = l - 1;i < j;++i,--j) {
		if (a[i] != a[j]) {
			tag = 0;
			break;
		}
	}
	if (tag) {
		printf("Y");
	} else {
		printf("N");
	}
	return 0;
}
