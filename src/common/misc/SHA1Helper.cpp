#include "SHA1Helper.hpp"
#include "SHA1.hpp"
#include <algorithm>

auto sha1file(const std::string &file) -> tsha1digest
{
  auto sha1 = SHA1::from_file(file);
  tsha1digest ret;
  std::copy(sha1.begin(), sha1.end(), ret.Dummy.begin());
  return ret;
}
