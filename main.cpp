#include <sstream>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <vector>

using namespace std;

#define MATCH 1
#define GAP_PENALTY 2
#define MISMATCH_PENALTY 1

struct score {
  int score;
  bool processed;
};

struct process_matrix {
  int i;
  int j;
  string *seq_a;
  string *seq_b;
  vector<vector<struct score>> *H_matrix;
};

int similarity_score(char a, char b) {
  int score;
  if ((a == b) || (a == '?' || b == '?')) {
    score = MATCH;
  } else {
    score = -MISMATCH_PENALTY;
  }
  return score;
}

struct score max_score(int scores[]) {
  struct score s;
  s.score = 0;
  for (int i = 0; i < 3; i++) {
    if (scores[i] > s.score) {
      s.score = scores[i];
    }
  }
  s.processed = true;
  return s;
}

string slurp(const string &filename) {
  ifstream in(filename, ifstream::in);
  stringstream sstr;
  sstr << in.rdbuf();
  string file = sstr.str();
  return file;
}

void *process_matrix_thread(void *threadarg) {
  struct process_matrix *matrix;
  matrix = (struct process_matrix *)threadarg;
  int max_options[3];
  vector<vector<struct score>> H_matrix = *(matrix->H_matrix);
  string seq_a = *(matrix->seq_a);
  string seq_b = *(matrix->seq_b);
  if (H_matrix[matrix->i][matrix->j].processed) {
    pthread_exit(NULL);
  }
  if (H_matrix[matrix->i - 1][matrix->j - 1].processed &&
      H_matrix[matrix->i - 1][matrix->j].processed &&
      H_matrix[matrix->i][matrix->j - 1].processed) {
    max_options[0] =
        H_matrix[matrix->i - 1][matrix->j - 1].score +
        similarity_score(seq_a.at(matrix->i - 1), seq_b.at(matrix->j - 1));
    max_options[1] = H_matrix[matrix->i - 1][matrix->j].score - GAP_PENALTY;
    max_options[2] = H_matrix[matrix->i][matrix->j - 1].score - GAP_PENALTY;
    (*matrix->H_matrix)[matrix->i][matrix->j] = max_score(max_options);
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int c;
  bool showtime = false;
  string file1 = argv[1];
  string file2 = argv[2];
  if (argc > 1) {
    for (c = 1; c < argc; c++) {
      if (strcmp(argv[c], "-t") == 0) {
        showtime = true;
      }
    }
  }

  if (argc < 3) {
    cout << "Usage: sequence-alignment filename_sequence_a filename_sequence_b "
            "[-t]";
    return 0;
  }
  cout << "Starting alignment with files: " << file1 << ", " << file2 << endl;

  clock_t begin_read = clock();
  string seq_a = slurp(file1);
  string seq_b = slurp(file2);
  clock_t end_read = clock();
  double elapsed_secs_read = double(end_read - begin_read) / CLOCKS_PER_SEC;

  // create the matrices
  size_t length_m = seq_a.size();
  size_t length_n = seq_b.size();
  struct score unprocessed_score;
  unprocessed_score.score = 0;
  unprocessed_score.processed = false;
  vector<vector<struct score>> H_matrix;
  H_matrix.resize(length_m,
                  std::vector<struct score>(length_n, unprocessed_score));

  struct score s;
  s.score = 0;
  s.processed = true;
  // set first row and first column to zero
  for (int i = 0; i < length_m; i++) {
    H_matrix[0][i] = s;
  }
  for (int j = 0; j < length_n; j++) {
    H_matrix[j][0] = s;
  }

  clock_t begin_matrix_calc = clock();
  struct process_matrix pm;
  pm.i = 1;
  pm.j = 1;
  pm.seq_a = &seq_a;
  pm.seq_b = &seq_b;
  pm.H_matrix = &H_matrix;
  process_matrix_thread((void *)&pm);
  struct process_matrix pm1;
  struct process_matrix pm2;
  pthread_t threads[length_m * length_n];
  for (int i = 2; i < length_m; i++) {
    for (int j = 1; j < length_n; j++) {
      pm1.i = i;
      pm1.j = j;
      pm1.seq_a = &seq_a;
      pm1.seq_b = &seq_b;
      pm1.H_matrix = &H_matrix;

      pm2.i = j;
      pm2.j = i;
      pm2.seq_a = &seq_a;
      pm2.seq_b = &seq_b;
      pm2.H_matrix = &H_matrix;

      pm.i = i + 1;
      pm.j = j;
      pm.seq_a = &seq_a;
      pm.seq_b = &seq_b;
      pm.H_matrix = &H_matrix;

      pthread_create(&threads[(j * i) + 0], NULL, process_matrix_thread,
                     (void *)&pm1);
      pthread_create(&threads[(j * i) + 1], NULL, process_matrix_thread,
                     (void *)&pm2);
      pthread_create(&threads[(j * i) + 2], NULL, process_matrix_thread,
                     (void *)&pm);
    }
  }
  clock_t end_matrix_calc = clock();
  double elapsed_secs_matrix_calc =
      double(end_matrix_calc - begin_matrix_calc) / CLOCKS_PER_SEC;

  for (int i = 0; i < length_m; i++) {
    for (int j = 0; j < length_n; j++) {
      cout << H_matrix[i][j].score << ' ';
    }
    cout << endl;
  }

  cout << "Elapsed Calculation Time: " << elapsed_secs_matrix_calc << endl;
  return 0;
}
