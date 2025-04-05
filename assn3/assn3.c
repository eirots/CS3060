/**
 * Filename: assn3.c
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

/**
 * main - The main entry point for the program.
 * @argc: The number of command-line arguments.
 * @argv: An array of strings containing the command-line arguments.
 *
 * Return: 0 on success, non-zero on failure.
 */

typedef struct {
  int number;
} ThreadArguments;

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

void *threadWork(void *arg) {
  ThreadArguments *args = (ThreadArguments *)arg;
  int number = args->number;

  int *factors = trialDivision(number);

  free(args);

  pthread_exit(factors);
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <number1> <number2> ...\n", argv[0]);
    return EXIT_FAILURE;
  }

  int numthreads = argc - 1;

  pthread_t *threads = malloc(numthreads * sizeof(pthread_t));

  // DEBUG LINE, RAN INTO TROUBLE A FEW TIMES
  if (!threads) {
    perror("Could not allocate memory for threads");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < numthreads; i++) {
    ThreadArguments *threadArgs = malloc(sizeof(ThreadArguments));

    // DEBUG LINES, RAN INTO TROUBLE A FEW TIMES
    if (!threadArgs) {
      perror("Couldn't allocate memory for thread arguments");
      return EXIT_FAILURE;
    }

    threadArgs->number = atoi(argv[i + 1]);

    // DEBUG LINE, RAN INTO TROUBLE A FEW TIMES
    if (pthread_create(&threads[i], NULL, threadWork, threadArgs) != 0) {
      perror("error in: pthread_create");
      free(threadArgs);
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < numthreads; i++) {
    int *factors = NULL;

    // main thread waiting here
    if (pthread_join(threads[i], (void **)&factors) != 0) {
      perror("error in: pthread_join");
      continue;
    }

    if (factors) {
      int number = atoi(argv[i + 1]);
      printf("%d:\t", number);

      for (int factor = 0; factors[factor] != -1; factor++) {
        printf(" %d ", factors[factor]);
      }
      printf("\n");

      free(factors);
    }
  }
  free(threads);

  /*
  create multithreaded program that takes one or more numeric arguments from
  command line and displays the prime factors of each number.

  Each child thread will be passed a numbwer from the parent thread that should
  be factored by that thread.

  each child thread will return a sorted integer array of prime factors for that
  number.

  The parent thread will wait for each thread to terminate

  The parent thread will cout the original number provided and all factors
  returned by the child thread

  Child thread will not display any data. Only output should be provided by main
  thread, and only after waiting.

  */

  return 0;
}
