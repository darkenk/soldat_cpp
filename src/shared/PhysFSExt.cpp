#include "PhysFSExt.hpp"
#include "Logging.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "misc/TStream.hpp"
#include <filesystem>

auto LOG = "physfs";

class PhysFSStream : public TStream
{
  public:
    explicit PhysFSStream(const std::string_view &filename)
    {
        Handle = PHYSFS_openRead(filename.data());
        Assert(Handle != nullptr);
    }

    ~PhysFSStream()
    {
        auto errorCode = PHYSFS_close(Handle);
        Assert(errorCode != 0);
    }

    bool ReadLine(std::string &out) override
    {
        if (PHYSFS_eof(Handle))
        {
            return false;
        }
        PhysFS_ReadLn(Handle, out);
        return true;
    }

    void Reset() override
    {
        PHYSFS_seek(Handle, 0);
    }

  private:
    PHYSFS_File *Handle;
};

void PhysFS_ReadLn(PHYSFS_File *fileHandle, std::string &line)
{
    char c;
    line = "";
    while (PHYSFS_readBytes(fileHandle, &c, 1) && c != '\n')
    {
        if (c != '\r')
        {
            line += c;
        }
    }
}

PhysFS_Buffer PhysFS_readBuffer(const std::string_view &name)
{
    Assert(not name.empty());
    LogDebug(LOG, "Loading file {}", name);
    PhysFS_Buffer result;
    if (not PHYSFS_exists(name.data()))
    {
        LogWarn(LOG, "File does not exist {}", name);
        return result;
    }
    auto FileHandle = PHYSFS_openRead(name.data());
    if (not FileHandle)
    {
        LogError(LOG, "Cannot open file %s", name);
        return result;
    }
    auto length = PHYSFS_fileLength(FileHandle);
    result.resize(length);
    auto read = PHYSFS_readBytes(FileHandle, result.data(), length);
    PHYSFS_close(FileHandle);
    if (read == -1)
    {
        LogError(LOG, "Error while reading data");
        result.resize(0);
    }
    return result;
}

bool PhysFS_CopyFileFromArchive(const std::string_view &sourceFile,
                                const std::string_view &destination)
{
    if (std::filesystem::exists(destination.data()))
    {
        return false;
    }
    auto inputFile = PHYSFS_openRead(sourceFile.data());
    auto outputFile = std::fopen(destination.data(), "wb");
    if (inputFile && outputFile)
    {
        std::array<std::byte, 1024> data;
        while (PHYSFS_eof(inputFile) == 0)
        {
            auto dataRead = PHYSFS_readBytes(inputFile, data.data(), data.size());
            std::fwrite(data.data(), dataRead, 1, outputFile);
        }
    }
    std::fclose(outputFile);
    PHYSFS_close(inputFile);
    return true;
}

std::unique_ptr<TStream> PhysFS_ReadAsStream(const std::string_view &file)
{
    if (not PHYSFS_exists(file.data()))
    {
        return nullptr;
    }
    auto ret = std::make_unique<PhysFSStream>(file);
    return ret;
}
