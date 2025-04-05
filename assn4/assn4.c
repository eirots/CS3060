/**
 * Filename: assn4.c
 */

/* Promise of Originality
I promise that this source code file has, in its entirety, been written by
myself and no other person or persons. If at any time an exact copy of this
source code is found to be used by another person in this term, I understand
that both myself and the student that submitted the copy will receive a zero on
this assignment.
*/

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  int argc;
  char **argv;
} ProducerArgs;

#define BUFFER_SIZE 10

int *numBuffer;
int inNums = 0;
int outNums = 0;
int countNums = 0;

int **factorBuffer;
int inFactors = 0;
int outFactors = 0;
int countFactors = 0;
int consumed = 0;

pthread_mutex_t factorsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t factorsNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t factorsNotFull = PTHREAD_COND_INITIALIZER;

pthread_mutex_t numbersMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t numsNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t numsNotFull = PTHREAD_COND_INITIALIZER;

// adapted trial division algorithm from
// https://literateprograms.org/trial_division__c_.html
int isprime(unsigned long x) {
  if (x < 2)
    return 0;

  if (x % 2 == 0) {
    return (x == 2);
  }

  unsigned long limit = (unsigned long)floor(sqrt(((long double)x)));
  for (unsigned long i = 3; i <= limit; i += 2) {
    if (x % i == 0) {
      return 0;
    }
  }
  return 1;
}

// Adapted trial division algorithm from
// https://literateprograms.org/trial_division__c_.html to use int*
int *trialDivision(unsigned long n) {
  size_t cap = 10;
  size_t count = 0;
  int *factors = malloc(cap * sizeof(int));

  // debug line, ran into some problems
  if (!factors) {
    perror("Couldn't allocate memory for factors.");
    return NULL;
  }

  for (unsigned long i = 1; i <= n; i++) {
    while ((n % i == 0) && isprime(i)) {

      // if there are a lot of factors, copies and expands the array.
      if (count == cap) {
        cap *= 2;
        int *temp = realloc(factors, cap * sizeof(int));
        if (!temp) {
          perror("Coudn't upsize factor array");
          free(factors);
          return NULL;
        }
        factors = temp;
      }
      factors[count++] = i;
      n /= i;
    }
  }

  // making absolutely sure that there is space for a "marker" for the end of
  // the factors; assuming -1 isn't a factor
  if (count == cap) {
    cap++;
    int *temp = realloc(factors, cap * sizeof(int));
    if (!temp) {
      perror("couldn't upsize factor array");
      free(factors);
      return NULL;
    }
    factors = temp;
  }
  factors[count] = -1;

  return factors;
}

// printer
void *consumerFunction(void *arg) {
  (void)arg;

  while (1) {
    pthread_mutex_lock(&factorsMutex);
    while (countFactors == 0) {
      pthread_cond_wait(&factorsNotEmpty, &factorsMutex);
    }

    int *factors = factorBuffer[outFactors];
    outFactors = (outFactors + 1) % BUFFER_SIZE;
    countFactors--;

    pthread_cond_signal((&factorsNotFull));
    pthread_mutex_unlock(&factorsMutex);

    if (factors == NULL) {
      break;
    }
    // tar =cf assn4.tar.gz assn4
    printf("Factors:\t");
    for (int i = 0; factors[i] != -1; i++) {
      printf(" %d ", factors[i]);
    }
    printf("\n");

    free(factors);
  }

  return NULL;
}

// factor-er
void *producerFunction(void *arg) {
  (void)arg;

  while (1) {
    pthread_mutex_lock(&numbersMutex);

    while (countNums == 0) {
      pthread_cond_wait(&numsNotEmpty, &numbersMutex);
    }

    int num = numBuffer[outNums];
    outNums = (outNums + 1) % BUFFER_SIZE;
    countNums--;

    pthread_cond_signal(&numsNotFull);
    pthread_mutex_unlock(&numbersMutex);

    if (num == -1) {

      pthread_mutex_lock(&factorsMutex);
      while (countFactors == BUFFER_SIZE) {
        pthread_cond_wait(&factorsNotFull, &factorsMutex);
      }
      factorBuffer[inFactors] = NULL;
      inFactors = (inFactors + 1) % BUFFER_SIZE;
      countFactors++;
      pthread_cond_signal(&factorsNotEmpty);
      pthread_mutex_unlock(&factorsMutex);
      break;
    }

    int *factors = trialDivision(num);
    pthread_mutex_lock(&factorsMutex);
    while (countFactors == BUFFER_SIZE) {
      pthread_cond_wait(&factorsNotFull, &factorsMutex);
    }

    factorBuffer[inFactors] = factors;
    inFactors = (inFactors + 1) % BUFFER_SIZE;
    countFactors++;

    pthread_cond_signal(&factorsNotEmpty);
    pthread_mutex_unlock(&factorsMutex);
  }
  return NULL;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <number1> <number2> ...\n", argv[0]);
    return EXIT_FAILURE;
  }

  numBuffer = malloc(sizeof(int) * BUFFER_SIZE);
  factorBuffer = malloc(sizeof(int) * BUFFER_SIZE);

  pthread_t producerThread, consumerThread;
  pthread_create(&producerThread, NULL, producerFunction, NULL);
  pthread_create(&consumerThread, NULL, consumerFunction, NULL);

  for (int i = 1; i < argc; i++) {
    int num = atoi(argv[i]);

    pthread_mutex_lock(&numbersMutex);
    while (countNums == BUFFER_SIZE) {
      pthread_cond_wait(&numsNotFull, &numbersMutex);
    }

    numBuffer[inNums] = num;
    inNums = (inNums + 1) % BUFFER_SIZE;
    countNums++;

    pthread_cond_signal(&numsNotEmpty);
    pthread_mutex_unlock(&numbersMutex);
  }
  pthread_mutex_lock(&numbersMutex);
  while (countNums == BUFFER_SIZE) {
    pthread_cond_wait(&numsNotFull, &numbersMutex);
  }
  numBuffer[inNums] = -1;
  inNums = (inNums + 1) % BUFFER_SIZE;
  countNums++;
  pthread_cond_signal(&numsNotEmpty);
  pthread_mutex_unlock(&numbersMutex);

  pthread_join(producerThread, NULL);
  pthread_join(consumerThread, NULL);

  return 0;
}
