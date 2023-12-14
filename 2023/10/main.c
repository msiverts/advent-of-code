#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

void floodFill(char **mapStencil, int mapWidth, int mapHeight, int x, int y);

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

//Infer the pipe that connects adjacent pipes if possible
char figurePipe(char **map, int currentX, int currentY, int mapWidth, int mapHeight) {
	bool validUp = currentY > 0 && isValidUp(map[currentY-1][currentX]);
	bool validDown = currentY < (mapHeight-1) && isValidDown(map[currentY+1][currentX]);
	bool validLeft = currentX > 0 && isValidLeft(map[currentY][currentX-1]);
	bool validRight = currentX < (mapWidth-1) && isValidRight(map[currentY][currentX+1]);

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

	return map[currentY][currentX];
}

//isValid calls seem reversed here, but it's right. It's because they're looking at adjacent spots
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

//The stencil should be double the size of the map, we'll fill the pipes on the stencil on even rows/columns
//Everything else will get marked as '.'
int exploreTubes(char **map, char **mapStencil, int currentX, int currentY, int mapWidth, int mapHeight) {
	int lastX = currentX;
	int lastY = currentY;
	int distanceTraveled = 0;

	char ch = map[currentY][currentX];
	if (ch == 'S') {
		ch = figurePipe(map, currentX, currentY, mapWidth, mapHeight);
	}
	
	while (ch != 'S') {
		mapStencil[currentY*2][currentX*2] = ch;	
		distanceTraveled++;

		int deltaX = 0;
		int deltaY = 0;

		if (shouldMoveUp(ch, currentX, currentY, lastX, lastY)) {
			deltaY = -1;
		} else if (shouldMoveDown(ch, currentX, currentY, lastX, lastY)) {
			deltaY = 1;
		} else if (shouldMoveLeft(ch, currentX, currentY, lastX, lastY)) {
			deltaX = -1;
		} else if (shouldMoveRight(ch, currentX, currentY, lastX, lastY)) {
			deltaX = 1;
		}

		lastX = currentX;
		lastY = currentY;

		currentX += deltaX;
		currentY += deltaY;

		ch = map[currentY][currentX];
	}

	return distanceTraveled;
}

//if it's a valid location and it hasn't been visited, as indicated by the .
bool canMoveTo(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	return x >= 0 && y >= 0 && x < mapWidth-1 && y < mapHeight && mapStencil[y][x] == '.';  
}

void maybeFloodFill(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (canMoveTo(mapStencil, mapWidth, mapHeight, x, y)) {
		floodFill(mapStencil, mapWidth, mapHeight, x, y);
	}
}

//floodFill from the given location, marking as 0 along the way
void floodFill(char **mapStencil, int mapWidth, int mapHeight, int x, int y) {
	if (mapStencil[y][x] == '.') {
		mapStencil[y][x] = '0';
	} 

	maybeFloodFill(mapStencil, mapWidth, mapHeight, x-1, y);
	maybeFloodFill(mapStencil, mapWidth, mapHeight, x+1, y);
	maybeFloodFill(mapStencil, mapWidth, mapHeight, x, y-1);
	maybeFloodFill(mapStencil, mapWidth, mapHeight, x, y+1);
}

//floodFill all around the edges, and anything left as a . is enclosed
void findEnclosures(char **mapStencil, int mapWidth, int mapHeight) {
	for (int y = 0; y < mapHeight; y++ )
	{
		if (y == 0 || y == mapHeight-1) {
			for (int x = 0; x < mapWidth-1; x++) {
				if (mapStencil[y][x] == '.') {
					floodFill(mapStencil, mapWidth, mapHeight, x, y);
				}
			}
			
		} else {
			if (mapStencil[y][0] == '.') {
				floodFill(mapStencil, mapWidth, mapHeight, 0, y);
			}
			if (mapStencil[y][mapWidth] == '.') {
				floodFill(mapStencil, mapWidth, mapHeight, mapWidth, y);
			}
		}
	}
}

//Since we expanded the map by 2, need to infer the missing pipe pieces
void inferStencilGaps(char **mapStencil, int stencilWidth, int stencilHeight) {
	for (int y = 0; y < stencilHeight; y++) {
		for (int x = 0; x < stencilWidth; x++) 	{
			mapStencil[y][x] = figurePipe(mapStencil, x, y, stencilWidth, stencilHeight);			
		}
	}
}

int countDotsInEvens(char **stencilMap, int maxWidth, int maxHeight) {
	int numDots = 0;
	for (int y = 0; y < maxHeight; y+=2) {
		for (int x = 0; x < maxWidth; x+=2) {
			if (stencilMap[y][x] == '.') {
				numDots++;
			}
		}
	}
	return numDots;
}

int main(int argc, char *argv[]) {
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

	int stencilWidth = (rowWidth - 1) * 2;
	int stencilHeight = numRows * 2;
	char **mapStencil = (char **)malloc(sizeof(char *) * stencilWidth);
	for (int i = 0; i < stencilHeight; i++) {
		mapStencil[i] = (char *)malloc(stencilWidth);
		memset(mapStencil[i], '.', stencilWidth);
		mapStencil[stencilWidth] = 0;
	}

	long totalDistance = exploreTubes(map, mapStencil, startX, startY, rowWidth, numRows);

	inferStencilGaps(mapStencil, stencilWidth, stencilHeight); 
	findEnclosures(mapStencil, stencilWidth, stencilHeight);

	for (int y = 0; y < stencilHeight; y+=2) {
		for (int x = 0; x < stencilWidth; x+= 2) {
			printf("%c", mapStencil[y][x]);
		}
		printf("\n");
	}

	printf("Total distance = %ld, part 1 answer = %ld\n", totalDistance, totalDistance / 2);
	printf("Part 2 answer numDots = %d\n", countDotsInEvens(mapStencil, stencilWidth, stencilHeight));

	for (int i = 0; i < numRows; i++) {
		free(map[i]);
		free(mapStencil[i]);
	}
	free(map);


	fclose(fp);
	free(line);

	return 0;
}
