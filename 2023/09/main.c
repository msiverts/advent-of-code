#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

int resolveLine(int lineNums[], int numEntries, bool prependMode) {
	int numDiffs = numEntries - 1;
	int diffs[numDiffs];
	bool allZeroes = true;
	for (int i = 0; i < numDiffs; i++) {
		int diff = lineNums[i+1] - lineNums[i];	
		if (diff != 0) {
			allZeroes = false;
		}
		diffs[i] = diff;
	}

	if (allZeroes) {
		if (prependMode) {
			return lineNums[0];
		} else {
			return lineNums[numEntries-1];
		}
	} else {
		int nextDiff = resolveLine(diffs, numDiffs, prependMode); 

		if (prependMode) {
			return lineNums[0] - nextDiff;
		} else {
			return lineNums[numEntries-1] + nextDiff; 
		}
	}
}

int main(int argc, char *argv[]) {
	FILE *fp = fopen("input.txt", "r");
	if (fp == NULL) {
		fprintf(stderr, "File input.txt not found\n");
		return -1;
	}

	char *line = (char *)malloc(1024);
	memset(line, 0, 1024);
	size_t lineCap = 1024;

	long sum = 0;
	long partTwoSum = 0;
	while (getline(&line, &lineCap, fp) > 0) {
		int lineNums[255];
		memset(lineNums, 0, sizeof(lineNums));

		int i = 0;
		char *val = strtok(line, " "); 
		while (val != NULL) {
			lineNums[i++] = atoi(val);
			val = strtok(NULL, " ");
		}

		sum += resolveLine(lineNums, i, false);	
		partTwoSum += resolveLine(lineNums, i, true);
	} 

	printf("Part 1 = %ld\n", sum);
	printf("Part 2 = %ld\n", partTwoSum);

	fclose(fp);
	free(line);

	return 0;
}
