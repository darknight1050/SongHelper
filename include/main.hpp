#include <dlfcn.h>
#include <vector>
#include <map>
#include <chrono>
#include <iomanip>
#include "../extern/beatsaber-hook/shared/utils/utils.h"

using namespace std;

__attribute__((constructor)) void lib_main();