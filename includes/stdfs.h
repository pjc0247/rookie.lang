#ifdef _MSC_VER
#include <filesystem>
#define fs std::experimental::filesystem
#else
#include <experimental/filesystem>
#define fs std::filesytem
#endif