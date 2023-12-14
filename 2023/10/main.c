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

bool isPipe(char ch) {
	return ch == '-' || ch == '7' || ch == 'J' || ch == '|' || ch == 'F' || ch == 'L';
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

bool isValidVertical(char ch) {
	return isValidUp(ch) || isValidDown(ch);
}

bool isValidHorizontal(char ch) {
	return isValidLeft(ch) || isValidRight(ch);
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


/*     Doors
 *   JL can go up
 *   7F can go down
 *   L can go left
 *   F
 *
 *   J can go right
 *   7
 */


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

int exploreTubes(char **map, char **mapStencil, int currentX, int currentY) {
	int lastX = currentX;
	int lastY = currentY;
	int distanceTraveled = 0;

	char ch = map[currentY][currentX];
	if (ch == 'S') {
		ch = figurePipe(map, currentX, currentY);
		printf("S cell is a %c\n", ch);
	}

	
	while (ch != 'S') {
		//printf("exploreTubes current = (%d, %d), last = (%d, %d)\n", currentX, currentY, lastX, lastY);
		mapStencil[currentY][currentX] = ch;	
		distanceTraveled++;

		int deltaX = 0;
		int deltaY = 0;
		/*printf("tube = %c valids = (%d, %d, %d, %d) shoulds = %d, %d, %d, %d\n",
			ch, 
			isValidDown(ch),
			isValidUp(ch),
			isValidRight(ch),
			isValidLeft(ch),
			shouldMoveUp(ch, currentX, currentY, lastX, lastY),
			shouldMoveDown(ch, currentX, currentY, lastX, lastY),
			shouldMoveLeft(ch, currentX, currentY, lastX, lastY),
			shouldMoveRight(ch, currentX, currentY, lastX, lastY));
			*/


		if (shouldMoveUp(ch, currentX, currentY, lastX, lastY)) {
			deltaY = -1;
			//printf("Moving up\n");
		} else if (shouldMoveDown(ch, currentX, currentY, lastX, lastY)) {
			deltaY = 1;
			//printf("Moving down\n");
		} else if (shouldMoveLeft(ch, currentX, currentY, lastX, lastY)) {
			deltaX = -1;
			//printf("Moving left\n");
		} else if (shouldMoveRight(ch, currentX, currentY, lastX, lastY)) {
			deltaX = 1;
			//printf("Moving right\n");
		}

		lastX = currentX;
		lastY = currentY;

		currentX += deltaX;
		currentY += deltaY;

		ch = map[currentY][currentX];
	}

	return distanceTraveled;
}



bool canMoveUp(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (y > 0) {
		char ch = mapStencil[y-1][x];
		char nextCh = mapStencil[y-1][x+1];
		if (ch == '.') {
			return true;
		}
	}
	return false;
}

bool canMoveDown(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (y < mapHeight-1) {
		char ch = mapStencil[y+1][x];
		if (ch == '.') {
			return true;
		}
	}
	return false;
}

bool canMoveLeft(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (x > 0) {
		char ch = mapStencil[y][x-1];
		if (ch == '.') {
			return true;
		}
	}
	return false;
}

bool canMoveRight(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (x < mapWidth-1) {
		char ch = mapStencil[y][x+1];
		if (ch == '.') {
			return true;
		}
	}
	return false;
}

void floodFill(char **map, char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (mapStencil[y][x] == '.') {
		mapStencil[y][x] = '0';
	} else {
	//	mapStencil[y][x] = 'P';
	}
	if (canMoveLeft(mapStencil, mapWidth, mapHeight, x, y)) {
		floodFill(map, mapStencil, mapWidth, mapHeight, x-1, y);
	}
	if (canMoveRight(mapStencil, mapWidth, mapHeight, x, y)) {
		floodFill(map, mapStencil, mapWidth, mapHeight, x+1, y);
	}
	if (canMoveUp(mapStencil, mapWidth, mapHeight, x, y)) {
		if (mapStencil[y-1][x] == 'J' && mapStencil[y-1][x+1] == 'L') {
			printf("FOUND A DOOR!!!!\n");	
			y--;
			char left = mapStencil[y][x];
			char right = mapStencil[y][x+1];
			do {
				y--;
				left = mapStencil[y][x];
				right = mapStencil[y][x+1];
			} while (isValidVertical(left) || isValidVertical(right));

			if (!isPipe(left)) {
				floodFill(map, mapStencil, mapWidth, mapHeight, x, y);
			} else {
				floodFill(map, mapStencil, mapWidth, mapHeight, x+1, y);
			}
		}
		floodFill(map, mapStencil, mapWidth, mapHeight, x, y-1);
	}
	if (canMoveDown(mapStencil, mapWidth, mapHeight, x, y)) {
		floodFill(map, mapStencil, mapWidth, mapHeight, x, y+1);
	}
}

void findEnclosures(char **map, char **mapStencil, int mapWidth, int mapHeight) {
	//Go around the edges and floodfill
	for (int y = 0; y < mapHeight; y++ )
	{
		if (y == 0 || y == mapHeight-1) {
			for (int x = 0; x < mapWidth-1; x++) {
				if (mapStencil[y][x] == '.') {
					floodFill(map, mapStencil, mapWidth, mapHeight, x, y);
				}
			}
			
		} else {
			if (mapStencil[y][0] == '.') {
				floodFill(map, mapStencil, mapWidth, mapHeight, 0, y);
			}
			if (mapStencil[y][mapWidth] == '.') {
				floodFill(map, mapStencil, mapWidth, mapHeight, mapWidth, y);
			}
		}
	}
}

int countDots(char **stencilMap, int maxWidth, int maxHeight) {
	int numDots = 0;
	for (int y = 0; y < maxHeight; y++) {
		for (int x = 0; x < maxWidth; x++) {
			if (stencilMap[y][x] == '.') {
				numDots++;
			}
		}
	}
	return numDots;
}

int main(int argc, char *argv[]) {
	printf("argc = %d\n", argc);
	char *inputFile = "input.txt";
	if (argc == 2) {
		inputFile = argv[1];
	}	
	FILE *fp = fopen(inputFile, "r");
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

	int rowWidth = strlen(map[0]) + 1;
	char **mapStencil = (char **)malloc(sizeof(char *) * numRows);
	for (int i = 0; i < numRows; i++) {
		mapStencil[i] = (char *)malloc(rowWidth);
		memset(mapStencil[i], '.', rowWidth);
		mapStencil[rowWidth] = 0;
	}

	long totalDistance = exploreTubes(map, mapStencil, startX, startY);

	printf("Total distance = %ld, part 1 answer = %ld\n", totalDistance, totalDistance / 2);

	for (int i = 0; i < numRows; i++) {
		printf("%s\n", mapStencil[i]);
	}

	findEnclosures(map, mapStencil, rowWidth, numRows);

	printf("\n\n");
	for (int i = 0; i < numRows; i++) {
		printf("%s\n", mapStencil[i]);
	}

	printf("numDots = %d\n", countDots(mapStencil, rowWidth, numRows));

	for (int i = 0; i < numRows; i++) {
		free(map[i]);
		free(mapStencil[i]);
	}
	free(map);


	fclose(fp);
	free(line);

	return 0;
}
