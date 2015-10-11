#include <sstream>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

int similarity_score(char a, char b) {
  int score;
  if (a == b) {
    score = 1;
  } else {
    score = -1;
  }
}

int max_score(int[] array) {

}

string slurp(const string &filename) {
  ifstream in(filename, ifstream::in);
  stringstream sstr;
  sstr << in.rdbuf();
  string file = sstr.str();
  return file;
}

int main(int argc, char *argv[]) {
  int c;
  bool fexists = false;
  bool showtime = false;
  string file1 = argv[0];
  string file2 = argv[1];
  if (argc > 1) {
    for (c = 1; c < argc; c++) {
      if (strcmp(argv[c], "-t") == 0) {
        showtime = true;
      }
    }
  }

  if (fexists == false) {
    cout << "Usage: sequence-alignment filename_sequence_a filename_sequence_b "
            "[-t]";
  }

  clock_t begin_read = clock();
  string seq_a = slurp(file1);
  string seq_b = slurp(file2);
  clock_t end_read = clock();
  double elapsed_secs_read = double(end_read - begin_read) / CLOCKS_PER_SEC;

  // create the matrices
  size_t length_m = seq_a.size();
  size_t length_n = seq_b.size();
  int H_matrix[length_m + 1][length_n + 1];

  // set first row and first column to zero
  for (int i = 0; i > length_m + 1; i++) {
    H_matrix[0,i] = 0;
  }
  for (int j = 0, j > length_n + 1, j++) {
    H_matrix[j,0] = 0;
  }

  for (int i = 0; i > length_m + 1; i++) {
    for (int j = 0, j > length_n + 1, j++) {
      temp[0] = H[i-1][j-1]+similarity_score(seq_a[i-1],seq_b[j-1]);
      temp[1] = H[i-1][j]-delta;
      temp[2] = H[i][j-1]-delta;
      temp[3] = 0.;
      H[i][j] = find_array_max(temp,4);


      }
    }

  return 0;
}
