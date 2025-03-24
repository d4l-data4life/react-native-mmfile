#include <gtest/gtest.h>
#include "../MMapFile.h"
#include <vector>

TEST(MMapFileTest, CreatesAndMapsFile) {
  const std::string filePath = "/tmp/testfile";
  size_t size = 1024;

  MMapFile file(filePath);
  file.resize(size);
  ASSERT_NE(file.data(), nullptr);
  ASSERT_EQ(file.size(), size);

  // Write and read data
  std::string str = "Hello, TurboModule!";
  file.write(0, reinterpret_cast<const uint8_t*>(str.data()), str.size());

  std::string str2(str.size(), '\0');
  ASSERT_EQ(file.read(0, reinterpret_cast<uint8_t*>(const_cast<char*>(str2.data())), str.size()), str.size());
  ASSERT_EQ(str2, str);
  file.clear();
}

TEST(MMapFileTest, ThrowsForInvalidFile) {
  ASSERT_THROW(MMapFile("/invalid/path", 1024), std::runtime_error);
}
