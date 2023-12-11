#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

#define NUM_HANDS 1000
#define CARDS_PER_HAND 5

#define FIVE_OF_A_KIND    10
#define FOUR_OF_A_KIND     9
#define FULL_HOUSE         8
#define THREE_OF_A_KIND    7
#define TWO_PAIR           6
#define ONE_PAIR           5
#define HIGH_CARD          4

#define NO_JOKERS 0
#define JOKERS    1

typedef struct {
	char cards[CARDS_PER_HAND + 1];
	char sortedCards[CARDS_PER_HAND + 1];
	int bid;
} hand_t;

hand_t hands[NUM_HANDS];

int mode = NO_JOKERS;

bool startsWith(const char *start, const char *check) {
    return strncmp(start, check, strlen(start)) == 0;
}

int scoreHandNoTiebreaks(hand_t *hand) {
	int runCountIndex = -1;
	int runCount[CARDS_PER_HAND];
	memset(runCount, 0, sizeof(runCount));
	int numJokers = 0;

	char last = ' ';
	int maxRun = 1;
	for (int i = 0; i < CARDS_PER_HAND; i++) {
		if (hand->sortedCards[i] == 'J') {
			numJokers++;
		}
		if (hand->sortedCards[i] == last) {
			runCount[runCountIndex]++;
			if (runCount[runCountIndex] > maxRun) {
				if (mode == JOKERS && last == 'J') {
					//Don't count jokers in the maxRun because we'll apply them to something else	
					
				} else {
					maxRun = runCount[runCountIndex];
				}
			}
		} else {
			runCount[++runCountIndex] = 1;
			last = hand->sortedCards[i];
		}
	}

	if (mode == JOKERS) {
		maxRun += numJokers;
	}

	if (maxRun == 5 || numJokers == 5) {
		return FIVE_OF_A_KIND;
	} else if (maxRun == 4) {
		return FOUR_OF_A_KIND;
	} else if (maxRun == 3) {
		if (mode == JOKERS && numJokers > 0) {
			if (runCount[3] == 0) {
				return FULL_HOUSE;
			} else {
				return THREE_OF_A_KIND;
			}
		} else {
			if (runCount[2] == 0) {
				return FULL_HOUSE;
			} else {
				return THREE_OF_A_KIND;
			}
		}
	} else if (maxRun == 2) {
		if (runCount[3] == 0) {
			return TWO_PAIR;
		} else {
			return ONE_PAIR; 
		}
	} else {
		return HIGH_CARD;
	}	

	return 0;
	
}

int scoreCard(char card) {
	if (card == 'A') {
		return 100;
	} else if (card == 'K') {
		return 99;
	} else if (card == 'Q') {
		return 98;
	} else if (card == 'J') {
		if (mode == JOKERS) {
			return 1;
		} else {
			return 97;
		}
	} else if (card == 'T') {
		return 96;
	} else {
		return card - '0';
	}
}

int compHands(const void *elem1, const void *elem2) {
	hand_t *f = (hand_t *) elem1;
	hand_t *s = (hand_t *) elem2;

	int fScore = scoreHandNoTiebreaks(f);
	int sScore = scoreHandNoTiebreaks(s);

	//break the tie
	int i = 0;
	while (fScore == sScore && i < CARDS_PER_HAND) {
		//if the same cards, will do nothing, if greater will add and if lesser will reduce fScore
		fScore += scoreCard(f->cards[i]) - scoreCard(s->cards[i]);
		i++;
	}

    if (fScore > sScore) return  1;
    if (fScore < sScore) return -1;
    return 0;
}


int compCards (const void * elem1, const void * elem2)
{
    char f = *((char *)elem1);
    char s = *((char *)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
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

	int i = 0;

	while (getline(&line, &lineCap, fp) > 0) {
		strncpy(hands[i].cards, line, CARDS_PER_HAND);	
		hands[i].bid = atoi(&line[6]);

		memcpy(hands[i].sortedCards, hands[i].cards, sizeof(hands[i].sortedCards));
		qsort(hands[i].sortedCards, CARDS_PER_HAND, sizeof(char), compCards);
		

		printf("%s %d\n", hands[i].cards, hands[i].bid);
		i++;		
	}

	//part 1
	qsort(hands, NUM_HANDS, sizeof(hand_t), compHands);

	long totalScore = 0;
	for (int count = 0; count < NUM_HANDS; count++) {
		printf("%s %d %d\n", hands[count].sortedCards, hands[count].bid, scoreHandNoTiebreaks(&hands[count]));
		totalScore += (count+1) * hands[count].bid;
	}

	printf("Part 1 answer = %ld\n", totalScore);

	//part 2
	mode = JOKERS;
	qsort(hands, NUM_HANDS, sizeof(hand_t), compHands);

	totalScore = 0;
	for (int count = 0; count < NUM_HANDS; count++) {
		printf("%s %d %d\n", hands[count].sortedCards, hands[count].bid, scoreHandNoTiebreaks(&hands[count]));
		totalScore += (count+1) * hands[count].bid;
	}

	printf("Part 2 answer = %ld\n", totalScore);


	fclose(fp);
	free(line);

	return 0;
}
