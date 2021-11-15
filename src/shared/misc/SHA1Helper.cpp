#include "SHA1Helper.hpp"
#include "SHA1.hpp"
#include <algorithm>

tsha1digest sha1file(const std::string &file)
{
    auto sha1 = SHA1::from_file(file);
    tsha1digest ret;
    std::copy(sha1.begin(), sha1.end(), ret.Dummy.begin());
    return ret;
}
