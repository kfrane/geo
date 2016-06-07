#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

bool file_exists(const char* filename) {
  int fd = open(
      filename,
      O_CREAT | O_EXCL | O_RDWR,
      S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH );
  if (fd != -1) {
    close(fd);
  }
  return fd == -1;
  // struct stat fileInfo;
  // return stat(filename, &fileInfo) == 0;
}

std::ofstream& create_next_file(std::string base_filename) {
  for (int file_id = 0;; file_id++) {
    std::stringstream filename;
    filename << base_filename << "_"
             << std::setfill('0') << std::setw(4) << file_id;
    if (!file_exists(filename.str().c_str())) {
      return *new std::ofstream(filename.str(), std::ofstream::out);
      // return *new std::ofstream("results/bla", std::ofstream::out);
    }
  }
}

std::ofstream& init_log(std::string folder, int argc, char** argv) {
  if (folder[folder.size()-1] != '/') {
    folder += "/";
  }
  std::ofstream& log_stream = create_next_file(folder+std::string(argv[0]));
  for (int arg = 0; arg < argc; arg++) {
    log_stream << argv[arg] << " ";
  }
  log_stream << std::endl;
  return log_stream;
}
