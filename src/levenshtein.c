#include <string.h>
#include "levenshtein.h"

int distance(char* a, char* b) {
	size_t maxi = strlen(b);
	size_t maxj = strlen(a);
	int i, j;

	unsigned int compare[maxi+1][maxj+1];

	compare[0][0] = 0;
	for (i = 1; i <= maxi; i++) compare[i][0] = i;
	for (j = 1; j <= maxj; j++) compare[0][j] = j;

	for (i = 1; i <= maxi; i++) {
		for (j = 1; j <= maxj; j++) {
			int left   = compare[i-1][j] + 1;
			int right  = compare[i][j-1] + 1;
			int middle = compare[i-1][j-1] + (a[j-1] == b[i-1] ? 0 : 1);

			if( left < right && left < middle )       compare[i][j] = left;
			else if( right < left && right < middle ) compare[i][j] = right;
			else                                      compare[i][j] = middle;
		}
	}
 
	return compare[maxi][maxj];
}

