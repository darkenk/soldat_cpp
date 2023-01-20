#pragma once

#include <memory>
#include <vector>


class FileUtility
{
public:
    struct File;
    FileUtility();
    ~FileUtility();

    void Mount(const std::string_view item, const std::string_view mount_point);
    File& Open(const std::string_view path);
    std::size_t Read(File& file);
    void Close(File& file);

    bool Write(File &file, const std::byte *data, const std::size_t size);
private:
    class FileUtilityPIMPL;
    std::unique_ptr<FileUtilityPIMPL> PIMPL;
};
