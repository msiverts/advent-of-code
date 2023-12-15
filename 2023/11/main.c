#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

typedef struct {
	long x; 
	long y;
} loc_t;

loc_t *makeLocation(long x, long y) {
	loc_t *result = malloc(sizeof(loc_t));
	result->x = x;
	result->y = y;

	return result;
}

int compLocX (const void * elem1, const void * elem2)
{
    loc_t *f = *((loc_t **)elem1);
    loc_t *s = *((loc_t **)elem2);
    if (f->x > s->x) return  1;
    if (f->x < s->x) return -1;
    return 0;
}

long measureGalaxies(char *filename, long galaxyExpansionFactor) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "File input.txt not found\n");
		return -1;
	}

	loc_t *galaxyLocs[1024];
	memset(galaxyLocs, 0, sizeof(1024 * sizeof(loc_t *)));

	int numGalaxies = 0;
	char ch = 0;
	long x = 0;
	long y = 0;
	loc_t *lastGalaxy;
	long maxX = 0;

	while ((ch = fgetc(fp)) != EOF) {
		if (ch == '#') {
			long yDelta = 0; 
			if (numGalaxies > 0) {
				yDelta = y - galaxyLocs[numGalaxies-1]->y;
			}
			if (yDelta > 1) {
				y += ((yDelta - 1) * galaxyExpansionFactor);
			}
			lastGalaxy = makeLocation(x, y);
			galaxyLocs[numGalaxies++] = lastGalaxy;
		}
		if (ch == '\n') {
			x = 0;
			y++;
		} else {
			if (x > maxX) {
				maxX = x;
			}
			x++;
		}
	}

	qsort(galaxyLocs, numGalaxies, sizeof(loc_t *), compLocX);

	long lastX = 0;
	long runningDelta = 0;
	for (int i = 0; i < numGalaxies; i++) {
		long deltaX = 0;	
		if (i > 0) {
			deltaX = galaxyLocs[i]->x - lastX;
			if (deltaX > 1) {
				runningDelta += ((deltaX - 1) * galaxyExpansionFactor);
			}
		}
		lastX = galaxyLocs[i]->x;
		galaxyLocs[i]->x += runningDelta;
	}

	long distanceSum = 0;
	for (int galaxy = 0; galaxy < numGalaxies-1; galaxy++) {
		for (int nextGalaxy = galaxy+1; nextGalaxy < numGalaxies; nextGalaxy++) {
			long distance = labs(galaxyLocs[galaxy]->x - galaxyLocs[nextGalaxy]->x) + labs(galaxyLocs[galaxy]->y - galaxyLocs[nextGalaxy]->y);
			distanceSum += distance;
		}
	}

	fclose(fp);	
	for (int i = 0; i < numGalaxies; i++) {
		free(galaxyLocs[i]);
	}

	return distanceSum;
}

int main(int argc, char *argv[]) {
	char *filename = "input.txt";
	if (argc == 2) {
		filename = argv[1];
	}

	printf("Part one answer = %ld\n", measureGalaxies(filename, 1));
	printf("Part two answer = %ld\n", measureGalaxies(filename, 999999));
}
