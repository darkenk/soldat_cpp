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

constexpr auto kLog = "fs";

namespace
{
	struct FPhysfsIoMemory
	{
		FPhysfsIoMemory(const FPhysfsIoMemory&) = delete;
		FPhysfsIoMemory(FPhysfsIoMemory&&) = delete;
		FPhysfsIoMemory& operator=(const FPhysfsIoMemory&) = delete;
		FPhysfsIoMemory& operator=(FPhysfsIoMemory&&) = delete;
		using TFileContent = std::vector<std::byte>;

		static auto GetThis(PHYSFS_Io* io) -> FPhysfsIoMemory* { return static_cast<FPhysfsIoMemory*>(io->opaque); }

		static auto Create(const std::string& nodeName, TFileContent* content = nullptr) -> PHYSFS_Io*
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
			auto* Iom = GetThis(io);
			auto& FileContent = *Iom->MContent;
			auto& Pos = Iom->MPosition;
			if ((Pos + len) > FileContent.size())
			{
				len = FileContent.size() - Pos;
			}
			auto ReadStart = std::begin(FileContent) + Pos;
			std::copy(ReadStart, ReadStart + len, static_cast<std::byte*>(buf));
			Pos += len;
			return len;
		}

		static auto Write(struct PHYSFS_Io* io, const void* buffer, PHYSFS_uint64 len) -> PHYSFS_sint64
		{
			auto* Iom = GetThis(io);
			auto& FileContent = *Iom->MContent;
			auto& Pos = Iom->MPosition;
			auto NewSize = Pos + len;
			if (NewSize > FileContent.size())
			{
				FileContent.reserve(NewSize);
			}
			std::copy(static_cast<const std::byte*>(buffer),
				static_cast<const std::byte*>(buffer) + len,
				std::inserter(FileContent, FileContent.begin() + Pos));
			Pos += len;
			return len;
		}

		static auto Seek(struct PHYSFS_Io* io, PHYSFS_uint64 offset) -> int
		{
			auto* t = GetThis(io);
			t->MPosition = offset;
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
			return t->MContent->size();
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
		std::size_t MPosition = 0;
		std::string MNodeName;
		TFileContent* MContent = nullptr;
		bool MOwnsContent = false;

		FPhysfsIoMemory(std::string nodeName, TFileContent* content) : MNodeName(std::move(nodeName)), MContent(content)
		{
			if (MContent == nullptr)
			{
				MContent = new TFileContent();
				MOwnsContent = true;
			}
		}

		~FPhysfsIoMemory()
		{
			if (MOwnsContent)
			{
				delete MContent;
			}
		}
	};

	struct FMemoryArchive
	{
	public:
		FMemoryArchive(const FMemoryArchive&) = default;
		FMemoryArchive(FMemoryArchive&&) = delete;
		FMemoryArchive& operator=(const FMemoryArchive&) = delete;
		FMemoryArchive& operator=(FMemoryArchive&&) = delete;
		FMemoryArchive(PHYSFS_Io* io, const char* name) : Name{ name }, Io{ io }
		{
			LogDebug(kLog, "Create tmpfs archive {}", name);
			Directories.emplace_back("");
		}

		~FMemoryArchive()
		{
			LogDebug(kLog, "Destroy tmpfs archive {}", Name);
			Io->destroy(Io);
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
			auto& Files = m->Files;
			auto f = Files.find(fnm);
			if (f == std::end(Files))
			{
				return nullptr;
			}
			return FPhysfsIoMemory::Create(fnm, &f->second);
		}

		static auto OpenWrite(void* opaque, const char* filename) -> PHYSFS_Io*
		{
			auto* m = GetThis(opaque);
			return FPhysfsIoMemory::Create(filename, &m->Files[filename]);
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
			auto& Directories = m->Directories;
			auto it = std::ranges::find(Directories, filename);
			if (it == std::end(Directories))
			{
				Directories.emplace_back(filename);
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
			if (std::end(m->Directories) != std::ranges::find(m->Directories, fn))
			{
				stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
				return 1;
			}
			if (std::end(m->Files) != m->Files.find(fn))
			{
				stat->filetype = PHYSFS_FILETYPE_REGULAR;
				return 1;
			}

			PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
			return 0;
		}
		static void CloseArchive(void* opaque) { delete GetThis(opaque); }

	private:
		std::string Name;
		std::string MountPoint;
		PHYSFS_Io* Io{ nullptr };
		std::unordered_map<std::string, std::vector<std::byte>> Files;
		std::vector<std::string> Directories;
	};
	PHYSFS_Archiver gSMemoryArchiver = {
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

FFileUtility::FFileUtility(const std::string_view InRootPrefix) : RootPrefix{ InRootPrefix }
{
	auto r = PhysFS_InitThreadSafe();
	if (r == 0U)
	{
		LogError(kLog, "FS init failed {}", PHYSFS_getLastErrorCode());
	}
	DEBUG_ASSERT(r);
	if (r == 1)
	{
		r = PHYSFS_registerArchiver(&gSMemoryArchiver);
	}
	DEBUG_ASSERT(r);
}

FFileUtility::~FFileUtility()
{
	auto r = PhysFS_DeinitThreadSafe();
	DEBUG_ASSERT(r);
}

auto FFileUtility::Mount(const std::string_view InItem, const std::string_view InMountPoint) -> bool
{
	auto mp = ApplyRootPrefix(InMountPoint);
	if (InItem == "tmpfs.memory"sv)
	{
		const auto FNAME = ApplyRootPrefix(InItem);
		auto* io = FPhysfsIoMemory::Create(mp);
		const auto E = PHYSFS_mountIo(io, FNAME.data(), mp.c_str(), 0);
		DEBUG_ASSERT(E != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return E != 0;
	}
	auto e = PHYSFS_mount(std::string(InItem).c_str(), mp.c_str(), 0);
	DEBUG_ASSERT(e != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	std::string_view Nmp = PHYSFS_getMountPoint(std::string(InItem).c_str());
	DEBUG_ASSERT(Nmp.size() >= InMountPoint.size());
	Nmp.remove_suffix(InMountPoint.size());
	RootPrefix = Nmp;
	return e != 0;
}

void FFileUtility::Unmount(const std::string_view InItem)
{
	const auto Name = (InItem == "tmpfs.memory"sv) ? ApplyRootPrefix(InItem) : std::string(InItem);
	auto e = PHYSFS_unmount(Name.data());
	DEBUG_ASSERT(e != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
}

auto FFileUtility::Open(const std::string_view InPath, FFileUtility::EFileMode fm) -> FFileUtility::TFile*
{
	PHYSFS_File* f = nullptr; // NOLINT
	auto p = ApplyRootPrefix(InPath);
	switch (fm)
	{
		case EFileMode::Read:
			f = PHYSFS_openRead(p.c_str());
			break;
		case EFileMode::Write:
			f = PHYSFS_openWrite(p.c_str());
			break;
	}
	DEBUG_ASSERT(f != nullptr, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	return f;
}

auto FFileUtility::Read(TFile* file, std::byte* data, const std::size_t InSize) -> std::size_t
{
	DEBUG_ASSERT(file);
	auto BytesRead = PHYSFS_readBytes(file, data, InSize);
	DEBUG_ASSERT(BytesRead > 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	DEBUG_ASSERT(std::cmp_equal(BytesRead, InSize));
	return BytesRead;
}

auto FFileUtility::Write(TFile* file, const std::byte* data, const std::size_t InSize) -> bool
{
	DEBUG_ASSERT(file);
	auto BytesWritten = PHYSFS_writeBytes(file, data, InSize);
	DEBUG_ASSERT(BytesWritten > 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	DEBUG_ASSERT(std::cmp_equal(BytesWritten, InSize));
	return std::cmp_equal(BytesWritten, InSize);
}

auto FFileUtility::Exists(const std::string_view InPath) -> bool
{
	return PHYSFS_exists(ApplyRootPrefix(InPath).c_str()) != 0;
}

auto FFileUtility::Size(TFile* file) -> std::size_t
{
	return PHYSFS_fileLength(file);
}

auto FFileUtility::Size(const std::string_view InPath) -> std::size_t
{
	DEBUG_ASSERT(Exists(InPath));
	auto* f = Open(InPath, EFileMode::Read);
	auto FileSize = Size(f);
	Close(f);
	return FileSize;
}

void FFileUtility::Close(TFile* file)
{
	auto r = PHYSFS_close(file);
	DEBUG_ASSERT(r != 0, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
}

auto FFileUtility::MkDir(const std::string_view InDirPath) -> bool
{
	return PHYSFS_mkdir(ApplyRootPrefix(InDirPath).c_str()) != 0;
}

auto FFileUtility::Copy(const std::string_view InSrc, const std::string_view InDst) -> bool
{
	if (Exists(InDst))
	{
		return false;
	}
	auto* Input = Open(InSrc, EFileMode::Read);
	auto* Output = Open(InDst, EFileMode::Write);
	auto InputFileSize = Size(Input);
	std::vector<std::byte> Buffer(InputFileSize);
	Read(Input, Buffer.data(), InputFileSize);
	Write(Output, Buffer.data(), InputFileSize);
	Close(Output);
	Close(Input);
	return true;
}

auto FFileUtility::ReadFile(const std::string_view InPath) -> std::vector<std::uint8_t>
{
	DEBUG_ASSERT(not InPath.empty());
	LogDebug(kLog, "Loading file {}", InPath);
	std::vector<std::uint8_t> Result;
	if (!Exists(InPath))
	{
		LogWarn(kLog, "File does not exist {}", InPath);
		return Result;
	}
	auto* fh = Open(InPath, EFileMode::Read);
	auto Length = Size(fh);
	Result.resize(Length);
	auto BytesRead = Read(fh, reinterpret_cast<std::byte*>(Result.data()), Length);
	Close(fh);
	if (std::cmp_equal(BytesRead, -1))
	{
		LogError(kLog, "Error while reading data");
		Result.resize(0);
	}
	return Result;
}

#if __EMSCRIPTEN__
std::string FileUtility::GetBasePath()
{
	return "/game";
}
#else
auto FFileUtility::GetBasePath() -> std::string
{
	return PHYSFS_getBaseDir();
}
#endif

auto FFileUtility::GetPrefPath(const std::string_view InPostfix, const bool InDebugBuild) -> std::string
{
	std::string PrefPath{ InDebugBuild ? PHYSFS_getBaseDir() : PHYSFS_getPrefDir("Soldat", "Soldat") };
	PrefPath += InPostfix.data();
	if (!std::filesystem::exists(PrefPath))
	{
		SoldatEnsure(std::filesystem::create_directories(PrefPath));
	}
	DEBUG_ASSERT(std::filesystem::is_directory(PrefPath));
	return PrefPath;
}

// tests
#include <doctest/doctest.h>
#include <fstream>

namespace
{

	class FIleUtilityFixture
	{
	public:
		FIleUtilityFixture() = default;
		~FIleUtilityFixture() = default;
		FIleUtilityFixture& operator=(const FIleUtilityFixture&) = delete;
		FIleUtilityFixture&& operator=(const FIleUtilityFixture&&) = delete;
		FIleUtilityFixture(const FIleUtilityFixture&) = delete;
		FIleUtilityFixture(const FIleUtilityFixture&&) = delete;
	};

	TEST_SUITE("FileUtility")
	{

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Mount memory and write file and later read it")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
				auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Read);
				auto d = std::to_array({ std::byte(0), std::byte(0), std::byte(0), std::byte(0) });
				auto r = FFileUtility::Read(f, d.data(), d.size());
				FFileUtility::Close(f);
				CHECK_EQ(d.size(), r);
				CHECK_EQ(d, TestData);
			}
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Mount file system write and read file")
		{
			auto TestDirectory = fs::temp_directory_path() / "Soldat_test";
			fs::remove_all(TestDirectory);
			fs::create_directories(TestDirectory);
			FFileUtility fu;
			fu.Mount(TestDirectory.string(), "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
				auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Read);
				std::array<std::byte, TestData.size()> d = {};
				std::ranges::fill(d, std::byte(0));
				auto r = FFileUtility::Read(f, d.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(TestData.size(), r);
				CHECK_EQ(d, TestData);
			}
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "FileUtility initialized twice does not crash")
		{
			FFileUtility const FU1;
			FFileUtility const FU2;
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Create directory in memory")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto Created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, Created);
			Created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, Created);
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Create two directories in memory")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto Created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, Created);
			Created = fu.MkDir("/fs_mem/test_directory/test_directory2");
			CHECK_EQ(true, Created);
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Create directory in filesystem")
		{
			auto TestDirectory = fs::temp_directory_path() / "Soldat_test";
			fs::remove_all(TestDirectory);
			fs::create_directories(TestDirectory);
			FFileUtility fu;
			fu.Mount(TestDirectory.string(), "/fs_mem");
			auto Created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, Created);
			Created = fu.MkDir("/fs_mem/test_directory");
			CHECK_EQ(true, Created);
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Get base path returns path to directory with with exe")
		{
			FFileUtility const FU;
			auto s = FFileUtility::GetBasePath();
			CHECK_NE("", s);
			CHECK_EQ(true, std::filesystem::is_directory(s));
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Get pref data returns path to directory with user settings")
		{
			FFileUtility const FU;
			auto s = FFileUtility::GetPrefPath("test_pref");
			std::filesystem::path const P{ s };
			CHECK_EQ(P.filename().string(), std::string("test_pref"));
			CHECK_EQ(true, std::filesystem::is_directory(P));
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Exists return false if file does not exist")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
				auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
			CHECK_EQ(false, fu.Exists("/fs_mem/invalid"));
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Return size of file")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });

			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
				auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Read);
				CHECK_EQ(4, fu.Size(f));
				FFileUtility::Close(f);
			}
			{
				CHECK_EQ(4, fu.Size("/fs_mem/valid"));
			}
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Copy file")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			{
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
				auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
				CHECK_EQ(true, r);
			}
			{
				auto Copied = fu.Copy("/fs_mem/valid", "/fs_mem/copy");
				CHECK_EQ(true, Copied);
				auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Read);
				CHECK_EQ(4, fu.Size(f));
				FFileUtility::Close(f);
			}
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
		{
			FFileUtility fu("/test1");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto TestData = std::to_array({ std::byte(42), std::byte(42), std::byte(42), std::byte(40) });
			auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
			auto r = FFileUtility::Write(f, TestData.data(), TestData.size());
			CHECK(r);
			FFileUtility::Close(f);
			FFileUtility Fu2("/test2");
			Fu2.Mount("tmpfs.memory", "/fs_mem");

			{
				auto* f = Fu2.Open("/fs_mem/valid2", FFileUtility::EFileMode::Write);
				FFileUtility::Write(f, TestData.data(), TestData.size());
				FFileUtility::Close(f);
			}

			CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
			CHECK_EQ(false, Fu2.Exists("/fs_mem/valid"));
			CHECK_EQ(false, fu.Exists("/fs_mem/valid2"));
			CHECK_EQ(true, Fu2.Exists("/fs_mem/valid2"));
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "The same file can be mounted twice in different objects")
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

			FFileUtility fu("/t1");
			auto TestDir = FFileUtility::GetPrefPath("mount_test", true);
			std::filesystem::remove_all(TestDir);
			// recreate directory
			TestDir = FFileUtility::GetPrefPath("mount_test", true);
			{
				std::ofstream s(TestDir + "/soldat.smod", std::ios_base::binary | std::ios_base::trunc);
				s.write(reinterpret_cast<char*>(soldat_smod), soldat_smod_len);
			}

			CHECK_EQ(true, fu.Mount(TestDir + "/soldat.smod", "/"));
			CHECK_EQ(true, fu.Exists("/client_test.txt"));
			FFileUtility Fu2("/t2");
			CHECK_EQ(true, Fu2.Mount(TestDir + "/soldat.smod", "/"));
			CHECK_EQ(true, Fu2.Exists("/client_test.txt"));
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
		{
			FFileUtility fu("/test8");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array<std::uint8_t>({ 42, 42, 42, 40 }); // NOLINT
			auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
			auto r = FFileUtility::Write(f, reinterpret_cast<std::byte*>(testData.data()), testData.size());
			CHECK(r);
			FFileUtility::Close(f);

			auto Data = fu.ReadFile("/fs_mem/valid");
			CHECK_EQ(testData.size(), Data.size());
			CHECK_EQ(testData[0], Data[0]);
			CHECK_EQ(testData[1], Data[1]);
			CHECK_EQ(testData[2], Data[2]);
			CHECK_EQ(testData[3], Data[3]);
		}

		TEST_CASE_FIXTURE(FIleUtilityFixture, "Test size method" * doctest::skip(false))
		{
			FFileUtility fu("/test1");
			fu.Mount("tmpfs.memory", "/fs_mem");
			auto testData = std::to_array<std::uint8_t>({ 42, 42, 42, 40 }); // NOLINT
			CHECK_FALSE(fu.Exists("/fs_mem/valid"));
			auto* f = fu.Open("/fs_mem/valid", FFileUtility::EFileMode::Write);
			auto r = FFileUtility::Write(f, reinterpret_cast<std::byte*>(testData.data()), testData.size());
			CHECK(r);
			FFileUtility::Close(f);
			auto Size = fu.Size("/fs_mem/valid");
			CHECK_EQ(4, Size);
		}

	} // TEST_SUITE(FileUtility)

} // namespace
