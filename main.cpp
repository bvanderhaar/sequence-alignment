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

struct process_matrix {
  int *i;
  int *j;
  string *seq_a;
  string *seq_b;
  vector<vector<int>> *H_matrix;
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

int max_score(int scores[]) {
  int score = 0;
  for (int i = 0; i < 3; i++) {
    if (scores[i] > score) {
      score = scores[i];
    }
  }
  return score;
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
  vector<vector<int>> H_matrix = *(matrix->H_matrix);
  string seq_a = *(matrix->seq_a);
  string seq_b = *(matrix->seq_b);
  max_options[0] =
      H_matrix[*(matrix->i) - 1][*(matrix->j) - 1] +
      similarity_score(seq_a.at(*(matrix->i) - 1), seq_b.at(*(matrix->j) - 1));
  max_options[1] = H_matrix[*(matrix->i) - 1][*(matrix->j)] - GAP_PENALTY;
  max_options[2] = H_matrix[*(matrix->i)][*(matrix->j) - 1] - GAP_PENALTY;
  H_matrix[*(matrix->i)][*(matrix->j)] = max_score(max_options);
  // this is inefficient and should be fixed
  (*matrix->H_matrix) = H_matrix;
  return 0;
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
  // cout << "length m " << length_m << endl;
  // cout << "length n " << length_n << endl;
  vector<vector<int>> H_matrix;
  H_matrix.resize(length_m, std::vector<int>(length_n, 0));

  // set first row and first column to zero
  for (int i = 0; i < length_m; i++) {
    H_matrix[0][i] = 0;
  }
  for (int j = 0; j < length_n; j++) {
    H_matrix[j][0] = 0;
  }

  struct process_matrix pm;
  clock_t begin_matrix_calc = clock();
  for (int i = 1; i < length_m; i++) {
    for (int j = 1; j < length_n; j++) {
      pm.i = &i;
      pm.j = &j;
      pm.seq_a = &seq_a;
      pm.seq_b = &seq_b;
      pm.H_matrix = &H_matrix;
      process_matrix_thread((void *)&pm);
    }
  }
  clock_t end_matrix_calc = clock();
  double elapsed_secs_matrix_calc =
      double(end_matrix_calc - begin_matrix_calc) / CLOCKS_PER_SEC;

  for (int i = 0; i < length_m; i++) {
    for (int j = 0; j < length_n; j++) {
      cout << H_matrix[i][j] << ' ';
    }
    cout << endl;
  }

  cout << "Elapsed Calculation Time: " << elapsed_secs_matrix_calc << endl;
  return 0;
}
