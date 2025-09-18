#include "FileUtility.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <ios>
#include <iterator>
#include <physfs.h>
#include <spdlog/fmt/bundled/core.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <libassert/assert.hpp>

#include "Logging.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "port_utils/NotImplemented.hpp"

namespace fs = std::filesystem;
using namespace std::string_view_literals;

constexpr auto LOG = "fs";

namespace
{
	struct FPhysfsIoMemory
	{
		using FileContent = std::vector<std::byte>;

		static auto GetThis(PHYSFS_Io* io) -> FPhysfsIoMemory*
		{
			return reinterpret_cast<FPhysfsIoMemory*>(io->opaque);
		}

		static auto Create(const std::string& nodeName, FileContent* content = nullptr) -> PHYSFS_Io*
		{
			auto* io = new PHYSFS_Io();
			io->version = 0;
			io->opaque = new FPhysfsIoMemory(nodeName, content);
			io->read = FPhysfsIoMemory::Read;
			io->write = FPhysfsIoMemory::Write;
			io->seek = FPhysfsIoMemory::Seek;
			io->tell = FPhysfsIoMemory::Tell;
			io->length = FPhysfsIoMemory::Length;
			io->duplicate = FPhysfsIoMemory::Duplicate;
			io->flush = FPhysfsIoMemory::Flush;
			io->destroy = FPhysfsIoMemory::Destroy;
			return io;
		}

		static auto Read(struct PHYSFS_Io* io, void* buf, PHYSFS_uint64 len) -> PHYSFS_sint64
		{
			auto* iom = GetThis(io);
			auto& fileContent = *iom->m_content;
			auto& pos = iom->m_position;
			if ((pos + len) > fileContent.size())
			{
				len = fileContent.size() - pos;
			}
			auto readStart = std::begin(fileContent) + pos;
			std::copy(readStart, readStart + len, reinterpret_cast<std::byte*>(buf));
			pos += len;
			return len;
		}

		static auto Write(struct PHYSFS_Io* io, const void* buffer, PHYSFS_uint64 len) -> PHYSFS_sint64
		{
			auto* iom = GetThis(io);
			auto& fileContent = *iom->m_content;
			auto& pos = iom->m_position;
			auto newSize = pos + len;
			if (newSize > fileContent.size())
			{
				fileContent.reserve(newSize);
			}
			std::copy(reinterpret_cast<const std::byte*>(buffer),
				reinterpret_cast<const std::byte*>(buffer) + len,
				std::inserter(fileContent, fileContent.begin() + pos));
			pos += len;
			return len;
		}

		static auto Seek(struct PHYSFS_Io* io, PHYSFS_uint64 offset) -> int
		{
			auto* t = GetThis(io);
			t->m_position = offset;
			return 1;
		}
		static auto Tell(struct PHYSFS_Io* /*io*/) -> PHYSFS_sint64
		{
			NotImplemented("fsMem");
			return {};
		}
		static auto Length(struct PHYSFS_Io* io) -> PHYSFS_sint64
		{
			auto* t = GetThis(io);
			return t->m_content->size();
		}

		static auto Duplicate(struct PHYSFS_Io* /*io*/) -> struct PHYSFS_Io*
		{
			NotImplemented("fsMem");
			return {};
		}
		static auto Flush(struct PHYSFS_Io* /*io*/) -> int
		{
			// do nothing?
			return 1;
		}
		static void Destroy(struct PHYSFS_Io* io)
		{
			delete GetThis(io);
			io->opaque = nullptr;
			delete io;
		}

	private:
		std::size_t m_position = 0;
		std::string mNodeName;
		FileContent* m_content = nullptr;
		bool m_ownsContent = false;

		FPhysfsIoMemory(std::string nodeName, FileContent* content) : mNodeName(std::move(nodeName)), m_content(content)
		{
			if (m_content == nullptr)
			{
				m_content = new FileContent();
				m_ownsContent = true;
			}
		}

		~FPhysfsIoMemory()
		{
			if (m_ownsContent)
			{
				delete m_content;
			}
		}
	};

	struct FMemoryArchive
	{
		std::string m_name;
		std::string m_mountPoint;
		PHYSFS_Io* m_io{ nullptr };
		std::unordered_map<std::string, std::vector<std::byte>> m_files;
		std::vector<std::string> m_directories;

		FMemoryArchive(const FMemoryArchive&) = default;
		FMemoryArchive(FMemoryArchive&&) = delete;
		FMemoryArchive& operator=(const FMemoryArchive&) = delete;
		FMemoryArchive& operator=(FMemoryArchive&&) = delete;
		FMemoryArchive(PHYSFS_Io* io, const char* name) : m_name{ name }, m_io{ io }
		{
			LogDebug(LOG, "Create tmpfs archive {}", name);
			m_directories.emplace_back("");
		}

		~FMemoryArchive()
		{
			LogDebug(LOG, "Destroy tmpfs archive {}", m_name);
			m_io->destroy(m_io);
		}

		static auto GetThis(void* opaque) -> FMemoryArchive*
		{
			return reinterpret_cast<FMemoryArchive*>(opaque); // NOLINT
		}
		static auto OpenArchive(PHYSFS_Io* io, const char* name, int /*forWrite*/, int* claimed) -> void*
		{
			*claimed = 1;
			return new FMemoryArchive(io, name);
		}

		static auto Enumerate(void* /*opaque*/,
			const char* /*dirname*/,
			PHYSFS_EnumerateCallback /*cb*/,
			const char* /*origdir*/,
			void* /*callbackdata*/) -> PHYSFS_EnumerateCallbackResult
		{
			NotImplemented("fsMem");
			return {};
		}
		static auto OpenRead(void* opaque, const char* fnm) -> PHYSFS_Io*
		{
			auto* m = GetThis(opaque);
			auto& files = m->m_files;
			auto f = files.find(fnm);
			if (f == std::end(files))
			{
				return nullptr;
			}
			return FPhysfsIoMemory::Create(fnm, &f->second);
		}

		static auto OpenWrite(void* opaque, const char* filename) -> PHYSFS_Io*
		{
			auto* m = GetThis(opaque);
			return FPhysfsIoMemory::Create(filename, &m->m_files[filename]);
		}

		static auto OpenAppend(void* /*opaque*/, const char* /*filename*/) -> PHYSFS_Io*
		{
			NotImplemented("fsMem");
			return {};
		}
		static auto Remove(void* /*opaque*/, const char* /*filename*/) -> int
		{
			NotImplemented("fsMem");
			return {};
		}
		static auto Mkdir(void* opaque, const char* filename) -> int
		{
			auto* m = GetThis(opaque);
			auto& directories = m->m_directories;
			auto it = std::ranges::find(directories, filename);
			if (it == std::end(directories))
			{
				directories.emplace_back(filename);
			}
			return 1;
		}
		static auto Stat(void* opaque, const char* fn, PHYSFS_Stat* stat) -> int
		{
			if (strcmp("", fn) == 0)
			{
				stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
				return 1;
			}
			auto* m = GetThis(opaque);
			if (std::end(m->m_directories) != std::ranges::find(m->m_directories, fn))
			{
				stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
				return 1;
			}
			if (std::end(m->m_files) != m->m_files.find(fn))
			{
				stat->filetype = PHYSFS_FILETYPE_REGULAR;
				return 1;
			}

			PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
			return 0;
		}
		static void CloseArchive(void* opaque) { delete GetThis(opaque); }
	};
	PHYSFS_Archiver s_memoryArchiver = {
		.version = 0,
		.info = { .extension = "memory",
				 .description = "Memory filesystem",
				 .author = "DK",
				 .url = "mem://",
				 .supportsSymlinks = 0 },
		.openArchive = FMemoryArchive::OpenArchive,
		.enumerate = FMemoryArchive::Enumerate,
		.openRead = FMemoryArchive::OpenRead,
		.openWrite = FMemoryArchive::OpenWrite,
		.openAppend = FMemoryArchive::OpenAppend,
		.remove = FMemoryArchive::Remove,
		.mkdir = FMemoryArchive::Mkdir,
		.stat = FMemoryArchive::Stat,
		.closeArchive = FMemoryArchive::CloseArchive
	};
} // namespace

FileUtility::FileUtility(const std::string_view rootPrefix) : RootPrefix{ rootPrefix }
{
	auto r = PhysFS_InitThreadSafe();
	if (r == 0U)
	{
		LogError(LOG, "FS init failed {}", PHYSFS_getLastErrorCode());
	}
	DEBUG_ASSERT(r);
	if (r == 1)
	{
		r = PHYSFS_registerArchiver(&s_memoryArchiver);
	}
	DEBUG_ASSERT(r);
}

FileUtility::~FileUtility()
{
	auto r = PhysFS_DeinitThreadSafe();
	DEBUG_ASSERT(r);
}

auto FileUtility::Mount(const std::string_view item, const std::string_view mount_point) -> bool
{
	auto mp = ApplyRootPrefix(mount_point);
	if (item == "tmpfs.memory"sv)
	{
		const auto fname = ApplyRootPrefix(item);
		auto* io = FPhysfsIoMemory::Create(mp);
		const auto e = PHYSFS_mountIo(io, fname.data(), mp.c_str(), 0);
		DEBUG_ASSERT(e != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return e != 0;
	}
	auto e = PHYSFS_mount(item.data(), mp.c_str(), 0);
	DEBUG_ASSERT(e != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	std::string_view nmp = PHYSFS_getMountPoint(item.data());
	DEBUG_ASSERT(nmp.size() >= mount_point.size());
	nmp.remove_suffix(mount_point.size());
	RootPrefix = nmp;
	return e != 0;
}

void FileUtility::Unmount(const std::string_view item)
{
	const auto fname = (item == "tmpfs.memory"sv) ? ApplyRootPrefix(item) : std::string(item);
	auto e = PHYSFS_unmount(fname.data());
	DEBUG_ASSERT(e != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
}

auto FileUtility::Open(const std::string_view path, FileUtility::FileMode fm) -> FileUtility::File*
{
	PHYSFS_File* f = nullptr;
	auto p = ApplyRootPrefix(path);
	switch (fm)
	{
		case FileMode::Read:
			f = PHYSFS_openRead(p.c_str());
			break;
		case FileMode::Write:
			f = PHYSFS_openWrite(p.c_str());
			break;
	}
	DEBUG_ASSERT(f != nullptr, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	return f;
}

auto FileUtility::Read(File* file, std::byte* data, const std::size_t size) -> std::size_t
{
	DEBUG_ASSERT(file);
	auto bytesRead = PHYSFS_readBytes(file, data, size);
	DEBUG_ASSERT(bytesRead > 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	DEBUG_ASSERT(std::cmp_equal(bytesRead, size));
	return bytesRead;
}

auto FileUtility::Write(File* file, const std::byte* data, const std::size_t size) -> bool
{
	DEBUG_ASSERT(file);
	auto bytesWritten = PHYSFS_writeBytes(file, data, size);
	DEBUG_ASSERT(bytesWritten > 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	DEBUG_ASSERT(std::cmp_equal(bytesWritten, size));
	return std::cmp_equal(bytesWritten, size);
}

auto FileUtility::Exists(const std::string_view path) -> bool
{
	return PHYSFS_exists(ApplyRootPrefix(path).c_str()) != 0;
}

auto FileUtility::Size(File* file) -> std::size_t
{
	return PHYSFS_fileLength(file);
}

auto FileUtility::Size(const std::string_view path) -> std::size_t
{
	DEBUG_ASSERT(Exists(path));
	auto* f = Open(path, FileMode::Read);
	auto size = Size(f);
	Close(f);
	return size;
}

void FileUtility::Close(File* file)
{
	auto r = PHYSFS_close(file);
	DEBUG_ASSERT(r != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
}

auto FileUtility::MkDir(const std::string_view dirPath) -> bool
{
	return PHYSFS_mkdir(ApplyRootPrefix(dirPath).c_str()) != 0;
}

auto FileUtility::Copy(const std::string_view src, const std::string_view dst) -> bool
{
	if (Exists(dst))
	{
		return false;
	}
	auto* input = Open(src, FileMode::Read);
	auto* output = Open(dst, FileMode::Write);
	auto inputFileSize = Size(input);
	std::vector<std::byte> buffer(inputFileSize);
	Read(input, buffer.data(), inputFileSize);
	Write(output, buffer.data(), inputFileSize);
	Close(output);
	Close(input);
	return true;
}

auto FileUtility::ReadFile(const std::string_view path) -> std::vector<std::uint8_t>
{
	DEBUG_ASSERT(not path.empty());
	LogDebug(LOG, "Loading file {}", path);
	std::vector<std::uint8_t> result;
	if (!Exists(path))
	{
		LogWarn(LOG, "File does not exist {}", path);
		return result;
	}
	auto* fh = Open(path, FileMode::Read);
	auto length = Size(fh);
	result.resize(length);
	auto read = Read(fh, reinterpret_cast<std::byte*>(result.data()), length);
	Close(fh);
	if (std::cmp_equal(read, -1))
	{
		LogError(LOG, "Error while reading data");
		result.resize(0);
	}
	return result;
}

#if __EMSCRIPTEN__
std::string FileUtility::GetBasePath()
{
	return "/game";
}
#else
auto FileUtility::GetBasePath() -> std::string
{
	return PHYSFS_getBaseDir();
}
#endif

auto FileUtility::GetPrefPath(const std::string_view postfix, const bool debugBuild) -> std::string
{
	std::string prefPath{ debugBuild ? PHYSFS_getBaseDir() : PHYSFS_getPrefDir("Soldat", "Soldat") };
	prefPath += postfix.data();
	if (!std::filesystem::exists(prefPath))
	{
		SoldatEnsure(std::filesystem::create_directories(prefPath));
	}
	DEBUG_ASSERT(std::filesystem::is_directory(prefPath));
	return prefPath;
}

// tests
#include <doctest/doctest.h>
#include <fstream>

namespace
{

	class FileUtilityFixture
	{
	public:
		FileUtilityFixture() = default;
		~FileUtilityFixture() = default;
		FileUtilityFixture& operator=(const FileUtilityFixture&) = delete;
		FileUtilityFixture&& operator=(const FileUtilityFixture&&) = delete;
		FileUtilityFixture(const FileUtilityFixture&) = delete;
		FileUtilityFixture(const FileUtilityFixture&&) = delete;
	};

	TEST_SUITE("FileUtility")
	{

		TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and write file and later read it")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
				auto r = FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
				auto d = std::to_array({ std::byte(0), std::byte(0), std::byte(0), std::byte(0) });
				auto r = FileUtility::Read(f, d.data(), d.size());
				FileUtility::Close(f);
				CHECK_EQ(d.size(), r);
				CHECK_EQ(d, testData);
			}
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system write and read file")
		{
			auto testDirectory = fs::temp_directory_path() / "Soldat_test";
			fs::remove_all(testDirectory);
			fs::create_directories(testDirectory);
			FileUtility fu;
			fu.Mount(testDirectory.string(), "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
				auto r = FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
				std::array<std::byte, testData.size()> d = {};
				std::ranges::fill(d, std::byte(0));
				auto r = FileUtility::Read(f, d.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(testData.size(), r);
				CHECK_EQ(d, testData);
			}
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "FileUtility initialized twice does not crash")
		{
			FileUtility const fu1;
			FileUtility const fu2;
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Create directory in memory")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, created);
			created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, created);
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Create two directories in memory")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, created);
			created = fu.MkDir("/fs_mem/test_directory/test_directory2");
			CHECK_EQ(true, created);
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Create directory in filesystem")
		{
			auto testDirectory = fs::temp_directory_path() / "Soldat_test";
			fs::remove_all(testDirectory);
			fs::create_directories(testDirectory);
			FileUtility fu;
			fu.Mount(testDirectory.string(), "/fs_mem");
			auto created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, created);
			created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, created);
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Get base path returns path to directory with with exe")
		{
			FileUtility const fu;
			auto s = FileUtility::GetBasePath();
			CHECK_NE("", s);
			CHECK_EQ(true, std::filesystem::is_directory(s));
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Get pref data returns path to directory with user settings")
		{
			FileUtility const fu;
			auto s = FileUtility::GetPrefPath("test_pref");
			std::filesystem::path p{ s };
			CHECK_EQ(p.filename().string(), std::string("test_pref"));
			CHECK_EQ(true, std::filesystem::is_directory(p));
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Exists return false if file does not exist")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
				auto r = FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
			CHECK_EQ(false, fu.Exists("/fs_mem/invalid"));
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Return size of file")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });

			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
				auto r = FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
				CHECK_EQ(4, fu.Size(f));
				FileUtility::Close(f);
			}
			{
				CHECK_EQ(4, fu.Size("/fs_mem/valid"));
			}
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Copy file")
		{
			FileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
				auto r = FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto copied = fu.Copy("/fs_mem/valid", "/fs_mem/copy");
				CHECK_EQ(true, copied);
				auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
				CHECK_EQ(4, fu.Size(f));
				FileUtility::Close(f);
			}
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
		{
			FileUtility fu("/test1");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
			auto r = FileUtility::Write(f, testData.data(), testData.size());
			CHECK(r);
			FileUtility::Close(f);
			FileUtility fu2("/test2");
			fu2.Mount("tmpfs.memory", "/fs_mem");

			{
				auto* f = fu2.Open("/fs_mem/valid2", FileUtility::FileMode::Write);
				FileUtility::Write(f, testData.data(), testData.size());
				FileUtility::Close(f);
			}

			CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
			CHECK_EQ(false, fu2.Exists("/fs_mem/valid"));
			CHECK_EQ(false, fu.Exists("/fs_mem/valid2"));
			CHECK_EQ(true, fu2.Exists("/fs_mem/valid2"));
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "The same file can be mounted twice in different objects")
		{
			// test soldat.smod, generated with xxd --include soldat.smod
			// contains:
			// client_test.txt
			// server_test.txt
			// shared_test.txt
			// NOLINTBEGIN
			// clang-format off
			unsigned char soldat_smod[] = {
				0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0x24, 0x33,
				0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x63, 0x6c,
				0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d,
				0x2e, 0x89, 0x4f, 0xce, 0xc9, 0x4c, 0xcd, 0x2b, 0xe1, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14,
				0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8, 0x12, 0xba, 0x0e, 0x00, 0x00,
				0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x5f,
				0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0x4e, 0x2d,
				0x2a, 0x4b, 0x2d, 0xe2, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00,
				0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
				0x0f, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e,
				0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0xce, 0x48, 0x2c, 0x4a, 0x4d, 0xe1, 0x02,
				0x00, 0x50, 0x4b, 0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21,
				0x00, 0x24, 0x33, 0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63,
				0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b,
				0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8,
				0x12, 0xba, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76,
				0x65, 0x72, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x01, 0x02, 0x14,
				0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e,
				0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f,
				0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
				0x03, 0x00, 0x03, 0x00, 0xb7, 0x00, 0x00, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00};
			unsigned int soldat_smod_len = 382;
			// clang-format on
			// NOLINTEND

			FileUtility fu("/t1");
			auto testDir = FileUtility::GetPrefPath("mount_test", true);
			std::filesystem::remove_all(testDir);
			// recreate directory
			testDir = FileUtility::GetPrefPath("mount_test", true);
			{
				std::ofstream s(testDir + "/soldat.smod", std::ios_base::binary | std::ios_base::trunc);
				s.write(reinterpret_cast<char*>(soldat_smod), soldat_smod_len);
			}

			CHECK_EQ(true, fu.Mount(testDir + "/soldat.smod", "/"));
			CHECK_EQ(true, fu.Exists("/client_test.txt"));
			FileUtility fu2("/t2");
			CHECK_EQ(true, fu2.Mount(testDir + "/soldat.smod", "/"));
			CHECK_EQ(true, fu2.Exists("/client_test.txt"));
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
		{
			FileUtility fu("/test8");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array<std::uint8_t>({ 42, 42, 42, 40 }); // NOLINT
			auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
			auto r = FileUtility::Write(f, reinterpret_cast<std::byte*>(testData.data()), testData.size());
			CHECK(r);
			FileUtility::Close(f);

			auto data = fu.ReadFile("/fs_mem/valid");
			CHECK_EQ(testData.size(), data.size());
			CHECK_EQ(testData[0], data[0]);
			CHECK_EQ(testData[1], data[1]);
			CHECK_EQ(testData[2], data[2]);
			CHECK_EQ(testData[3], data[3]);
		}

		TEST_CASE_FIXTURE(FileUtilityFixture, "Test size method" * doctest::skip(false))
		{
			FileUtility fu("/test1");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array<std::uint8_t>({ 42, 42, 42, 40 }); // NOLINT
			CHECK_FALSE(fu.Exists("/fs_mem/valid"));
			auto* f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
			auto r = FileUtility::Write(f, reinterpret_cast<std::byte*>(testData.data()), testData.size());
			CHECK(r);
			FileUtility::Close(f);
			auto size = fu.Size("/fs_mem/valid");
			CHECK_EQ(4, size);
		}

	} // TEST_SUITE(FileUtility)

} // namespace
