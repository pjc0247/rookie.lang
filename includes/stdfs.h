#ifdef _MSC_VER
#include <filesystem>
#define fs std::filesystem
#else
#include <experimental/filesystem>
#define fs std::filesytem
#endif