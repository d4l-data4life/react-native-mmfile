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
  std::string str = "Hello, NitroModules!";
  file.write(0, reinterpret_cast<const uint8_t*>(str.data()), str.size());

  std::string str2(str.size(), '\0');
  ASSERT_EQ(file.read(0, reinterpret_cast<uint8_t*>(const_cast<char*>(str2.data())), str.size()), str.size());
  ASSERT_EQ(str2, str);
  file.clear();
}

TEST(MMapFileTest, Resize) {
  const std::string filePath = "/tmp/testfile2";

  MMapFile file(filePath);
  file.resize(200, true);
  file.resize(800, true);
  file.resize(100, true);
  file.resize(0, true);
}

TEST(MMapFileTest, AppendAndClear) {
  const std::string filePath = "/tmp/testfile3";
  std::string str = "Hello, NitroModules!";

  MMapFile file(filePath);
  for (int i = 0; i < 3; ++i) {
    file.clear();
    for (int j = 0; j < 1000; ++j) {
      file.append(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }
  }
  file.clear();
}

TEST(MMapFileTest, ThrowsForInvalidFile) {
  ASSERT_THROW(MMapFile("/invalid/path", 1024), std::runtime_error);
}

TEST(MMapFileTest, Locking) {
  const std::string filePath = "/tmp/testfile4";
  {
    // will be closed automatically after the scope
    MMapFile file(filePath);
  }
  MMapFile file2(filePath);
  MMapFile file3;
  ASSERT_THROW(file3.open(filePath), std::runtime_error);
}
