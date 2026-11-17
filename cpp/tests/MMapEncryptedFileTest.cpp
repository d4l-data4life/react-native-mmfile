#include <gtest/gtest.h>
#include "../MMapEncryptedFile.h"
#include "../AES.h"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <unistd.h>

TEST(MMapEncryptedFileTest, FileFormatMismatch) {
  const std::string filePath = "/tmp/testfile2";
  size_t size = 1024;

  {
    MMapFile file(filePath);
    file.resize(10);
    ASSERT_EQ(file.size(), 10);
  }

  {
    std::string key(16, '\0');
    ASSERT_THROW(MMapEncryptedFile<128>(filePath, reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
  }

  {
    MMapFile file(filePath);
    file.clear();
  }
}

TEST(MMapEncryptedFileTest, CreatesAndMapsFile) {
  const std::string filePath = "/tmp/testfile2";
  size_t size = 1024;

  std::string key(16, '\0');
  fillRandom(key);

  MMapEncryptedFile<128> file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  ASSERT_EQ(file.size(), 0);

  // Write and read data
  std::string str = "Hello, NitroModules!";
  file.append(reinterpret_cast<const uint8_t*>(str.data()), str.size());
  ASSERT_EQ(file.size(), str.size());
  std::string str2(str.size(), '\0');
  ASSERT_EQ(file.read(0, reinterpret_cast<uint8_t*>(const_cast<char*>(str2.data())), str.size()), str.size());
  ASSERT_EQ(str2, str);
  file.clear();
}

TEST(MMapEncryptedFileTest, ReopenTest) {
  const std::string filePath = "/tmp/testfile3";

  std::string key(16, '\0');
  fillRandom(key);
  MMapEncryptedFile<128> file(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  file.clear();
  ASSERT_EQ(file.size(), 0);
  std::string str = "Hello World";
  file.append(reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size());
  ASSERT_EQ(file.size(), 11);
  file.close();

  MMapEncryptedFile<128> file2(filePath, reinterpret_cast<const uint8_t*>(key.data()));
  uint8_t buffer[15];
  size_t readCount = file2.read(3, buffer, 15);
  ASSERT_EQ(readCount, 8);
  file2.clear();
}

TEST(MMapEncryptedFileTest, ThrowsForInvalidFile) {
  std::string key(16, '\0');
  fillRandom(key);
  ASSERT_THROW(MMapEncryptedFile<128>("/invalid/path", reinterpret_cast<const uint8_t*>(key.data())), std::runtime_error);
}

// --- Keys from the OS random number generator, rekeying legacy files ---------------

namespace {

namespace fs = std::filesystem;

struct RekeyTest : ::testing::Test {
  fs::path dir;
  std::string tmpPath;
  std::string key = std::string(16, '\0');

  void SetUp() override {
    dir = fs::temp_directory_path() / ("mmfile-rekey-test-" + std::to_string(::getpid()));
    fs::remove_all(dir);
    fs::create_directories(dir);
    tmpPath = (dir / ".rekey" / "pending").string();
    fillRandom(key);
  }
  void TearDown() override { fs::remove_all(dir); }

  const uint8_t* k() const { return reinterpret_cast<const uint8_t*>(key.data()); }
  std::string path(const std::string& name) const { return (dir / name).string(); }

  void writeFile(const std::string& p, const std::string& content) {
    MMapEncryptedFile<128> file(p, k());
    file.append(reinterpret_cast<const uint8_t*>(content.data()), content.size());
  }

  // A file as written by mmfile <= 1.6.0: same layout, flags byte zero
  static void markLegacy(const std::string& p) {
    MMapFile raw(p);
    reinterpret_cast<EncryptedFileHeader*>(raw.data())->flags = 0;
  }

  void writeLegacyFile(const std::string& p, const std::string& content) {
    writeFile(p, content);
    markLegacy(p);
  }

  std::string readFile(const std::string& p) {
    MMapEncryptedFile<128> file(p, k(), true);
    std::string content(file.size(), '\0');
    file.read(0, reinterpret_cast<uint8_t*>(content.data()), content.size());
    return content;
  }

  std::string rawBytes(const std::string& p) {
    MMapFile raw(p, true);
    return std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
  }

  static std::string headerKeyAndIV(const std::string& bytes) {
    auto* h = reinterpret_cast<const EncryptedFileHeader*>(bytes.data());
    return std::string(reinterpret_cast<const char*>(h->encryptedDataKey), 16) +
           std::string(reinterpret_cast<const char*>(h->iv), 16);
  }
};

} // namespace

TEST(FillRandomTest, ProducesDistinctNonZeroOutput) {
  std::string a(32, '\0'), b(32, '\0');
  fillRandom(a);
  fillRandom(b);
  ASSERT_NE(a, b);
  ASSERT_NE(a, std::string(32, '\0'));
  // larger than a single getrandom/SecRandomCopyBytes chunk on some platforms
  std::string large(1 << 20, '\0');
  fillRandom(large);
  ASSERT_NE(large.substr(0, 64), std::string(64, '\0'));
  ASSERT_NE(large.substr(large.size() - 64), std::string(64, '\0'));
}

TEST_F(RekeyTest, NewFilesAreFlaggedAndStayReadableByOldVersions) {
  writeFile(path("a"), "hello");
  ASSERT_FALSE(isLegacyEncryptedFile(path("a")));
  {
    MMapEncryptedFile<128> file(path("a"), k(), true);
    ASSERT_FALSE(file.hasLegacyKeys());
  }
  // mmfile <= 1.6.0 only accepts version 1 and ignores the reserved bytes
  const std::string bytes = rawBytes(path("a"));
  auto* header = reinterpret_cast<const EncryptedFileHeader*>(bytes.data());
  ASSERT_EQ(FixEndianness(header->version), 1);
  ASSERT_EQ(header->flags, ENCRYPTED_FILE_FLAG_OS_RANDOM_KEYS);
}

TEST_F(RekeyTest, NewFilesGetDistinctKeysAndIVs) {
  writeFile(path("a"), "same content");
  writeFile(path("b"), "same content");
  ASSERT_NE(headerKeyAndIV(rawBytes(path("a"))), headerKeyAndIV(rawBytes(path("b"))));
  ASSERT_NE(rawBytes(path("a")), rawBytes(path("b")));
}

TEST_F(RekeyTest, ReadsLegacyFiles) {
  writeLegacyFile(path("legacy"), "legacy content");
  ASSERT_TRUE(isLegacyEncryptedFile(path("legacy")));
  ASSERT_EQ(readFile(path("legacy")), "legacy content");
  // writing to a legacy file keeps its (weak) keys until it is rekeyed
  {
    MMapEncryptedFile<128> file(path("legacy"), k());
    ASSERT_TRUE(file.hasLegacyKeys());
    std::string more = " and more";
    file.append(reinterpret_cast<const uint8_t*>(more.data()), more.size());
  }
  ASSERT_TRUE(isLegacyEncryptedFile(path("legacy")));
  ASSERT_EQ(readFile(path("legacy")), "legacy content and more");
}

TEST_F(RekeyTest, NonEncryptedFilesAreNotLegacy) {
  ASSERT_FALSE(isLegacyEncryptedFile(path("missing")));
  { std::FILE* f = std::fopen(path("empty").c_str(), "w"); std::fclose(f); }
  ASSERT_FALSE(isLegacyEncryptedFile(path("empty")));
  {
    MMapFile plain(path("plain"));
    std::string text(100, 'x');
    plain.append(reinterpret_cast<const uint8_t*>(text.data()), text.size());
  }
  ASSERT_FALSE(isLegacyEncryptedFile(path("plain")));
  ASSERT_FALSE(isLegacyEncryptedFile(dir.string()));
}

TEST_F(RekeyTest, RekeysLegacyFileUnderNewDataKeyAndIV) {
  const std::string p = path("legacy");
  writeLegacyFile(p, "sensitive payload");
  const auto mtime = fs::last_write_time(p) - std::chrono::hours(24);
  fs::last_write_time(p, mtime);
  const std::string before = rawBytes(p);

  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k()));

  const std::string after = rawBytes(p);
  ASSERT_FALSE(isLegacyEncryptedFile(p));
  ASSERT_EQ(readFile(p), "sensitive payload");
  ASSERT_NE(headerKeyAndIV(after), headerKeyAndIV(before));
  ASSERT_NE(after.substr(sizeof(EncryptedFileHeader)), before.substr(sizeof(EncryptedFileHeader)));
  ASSERT_EQ(after.size(), before.size());
  ASSERT_EQ(fs::last_write_time(p), mtime);
  ASSERT_FALSE(fs::exists(tmpPath));

  // already current: untouched unless forced
  ASSERT_FALSE(rekeyEncryptedFile<128>(p, tmpPath, k()));
  ASSERT_EQ(rawBytes(p), after);
  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k(), true));
  ASSERT_NE(headerKeyAndIV(rawBytes(p)), headerKeyAndIV(after));
  ASSERT_EQ(readFile(p), "sensitive payload");
}

TEST_F(RekeyTest, RekeysLargeFilesInChunks) {
  const std::string p = path("large");
  std::string content((3 << 20) + 12345, '\0');
  fillRandom(content);
  writeLegacyFile(p, content);
  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k()));
  ASSERT_FALSE(isLegacyEncryptedFile(p));
  ASSERT_EQ(readFile(p), content);
}

TEST_F(RekeyTest, RekeysWithAES256WrappingKey) {
  const std::string p = path("aes256");
  std::string key256(32, '\0');
  fillRandom(key256);
  const auto* k256 = reinterpret_cast<const uint8_t*>(key256.data());
  {
    MMapEncryptedFile<256> file(p, k256);
    std::string text = "wrapped with AES-256";
    file.append(reinterpret_cast<const uint8_t*>(text.data()), text.size());
  }
  markLegacy(p);
  ASSERT_TRUE(rekeyEncryptedFile<256>(p, tmpPath, k256));
  MMapEncryptedFile<256> file(p, k256, true);
  ASSERT_FALSE(file.hasLegacyKeys());
  std::string text(file.size(), '\0');
  file.read(0, reinterpret_cast<uint8_t*>(text.data()), text.size());
  ASSERT_EQ(text, "wrapped with AES-256");
}

TEST_F(RekeyTest, WrongKeyThrowsAndKeepsFile) {
  const std::string p = path("legacy");
  writeLegacyFile(p, "keep me");
  const std::string before = rawBytes(p);
  std::string wrongKey(16, '\0');
  fillRandom(wrongKey);
  ASSERT_THROW(rekeyEncryptedFile<128>(p, tmpPath, reinterpret_cast<const uint8_t*>(wrongKey.data())), std::runtime_error);
  ASSERT_EQ(rawBytes(p), before);
  ASSERT_FALSE(fs::exists(tmpPath));
}

TEST_F(RekeyTest, NonEncryptedOrMissingFileThrows) {
  {
    MMapFile plain(path("plain"));
    std::string text(100, 'x');
    plain.append(reinterpret_cast<const uint8_t*>(text.data()), text.size());
  }
  ASSERT_THROW(rekeyEncryptedFile<128>(path("plain"), tmpPath, k()), std::runtime_error);
  ASSERT_THROW(rekeyEncryptedFile<128>(path("missing"), tmpPath, k()), std::runtime_error);
  ASSERT_FALSE(fs::exists(path("missing")));
}

TEST_F(RekeyTest, FileInUseThrows) {
  const std::string p = path("legacy");
  writeLegacyFile(p, "in use");
  {
    MMapEncryptedFile<128> open(p, k());
    ASSERT_THROW(rekeyEncryptedFile<128>(p, tmpPath, k()), std::runtime_error);
  }
  ASSERT_TRUE(isLegacyEncryptedFile(p));
  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k()));
  ASSERT_EQ(readFile(p), "in use");
}

TEST_F(RekeyTest, ReplacesStaleTemporaryFile) {
  const std::string p = path("legacy");
  writeLegacyFile(p, "fresh");
  writeFile(tmpPath, "stale leftover of an interrupted rekey");
  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k()));
  ASSERT_EQ(readFile(p), "fresh");
  ASSERT_FALSE(fs::exists(tmpPath));
}

TEST_F(RekeyTest, EmptiesHeaderOnlyLegacyFile) {
  const std::string p = path("headerOnly");
  writeLegacyFile(p, "x");
  {
    MMapFile raw(p);
    reinterpret_cast<EncryptedFileHeader*>(raw.data())->size = 0;
    raw.resize(sizeof(EncryptedFileHeader), true);
  }
  ASSERT_TRUE(isLegacyEncryptedFile(p));
  ASSERT_TRUE(rekeyEncryptedFile<128>(p, tmpPath, k()));
  ASSERT_TRUE(fs::exists(p));
  ASSERT_EQ(fs::file_size(p), 0);
  ASSERT_FALSE(isLegacyEncryptedFile(p));
  ASSERT_FALSE(rekeyEncryptedFile<128>(p, tmpPath, k()));
}
