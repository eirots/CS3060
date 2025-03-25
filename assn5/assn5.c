#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// below value hard coded only for assignment purposes. Could add something that
// gets the total number of lines first, but I'm not doing that right now
#define MAX_PROCESSES 100

// ported queue from Sedgewick
typedef struct {
  int data[MAX_PROCESSES];
  int front;
  int rear;
} Queue;

void initQueue(Queue *q) {
  q->front = 0;
  q->rear = 0;
}

void enqueue(Queue *q, int value) {
  q->data[q->rear] = value;
  q->rear = (q->rear + 1) % MAX_PROCESSES;
}

int dequeue(Queue *q) {
  int val = q->data[q->front];
  q->front = (q->front + 1) % MAX_PROCESSES;
  return val;
}

bool isEmpty(const Queue *q) { return (q->front == q->rear); }

int readProcessData(const char *filename, int *start_times, int *durations);
void fcfs(const int arrival_times[], const int burst_times[],
          int n); // first come first served
void sjf(const int arrival_times[], const int burst_times[],
         int n); // shortest job first
void srtf(const int arrival_times[], const int burst_times[],
          int n); // shortest remaining time first
void roundRobin(const int arrival_times[], const int burst_times[],
                int n); // round robin
void printResults(char *name, int responses[], int turnaround[], int waiting[],
                  int n);

int readProcessData(const char *filename, int *start_times, int *durations) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Can't open file %s\n", filename);
    return -1;
  }

  int count = 0;

  while (fscanf(fp, "%d %d", &start_times[count], &durations[count]) == 2) {
    count++;
    if (count == MAX_PROCESSES) {
      fprintf(stderr, "Hit the max number of processes\n");
      break;
    }
  }

  fclose(fp);
  return count;
}

// first come first served
// assuming that arrival_times is already sorted like it was in the file
void fcfs(const int arrival_times[], const int burst_times[], int n) {
  // void to print results inside methods

  // 1. copy arrival_times and burst times
  // 2. calculate waiting times, turnaround times, response times
  // 3. print average metrics

  int arrivals[n], bursts[n];
  memcpy(arrivals, arrival_times, n * sizeof(int));
  memcpy(bursts, burst_times, n * sizeof(int));

  int responses[n], waiting[n], completion[n], turnaround[n];

  int current_time = 0;

  for (int i = 0; i < n; i++) {
    if (current_time < arrivals[i]) {
      current_time = arrivals[i];
    }
    responses[i] = current_time - arrivals[i];
    completion[i] = current_time + bursts[i];
    turnaround[i] = completion[i] - arrivals[i];
    waiting[i] = turnaround[i] - bursts[i];

    current_time = completion[i];
  }

  printResults("FCFS", responses, turnaround, waiting, n);
}

// shortest job first
void sjf(const int arrival_times[], const int burst_times[], int n) {
  int arrivals[n], bursts[n];
  memcpy(arrivals, arrival_times, n * sizeof(int));
  memcpy(bursts, burst_times, n * sizeof(int));

  int responses[n], waiting[n], completion[n], turnaround[n];
  bool finished[n];
  memset(finished, 0, sizeof(finished));

  int current_time = 0;
  int completed = 0;

  while (completed < n) {
    int next_index = -1;
    int min_burst = 999999999;

    for (int i = 0; i < n; i++) {
      if (!finished[i] && (arrivals[i] <= current_time)) {
        if (bursts[i] < min_burst) {
          min_burst = bursts[i];
          next_index = i;
        }
      }
    }

    if (next_index == -1) {
      int earliest = 999999999;
      for (int i = 0; i < n; i++) {
        if (!finished[i] && arrivals[i] < earliest) {
          earliest = arrivals[i];
        }
      }
      current_time = earliest;
      continue;
    }

    int i = next_index;

    responses[i] = current_time - arrivals[i];

    completion[i] = current_time + bursts[i];
    turnaround[i] = completion[i] - arrivals[i];
    waiting[i] = turnaround[i] - bursts[i];

    finished[i] = true;
    completed++;

    current_time = completion[i];
  }
  printResults("SJF", responses, turnaround, waiting, n);
}

// shortest remaining time first
void srtf(const int arrival_times[], const int burst_times[], int n) {
  int arrivals[n], bursts[n], remaining[n];
  memcpy(arrivals, arrival_times, n * sizeof(int));
  memcpy(bursts, burst_times, n * sizeof(int));
  memcpy(remaining, burst_times, n * sizeof(int));

  int responses[n], waiting[n], completion[n], turnaround[n];

  bool started[n], finished[n];
  memset(finished, 0, sizeof(finished));
  memset(started, 0, sizeof(started));

  int current_time = 0;
  int completed = 0;

  while (completed < n) {
    int next_ = -1;
    int min_remaining = 99999999;
    for (int i = 0; i < n; i++) {
      if (!finished[i] && arrivals[i] <= current_time) {
        if (remaining[i] < min_remaining) {
          min_remaining = remaining[i];
          next_ = i;
        }
      }
    }

    if (next_ == -1) {
      int earliest_ = 99999999;
      for (int i = 0; i < n; i++) {
        if (!finished[i] && arrivals[i] < earliest_) {
          earliest_ = arrivals[i];
        }
      }
      current_time = earliest_;
      continue;
    }

    int i = next_;
    if (!started[i]) {
      responses[i] = current_time - arrivals[i];
      started[i] = true;
    }

    int next_arrival = 999999999;
    for (int j = 0; j < n; j++) {
      if (!finished[j] && arrivals[j] > current_time &&
          arrivals[j] < next_arrival) {
        next_arrival = arrivals[j];
      }
    }

    if (next_arrival == 999999999) {
      next_arrival = current_time + remaining[i];
    }

    int slice = next_arrival - current_time;

    if (remaining[i] <= slice) {
      current_time += remaining[i];
      remaining[i] = 0;

      finished[i] = true;
      completed++;

      completion[i] = current_time;
      turnaround[i] = completion[i] - arrivals[i];
      waiting[i] = turnaround[i] - bursts[i];
    } else {
      remaining[i] -= slice;
      current_time += slice;
    }
  }

  printResults("SRTF", responses, turnaround, waiting, n);
}

// round robin
void roundRobin(const int arrival_times[], const int burst_times[], int n) {
  int arrivals[n], bursts[n], remaining[n];
  memcpy(arrivals, arrival_times, n * sizeof(int));
  memcpy(bursts, burst_times, n * sizeof(int));
  memcpy(remaining, burst_times, n * sizeof(int));

  int responses[n], waiting[n], completion[n], turnaround[n];

  bool started[n], finished[n];
  memset(finished, 0, sizeof(finished));
  memset(started, 0, sizeof(started));

  int current_time = 0;
  int completed = 0;
  int QUANTUM = 100;

  Queue r;
  initQueue(&r);

  int index = 0;
  while (completed < n) {
    while (index < n && arrivals[index] <= current_time) {
      enqueue(&r, index);
      index++;
    }

    if (isEmpty(&r)) {
      if (index < n) {
        current_time = arrivals[index];
      } else {
        break;
      }
      continue;
    }

    int i = dequeue(&r);
    if (!started[i]) {
      responses[i] = current_time - arrivals[i];
      started[i] = true;
    }

    int runtime = (remaining[i] < QUANTUM) ? remaining[i] : QUANTUM;

    current_time += runtime;
    remaining[i] -= runtime;

    while (index < n && arrivals[index] <= current_time) {
      enqueue(&r, index);
      index++;
    }

    if (remaining[i] > 0) {
      enqueue(&r, i);
    } else {
      finished[i] = true;
      completed++;
      completion[i] = current_time;
      turnaround[i] = completion[i] - arrivals[i];
      waiting[i] = turnaround[i] - bursts[i];
    }
  }

  printResults("Round Robin w/ quantum 100", responses, turnaround, waiting, n);
}

void printResults(char *name, int responses[], int turnaround[], int waiting[],
                  int n) {
  double total_wait = 0.0;
  double total_responses = 0.0;
  double total_turnaround = 0.0;

  for (int i = 0; i < n; i++) {
    total_wait += waiting[i];
    total_responses += responses[i];
    total_turnaround += turnaround[i];
  }
  double avg_wait = total_wait / n;
  double avg_responses = total_responses / n;
  double avg_turnaround = total_turnaround / n;

  printf("\n%s: \n", name);
  printf("\tAvg response: \t%.2f\n", avg_responses);
  printf("\tAvg turnaround: %.2f\n", avg_turnaround);
  printf("\tAvg wait: \t%.2f\n", avg_wait);
}

int main() {
  int start_times[MAX_PROCESSES];
  int durations[MAX_PROCESSES];

  int n = readProcessData("process_list.txt", start_times, durations);
  if (n < 0) {
    // will die if there's a problem opening the file
    return -1;
  }

  fcfs(start_times, durations, n);
  sjf(start_times, durations, n);
  srtf(start_times, durations, n);
  roundRobin(start_times, durations, n);

  // DEBUG LINES
  // printf("Read %d processes from file. \n", n);
  // for (int i = 0; i < n; i++) {
  //   printf("Process %2d: start = %5d, duration = %3d\n", i + 1,
  //   start_times[i],
  //          durations[i]);
  // }
}