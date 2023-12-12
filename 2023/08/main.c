#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

/*******************Building a stupid map********************/

typedef struct {
	char *key;
	void *value;
} map_entry_t;

typedef struct {
	int numBuckets;
	int initSlotsPerBucket;
	map_entry_t ***buckets;
} map_t;

int hash(const char *key) {
	long val = 0;
	int i = 0;

	while (key[i] != 0) {
		val += key[i];
		i++;
	}

	return val; 
}

map_t *map_create(int numBuckets) {
	map_t *val = malloc(sizeof(map_t));
	memset(val, 0, sizeof(map_t));
	val->numBuckets = numBuckets;
	val->buckets = malloc(sizeof(map_entry_t **) * numBuckets);
	memset(val->buckets, 0, sizeof(map_entry_t **) * numBuckets);
	//doesn't handle realloc now, but can add later
	val->initSlotsPerBucket = 64;

	return val;
}

map_entry_t *map_get(map_t *map, const char *key, bool createIfMissing) {
	map_entry_t *entry = NULL;
	int bucket = hash(key) % map->numBuckets;	

	if (map->buckets[bucket] == NULL) {
		int bucketSize = sizeof(map_entry_t **) * map->initSlotsPerBucket;
		map->buckets[bucket] = malloc(bucketSize);
		memset(map->buckets[bucket], 0, bucketSize);
	}

	int i = 0;
	while (map->buckets[bucket][i] != NULL) {
		if (strcmp(map->buckets[bucket][i]->key, key) == 0) {
			entry = map->buckets[bucket][i];	
			break;
		}
		i++;
	}

	if (entry == NULL && createIfMissing) {
		entry = (map_entry_t *)malloc(sizeof(map_entry_t));
		memset(entry, 0, sizeof(map_entry_t));
		entry->key = strdup(key);
		map->buckets[bucket][i] = entry;
	}

	return entry;
}

void map_put(map_t *map, char *key, void *value) {
	map_entry_t *entry = map_get(map, key, true);
	entry->value = value;
}

void map_free(map_t *map, bool freeValues) {
	for (int i = 0; i < map->numBuckets; i++) {
		map_entry_t **bucket = map->buckets[i];	
		int j = 0;
		while (bucket && bucket[j] != NULL) {
			map_entry_t *entry = bucket[j];
			if (entry != NULL) {
				if (freeValues) {
					free(entry->value);
				}
				free(entry->key);
				free(entry);
			}
			j++;
		}
		if (bucket != NULL) {
			free(bucket);
		}
	}

	free(map->buckets);
}

/*******************End building a stupid map********************/

bool startsWith(const char *start, const char *check) {
    return strncmp(start, check, strlen(start)) == 0;
}

typedef struct node_t node_t;

struct node_t {
	char id[4];
	node_t *left;
	node_t *right;
};

node_t *ensureNode(map_t *map, const char *key, const char *left, const char *right) {
	map_entry_t *mapEntry = map_get(map, key, true);	
	node_t *node = NULL;
	if (mapEntry->value == NULL) {
		node = malloc(sizeof(node_t));
		strcpy(node->id, key);

		mapEntry->value = node;
	} else {
		node = (node_t *)mapEntry->value;
	}

	if (left != NULL) {
		node->left = ensureNode(map, left, NULL, NULL);
	}
	if (right != NULL) {
		node->right = ensureNode(map, right, NULL, NULL);
	}

	return node;
}

uintmax_t gcd(uintmax_t a, uintmax_t b) {
	while (b > 0) {
		uintmax_t tmp = b;
		b = a % b;
		a = tmp;
	}
	return a;
}

uintmax_t lcm (uintmax_t a, uintmax_t b) {
	return a * (b / gcd(a, b));
}


uintmax_t leastCommonMultiple(long *stepsList, int numNodes) {
	uintmax_t result = stepsList[0];
	for (int i = 1; i < numNodes; i++) {
		result = lcm(result, stepsList[i]);
	}
	return result;
}

int main(int argc, char *argv[]) {
	FILE *fp = fopen("input.txt", "r");
	if (fp == NULL) {
		fprintf(stderr, "File input.txt not found\n");
		return -1;
	}

	map_t *nodeMap = map_create(255);

	char *line = (char *)malloc(1024);
	memset(line, 0, 1024);
	size_t lineCap = 1024;

	char *instructions = NULL;
	int aNodeCount = 0;
	node_t *aNodes[64];
	memset(aNodes, 0, 64 * sizeof(char *));

	while (getline(&line, &lineCap, fp) > 0) {
		if (instructions == NULL) {
			//take off the new line
			instructions = strndup(line, strlen(line)-1);
		} else if (strlen(line) > 1) {
			const char *nodeId = strtok(line, " =,()");		
			const char *leftNode = strtok(NULL, " =,()");		
			const char *rightNode = strtok(NULL, " =,()");		

			node_t *node = ensureNode(nodeMap, nodeId, leftNode, rightNode);	
			if (node->id[2] == 'A') {
				aNodes[aNodeCount++] = node;
			}

		}
	}

	printf("Instructions: (%s)\n", instructions);

	node_t *node = map_get(nodeMap, "AAA", false)->value;
	int i = 0;
	long partOneSteps = 1;
	long steps = 1;
	long stepsToFirstZ[32];
	memset(stepsToFirstZ, 0, sizeof(stepsToFirstZ));

	for (;;) {
		if (instructions[i] == 0) {
			i = 0;
		}
		char ch = instructions[i];
		if (ch == 'L') {
			node = node->left;
			for (int nodeIndex = 0; nodeIndex < aNodeCount; nodeIndex++) {
				if (aNodes[nodeIndex]->id[2] != 'Z') {
					aNodes[nodeIndex] = aNodes[nodeIndex]->left;
				} else if (stepsToFirstZ[nodeIndex] == 0) {
					stepsToFirstZ[nodeIndex] = steps - 1;
				}
			}
		} else if (ch == 'R') {
			node = node->right;	
			for (int nodeIndex = 0; nodeIndex < aNodeCount; nodeIndex++) {
				if (aNodes[nodeIndex]->id[2] != 'Z') {
					aNodes[nodeIndex] = aNodes[nodeIndex]->right;
				} else if (stepsToFirstZ[nodeIndex] == 0) {
					stepsToFirstZ[nodeIndex] = steps - 1;
				}
			}
		} else {
			printf("Bad instruction %c\n", ch);
			exit(1);
		}

		if (strcmp(node->id, "ZZZ") == 0) {
			partOneSteps = steps;
		}

		int numNotAtZ = 0;
		for (int nodeIndex = 0; nodeIndex < aNodeCount; nodeIndex++) {
			if (stepsToFirstZ[nodeIndex] == 0) {
				numNotAtZ++;
			} 
		}

		if (numNotAtZ == 0) {
			break;
		}

		i++;
		steps++;
	}

	uintmax_t partTwoSteps = leastCommonMultiple(stepsToFirstZ, aNodeCount);

	printf("Steps for part 1 = %ld\n", partOneSteps);
	printf("Steps for part 2 = %lu\n", partTwoSteps);


	map_free(nodeMap, false);
	free(instructions);

	fclose(fp);
	free(line);

	return 0;
}
