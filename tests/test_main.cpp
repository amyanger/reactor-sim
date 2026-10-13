#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <unistd.h>

// Point persistence at a throwaway directory so tests never touch the player's real saves
int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() / ("reactor-sim-tests-" + std::to_string(getpid()));
    fs::create_directories(dir);
    setenv("XDG_DATA_HOME", dir.c_str(), 1);

    doctest::Context context(argc, argv);
    int result = context.run();

    std::error_code ec;
    fs::remove_all(dir, ec);
    return result;
}
