#pragma once

#include <boost/di.hpp>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "misc/Config.hpp"

struct PHYSFS_File;

class FFileUtility
{
public:
	enum class EFileMode : std::uint8_t
	{
		Read,
		Write
	};

	using TFile = PHYSFS_File;

	BOOST_DI_INJECT(FFileUtility, (named = std::string("FileRootPrefix")) std::string_view InRootPrefix = "");
	FFileUtility(const FFileUtility&) = delete;
	FFileUtility(FFileUtility&&) = delete;
	FFileUtility& operator=(const FFileUtility&) = delete;
	FFileUtility& operator=(FFileUtility&&) = delete;
	~FFileUtility();

	bool Mount(std::string_view item, std::string_view InMountPoint);
	void Unmount(std::string_view item);
	TFile* Open(std::string_view path, EFileMode fm);
	static void Close(TFile* file);
	static std::size_t Read(TFile* file, std::byte* data, std::size_t size);
	static bool Write(TFile* file, const std::byte* data, std::size_t size);
	bool Exists(std::string_view path);
	static std::size_t Size(TFile* file);
	std::size_t Size(std::string_view path);
	bool MkDir(std::string_view InDirPath);
	bool Copy(std::string_view src, std::string_view dst);

	std::vector<std::uint8_t> ReadFile(std::string_view path);

	static std::string GetBasePath();
	static std::string GetPrefPath(std::string_view InPostfix, bool InDebugBuild = Config::IsDebug());

private:
	std::string RootPrefix;

	auto ApplyRootPrefix(const std::string_view InPath) const
	{
		return RootPrefix + (InPath.size() == 0 || InPath[0] != '/' ? "/" : "") + std::string(InPath);
	}
};
