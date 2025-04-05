#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int head;
  int *requests;
  int count;
} DiscSchedule;

// custom compare function
int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

// first come first served
int fcfs(const DiscSchedule *input) {
  int total_seek = abs(input->requests[0] - input->head);

  for (int i = 1; i < input->count; i++) {
    total_seek += abs(input->requests[i] - input->requests[i - 1]);
  }
  return total_seek;
}

// shortest seek time first
int sstf(const DiscSchedule *input) {
  int total_seek = 0;
  int head = input->head;
  int count = input->count;

  bool *visited = calloc(count, sizeof(bool));

  for (int i = 0; i < count; i++) {
    int min_dist = INT_MAX;
    int next = -1;

    for (int j = 0; j < count; j++) {
      if (!visited[j]) {
        int dist = abs(input->requests[j] - head);
        if (dist < min_dist) {
          min_dist = dist;
          next = j;
        }
      }
    }

    if (next != -1) {
      visited[next] = true;
      total_seek += abs(input->requests[next] - head);
      head = input->requests[next];
    }
  }

  free(visited);
  return total_seek;
}
// non-circular look
int ncLook(const DiscSchedule *input) {
  int head = input->head;
  int count = input->count;

  int *sorted = malloc(count * sizeof(int));

  for (int i = 0; i < count; i++) {
    sorted[i] = input->requests[i];
  }

  qsort(sorted, count, sizeof(int), compare);

  int total_seek = 0;
  int current = head;

  int split = 0;
  while (split < count && sorted[split] < head)
    split++;

  // elevator going up
  for (int i = split; i < count; i++) {
    total_seek += abs(sorted[i] - current);
    current = sorted[i];
  }

  // elevator going down
  for (int i = split - 1; i >= 0; i--) {
    total_seek += abs(sorted[i] - current);
    current = sorted[i];
  }

  free(sorted);

  return total_seek;
}

// circular look
int cLook(const DiscSchedule *input) {
  int count = input->count;
  int head = input->head;

  int *sorted = malloc(count * sizeof(int));
  for (int i = 0; i < count; i++) {
    sorted[i] = input->requests[i];
  }

  qsort(sorted, count, sizeof(int), compare);

  int total_seek = 0;
  int current = head;

  int split = 0;

  while (split < count && sorted[split] < head)
    split++;

  for (int i = split; i < count; i++) {
    total_seek += abs(sorted[i] - current);
    current = sorted[i];
  }

  if (split > 0) {
    total_seek += abs(sorted[0] - current);
    current = sorted[0];

    for (int i = 0; i < split; i++) {
      total_seek += abs(sorted[i] - current);
      current = sorted[i];
    }
  }

  free(sorted);
  return total_seek;
}

DiscSchedule buildFromFile(char *filename) {
  freopen(filename, "r", stdin);

  int capacity = 20;
  int count = 0;
  int *numbers = malloc(capacity * sizeof(int));
  int total_read = 0;

  char *line = NULL;
  size_t len = 0;

  while (getline(&line, &len, stdin) != -1) {
    if (count == capacity) {
      capacity *= 2;
      numbers = realloc(numbers, capacity * sizeof(int));

      if (!numbers) {
        perror("Couldn't resize array");
        exit(EXIT_FAILURE);
      }
    }
    numbers[count++] = atoi(line);
    total_read++;
  }

  DiscSchedule input;
  input.head = numbers[0];
  input.requests = &numbers[1];
  input.count = total_read - 1;
  free(line);

  /*
   int sum = 0;
   printf("Here's what I read :)");
   for (int i = 0; i < count; i++) {
     printf("%d, ", numbers[i]);
     sum += numbers[i];
   }
   printf("\n Sum is: %d\n", sum);*/

  return input;
}

void runTests() {

  DiscSchedule input = buildFromFile("block-list.txt");
  printf("FCFS Total Seek: %d\n", fcfs(&input));
  printf("SSTF Total Seek: %d\n", sstf(&input));
  printf("LOOK Total Seek: %d\n", ncLook(&input));
  printf("C-LOOK Total Seek: %d\n", cLook(&input));
}

int main() {

  runTests();

  return 0;
}