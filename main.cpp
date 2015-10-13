#include <sstream>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <vector>
using namespace std;

const int MATCH = 1;
const int GAP_PENALTY = 2;
const int MISMATCH_PENALTY = 1;

string GKnownSequence;
string GSearchSequence;
vector<vector<int>> GMatrix;

struct process_matrix {
  int known_sequence_slot;
  int search_sequence_slot;
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

int max_score(int scores[3]) {
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

void *process_known_row(void *threadarg) {
  struct process_matrix *matrix;
  matrix = (struct process_matrix *)threadarg;
  int max_options[3];
  if (GMatrix[matrix->search_sequence_slot - 1][matrix->known_sequence_slot] !=
      -1) {
    max_options[0] =
        GMatrix[matrix->search_sequence_slot - 1][matrix->known_sequence_slot -
                                                  1] +
        similarity_score(GKnownSequence.at(matrix->known_sequence_slot - 1),
                         GSearchSequence.at(matrix->search_sequence_slot - 1));
    max_options[1] = GMatrix[(matrix->search_sequence_slot) -
                             1][(matrix->known_sequence_slot)] -
                     GAP_PENALTY;
    max_options[2] =
        GMatrix[matrix->search_sequence_slot][matrix->known_sequence_slot - 1] -
        GAP_PENALTY;
    GMatrix[matrix->search_sequence_slot][matrix->known_sequence_slot] =
        max_score(max_options);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int c;
  bool showtime = false;
  bool showmatrix = false;
  string filename_known_sequence = argv[1];
  string filename_search_sequence = argv[2];
  if (argc > 1) {
    for (c = 1; c < argc; c++) {
      if (strcmp(argv[c], "-t") == 0) {
        showtime = true;
      }
      if (strcmp(argv[c], "-p") == 0) {
        showmatrix = true;
      }
    }
  }

  if (argc < 3) {
    cout << "Usage: sequence-alignment filename_known_sequence "
            "filename_search_sequence "
            "[-t]";
    return 0;
  }
  cout << "Starting alignment with known sequence: " << filename_known_sequence
       << endl;
  cout << "Starting alignment with search sequence " << filename_search_sequence
       << endl;

  clock_t begin_read = clock();
  GKnownSequence = slurp(filename_known_sequence);
  GSearchSequence = slurp(filename_search_sequence);
  clock_t end_read = clock();
  double elapsed_secs_read = double(end_read - begin_read) / CLOCKS_PER_SEC;

  // create the matrices
  size_t known_sequence_size = GKnownSequence.size();
  size_t search_sequence_size = GSearchSequence.size();
  GMatrix.resize(search_sequence_size,
                 std::vector<int>(known_sequence_size, -1));

  // set first row to zero
  for (int i = 0; i < known_sequence_size; i++) {
    GMatrix[0][i] = 0;
  }
  // set first column to zero
  for (int j = 0; j < search_sequence_size; j++) {
    GMatrix[j][0] = 0;
  }

  clock_t begin_matrix_calc = clock();
  struct process_matrix pm;
  for (int i = 1; i < search_sequence_size; i++) {
    for (int j = 1; j < known_sequence_size; j++) {
      pm.known_sequence_slot = j;
      pm.search_sequence_slot = i;
      process_matrix_thread((void *)&pm);
    }
  }
  clock_t end_matrix_calc = clock();
  double elapsed_msecs_matrix_calc =
      double(end_matrix_calc - begin_matrix_calc) / (CLOCKS_PER_SEC / 1000);

  if (showmatrix) {
    for (int i = 0; i < search_sequence_size; i++) {
      for (int j = 0; j < known_sequence_size; j++) {
        cout << GMatrix[i][j] << ' ';
      }
      cout << endl;
    }
  }

  cout << "Elapsed Calculation Time: " << elapsed_msecs_matrix_calc << " ms"
       << endl;
  return 0;
}
