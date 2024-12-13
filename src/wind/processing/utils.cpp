#include <wind/processing/utils.h>
#include <string>
#include <random>
#include <filesystem>

bool LexUtils::whitespace(char c) {
  return (
    c == ' ' ||
    c == '\t' ||
    c == '\n' ||
    c == '\r'
  );
}

bool LexUtils::hexadecimal(char c) {
  return (
    (c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'f') ||
    (c >= 'A' && c <= 'F') ||
    (c == 'x')
  );
}

bool LexUtils::digit(char c) {
  return (c >= '0' && c <= '9');
}

bool LexUtils::alphanum(char c) {
  return (
    (c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z')
  );
}

std::string generateRandomFilePath(const std::string& directory, const std::string& extension) {
  std::string tempDir = directory.empty() ? std::filesystem::temp_directory_path().string() : directory;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(100000, 999999);
  std::string randomFileName = "tmp" + std::to_string(dist(gen)) + extension;
  return tempDir + "/" + randomFileName;
}
