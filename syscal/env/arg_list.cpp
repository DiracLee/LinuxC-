#include <getopt.h>

#include <cstdlib>
#include <iostream>

using namespace std;

const char *program_name;

const char *output_filename;
int verbose = 0;

void OutputInfo(ostream &os, int exit_code) {
  cout << "Usage: " << program_name << "<options> [file]" << endl;
  cout << "-h, --help: Display this usage infomation." << endl;
  cout << "-o, --output <file>: Write the output to file." << endl;
  cout << "-v, --verbose: Print verbose messages." << endl;
  exit(exit_code);
}

int main(int argc, char *argv[]) {
  program_name = argv[0];

  const char *const shot_opts =
      "ho:v";  // shot option "-o" has additional params.

  const struct option long_opts[] = {{"help", 0, nullptr, 'h'},
                                     {"output", 1, nullptr, 'o'},
                                     {"verbose", 0, nullptr, 'v'},
                                     {nullptr, 0, nullptr, 0}};
  // return shot option name
  //        '?' when invalid name
  //        -1  when no option given
  int opt = getopt_long(argc, argv, shot_opts, long_opts, nullptr);

  while (opt != -1) {
    switch (opt) {
      case 'h':
        OutputInfo(cout, 0);
      case 'o': {
        output_filename = optarg;  // additional params are stored in `optarg`
        cout << "The output filename is set as: " << output_filename << endl;
        break;
      }
      case 'v': {
        verbose = 1;
      }
      case '?': {
        OutputInfo(cout, 1);
      }
      case -1: {
        break;
      }
      default: {
        abort();
      }
    }

    opt = getopt_long(argc, argv, shot_opts, long_opts, nullptr);
  }

  return 0;
}