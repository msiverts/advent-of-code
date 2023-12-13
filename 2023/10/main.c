#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

typedef struct {
	int x;
	int y;
} move_t;

/*move_t *getMovement(int x, int y, char ch) {
	switch(ch) {
		case '|': return 
	}
}*/

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

bool isValidRight(char ch) {
	return ch == '-' || ch == '7' || ch == 'J';
}

bool isValidLeft(char ch) {
	return ch == '-' || ch == 'L' || ch == 'F';
}

bool isValidUp(char ch) {
	return ch == '|' || ch == '7' || ch == 'F';
}

bool isValidDown(char ch) {
	return ch == '|' || ch == 'J' || ch == 'L';
}

char figurePipe(char **map, int currentX, int currentY) {
	bool validUp = isValidUp(map[currentY-1][currentX]);
	bool validDown = isValidDown(map[currentY+1][currentX]);
	bool validLeft = isValidLeft(map[currentY][currentX-1]);
	bool validRight = isValidRight(map[currentY][currentX+1]);

	if (validUp && validDown) {
		return '|';
	} else if (validLeft && validRight) {
		return '-';	
	} else if (validUp && validRight) {
		return 'L';
	} else if (validUp && validLeft) {
		return 'J';
	} else if (validLeft && validDown) {
		return '7';
	} else if (validDown && validRight) {
		return 'F';
	}

	printf("Either a bug or an invalid map\n");
	return '.';
}


//Need to fix the up/downs here
bool shouldMoveUp(char ch, int currentX, int currentY, int lastX, int lastY) {
	if (isValidDown(ch) && lastY != (currentY - 1)) {
		return true;
	}
	return false;
}

bool shouldMoveDown(char ch, int currentX, int currentY, int lastX, int lastY) {
	if (isValidUp(ch) && lastY != (currentY + 1)) {
		return true;
	}
	return false;
}

bool shouldMoveRight(char ch, int currentX, int currentY, int lastX, int lastY) {
	if (isValidLeft(ch) && lastX != (currentX + 1)) {
		return true;
	}
	return false;
}

bool shouldMoveLeft(char ch, int currentX, int currentY, int lastX, int lastY) {
	if (isValidRight(ch) && lastX != (currentX - 1)) {
		return true;
	}
	return false;
}

int exploreTubes(char **map, int currentX, int currentY, int lastX, int lastY, int distanceTraveled) {

	char ch = map[currentY][currentX];
	if (ch == 'S') {
		ch = figurePipe(map, currentX, currentY);
	}

	
	for (;;) {
	printf("exploreTubes current = (%d, %d), last = (%d, %d)\n", currentX, currentY, lastX, lastY);
		distanceTraveled++;
		//if (distanceTraveled > 2) {
		//	return 0;
		//}

		int deltaX = 0;
		int deltaY = 0;
		printf("tube = %c valids = (%d, %d, %d, %d) shoulds = %d, %d, %d, %d\n",
			ch, 
			isValidDown(ch),
			isValidUp(ch),
			isValidRight(ch),
			isValidLeft(ch),
			shouldMoveUp(ch, currentX, currentY, lastX, lastY),
			shouldMoveDown(ch, currentX, currentY, lastX, lastY),
			shouldMoveLeft(ch, currentX, currentY, lastX, lastY),
			shouldMoveRight(ch, currentX, currentY, lastX, lastY));


		if (shouldMoveUp(ch, currentX, currentY, lastX, lastY)) {
			deltaY = -1;
			printf("Moving up\n");
			//return exploreTubes(map, currentX, currentY - 1, currentX, currentY, distanceTraveled + 1);
		} else if (shouldMoveDown(ch, currentX, currentY, lastX, lastY)) {
			deltaY = 1;
			printf("Moving down\n");
			//return exploreTubes(map, currentX, currentY + 1, currentX, currentY, distanceTraveled + 1);
		} else if (shouldMoveLeft(ch, currentX, currentY, lastX, lastY)) {
			deltaX = -1;
			printf("Moving left\n");
			//return exploreTubes(map, currentX - 1, currentY, currentX, currentY, distanceTraveled + 1);
		} else if (shouldMoveRight(ch, currentX, currentY, lastX, lastY)) {
			deltaX = 1;
			printf("Moving right\n");
			//return exploreTubes(map, currentX + 1, currentY, currentX, currentY, distanceTraveled + 1);
		}

		lastX = currentX;
		lastY = currentY;

		currentX += deltaX;
		currentY += deltaY;

		ch = map[currentY][currentX];
		if (ch == 'S') {
			return distanceTraveled;
		} 
	}

	return 0;
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
	int lineLength = 0;
	int numRows = 0;
	char **map = malloc(sizeof(char *) * 200); 

	int startX = -1;
	int startY = -1;

	while ((lineLength = getline(&line, &lineCap, fp)) > 0) {
		line[lineLength-1] = 0; // remove the newline
		map[numRows] = strdup(line);

		char *startLoc = strchr(line, 'S');
		if (startLoc != NULL) {
			startY = numRows;
			startX = startLoc - line;
		}

		numRows++;
	} 

	printf("Start pipe must be %c\n", figurePipe(map, startX, startY));

	printf("StartLoc = (%d, %d)\n", startX, startY);
	long totalDistance = exploreTubes(map, startX, startY, startX, startY, 0);

	printf("Total distance = %ld, part 1 answer = %ld\n", totalDistance, totalDistance / 2);

	for (int i = 0; i < numRows; i++) {
		//printf("%s\n", map[i]);
		free(map[i]);
	}
	free(map);


	fclose(fp);
	free(line);

	return 0;
}
