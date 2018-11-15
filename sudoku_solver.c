#include <inttypes.h> //// gcc -lpthread sudoku_solver.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define BOARD_LENGTH 9
#define setLocation(n, val) ((val) |= (1 << (n)))

static char *_ROW = "Row";
static char *_COL = "Col";
static char *_BOX = "Box";

static int sudoku_board[BOARD_LENGTH][BOARD_LENGTH];

struct sudoku_data {
  char *solve_for;
  uint8_t num;
  uint8_t is_valid;
  uint16_t num_seen;
};

void checkForSudoku(struct sudoku_data *);

const char *print_mapping[BOARD_LENGTH] = {
  "Top Left", "Top Middle", "Top Right",
  "Mid Left", "Mid Middle", "Mid Right",
  "Bot Left", "Bot Middle", "Bot Right"
};

void* solve_row(struct sudoku_data *s_d) {
  for (int i = 0; i < BOARD_LENGTH; i++) {
    s_d->num_seen = setLocation(sudoku_board[s_d->num][i] - 1, s_d->num_seen);
  }
  checkForSudoku(s_d);
  return s_d;
}

void* solve_column(struct sudoku_data *s_d) {
  for (int i = 0; i < BOARD_LENGTH; i++) {
    s_d->num_seen = setLocation(sudoku_board[i][s_d->num] - 1, s_d->num_seen);
  }
  checkForSudoku(s_d);
  return s_d;
}

void* solve_block(struct sudoku_data *s_d) {
  for (int i = (s_d->num / 3) * 3; i <= (s_d->num / 3) * 3 + 2; i++) {
    for (int j = (s_d->num % 3) * 3; j <= (s_d->num % 3) * 3 + 2; j++) {
      s_d->num_seen = setLocation(sudoku_board[i][j] - 1, s_d->num_seen);
    }
  }
  checkForSudoku(s_d);
  return s_d;
}

struct sudoku_data *_s_malloc_sudoku(char *solve_for, uint8_t num) {
  struct sudoku_data *s_d = malloc(sizeof(struct sudoku_data));
  s_d->solve_for = solve_for;
  s_d->num = num;
  s_d->num_seen = 0;
  s_d->is_valid = 0;
  return s_d;
}

void checkForSudoku(struct sudoku_data *s_d) {
  if (s_d->num_seen != 511) {
    if (strcmp(s_d->solve_for, "Box") == 0) {
      printf("%s %d doesn't have the required values.\n", print_mapping[s_d->num], s_d->num + 1);
    } else {
      printf("%s %d doesn't have the required values.\n", s_d->solve_for, s_d->num + 1);
    }
    s_d->is_valid = 1;
  }
  else
    s_d->is_valid = 0;
}

int main(int argc, char **argv) {

  if (argc != 2) {
    printf("2 arguments required... <./sudoku_solver.out <filename>>\n");
    exit(-1);
  }

  FILE *file = fopen(*(argv + 1), "r");

  if (file == NULL) {
    printf("No file exists.\n");
    exit(-1);
  }

  int c, counter = 0;
  while ((c = getc(file)) != EOF) {
    if (c == ' ' || c == '\n') continue;
    sudoku_board[counter / BOARD_LENGTH][counter % BOARD_LENGTH] = (int)c - '0';
    ++counter;
  }

  pthread_t threads[27];
  int thread_count = 0;

  for (int i = 0; i < BOARD_LENGTH; i++) {
    struct sudoku_data *sdx =  _s_malloc_sudoku(_ROW, i);
    struct sudoku_data *sdx1 = _s_malloc_sudoku(_COL, i);
    struct sudoku_data *sdx2 = _s_malloc_sudoku(_BOX, i);
    pthread_create(&threads[thread_count++], NULL, (void *)solve_row, (void *) sdx);
    pthread_create(&threads[thread_count++], NULL, (void *)solve_column, (void *) sdx1);
    pthread_create(&threads[thread_count++], NULL, (void *)solve_block, (void *) sdx2);
  }

  int is_valid_sudoku = 0;
  for (int i = 0; i < BOARD_LENGTH * 3; i++) {
    void *sudoku_data_return;
    pthread_join(threads[i], &sudoku_data_return);
    struct sudoku_data *test = (struct sudoku_data *)sudoku_data_return;
    is_valid_sudoku |= test->is_valid;
    free(test);
  }

  if (is_valid_sudoku) {
    printf("The input is not a valid Sudoku.\n");
  } else {
    printf("The input is a valid Sudoku.\n");
  }

  return 1;
}
