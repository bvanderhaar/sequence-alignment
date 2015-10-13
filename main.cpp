#include <sstream>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

const int MATCH = 2;
const int GAP_PENALTY = 1;
const int MISMATCH_PENALTY = 1;

string GKnownSequence;
string GSearchSequence;
size_t GKnownSequenceLength;
vector<vector<int>> GMatrix;

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

void process_known_row(int search_sequence_slot) {
  int max_options[3];
  for (int i = 1; i < GKnownSequenceLength;) {
    if (GMatrix[search_sequence_slot - 1][i] != -1) {
      max_options[0] =
          GMatrix[search_sequence_slot - 1][i - 1] +
          similarity_score(GKnownSequence.at(i - 1),
                           GSearchSequence.at(search_sequence_slot - 1));
      max_options[1] = GMatrix[search_sequence_slot - 1][i] - GAP_PENALTY;
      max_options[2] = GMatrix[search_sequence_slot][i - 1] - GAP_PENALTY;
      GMatrix[search_sequence_slot][i] = max_score(max_options);
      i++;
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  int c;
  int numthreads = 2;
  bool showtime = false;
  bool showmatrix = false;
  string filename_known_sequence = argv[1];
  string filename_search_sequence = argv[2];
  if (argc > 1) {
    for (c = 1; c < argc; c++) {
      if (strcmp(argv[c], "-p") == 0) {
        showmatrix = true;
      }
      if (strcmp(argv[c], "-t") == 0) {
        numthreads = atoi(argv[c + 1]);
      }
    }
  }

  if (argc < 3) {
    cout << "Usage: sequence-alignment filename_known_sequence "
            "filename_search_sequence "
            "[-tp]";
    return 0;
  }
  cout << "Starting alignment with known sequence: " << filename_known_sequence
       << endl;
  cout << "Starting alignment with search sequence " << filename_search_sequence
       << endl;
  cout << "Starting alignment algorithm with " << numthreads << " thread(s)"
       << endl;

  clock_t begin_read = clock();
  GKnownSequence = slurp(filename_known_sequence);
  GSearchSequence = slurp(filename_search_sequence);
  clock_t end_read = clock();
  double elapsed_secs_read = double(end_read - begin_read) / CLOCKS_PER_SEC;

  // create the matrices
  GKnownSequenceLength = GKnownSequence.size();
  size_t search_sequence_size = GSearchSequence.size();
  GMatrix.resize(search_sequence_size,
                 std::vector<int>(GKnownSequenceLength, -1));

  // set first row to zero
  for (int i = 0; i < GKnownSequenceLength; i++) {
    GMatrix[0][i] = 0;
  }
  // set first column to zero
  for (int j = 0; j < search_sequence_size; j++) {
    GMatrix[j][0] = 0;
  }

  clock_t begin_matrix_calc = clock();
  if (numthreads == 1) {
    for (int i = 1; i < search_sequence_size; i++) {
      process_known_row(i);
    }
  } else {
    // divide the work by numthreads
    int cycles = search_sequence_size / numthreads;
    vector<thread> t(numthreads);
    int current_search_index = 1;
    int started_threads;
    for (int i = 0; i < cycles; i++) {
      started_threads = 0;
      for (int j = 0; j < numthreads; j++) {
        if (current_search_index < search_sequence_size) {
          t[j] = std::thread(process_known_row, current_search_index);
          started_threads++;
        }
        current_search_index++;
      }
      // Join the threads with the main thread
      for (int k = 0; k < started_threads; k++) {
        t[k].join();
      }
    }
  }
  clock_t end_matrix_calc = clock();
  double elapsed_msecs_matrix_calc =
      double(end_matrix_calc - begin_matrix_calc) / (CLOCKS_PER_SEC / 1000);

  if (showmatrix) {
    for (int i = 0; i < search_sequence_size; i++) {
      for (int j = 0; j < GKnownSequenceLength; j++) {
        cout << GMatrix[i][j] << ' ';
      }
      cout << endl;
    }
  }

  int best_score = 0;
  for (int i = 0; i < search_sequence_size; i++) {
    for (int j = 0; j < GKnownSequenceLength; j++) {
      if (GMatrix[i][j] > best_score) {
        best_score = GMatrix[i][j];
      }
    }
  }

  cout << "Elapsed Calculation Time: " << elapsed_msecs_matrix_calc << " ms"
       << endl;
  cout << "Max Score: " << best_score << endl;
  return 0;
}
