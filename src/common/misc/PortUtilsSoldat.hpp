#pragma once

#include "PortUtils.hpp"
#include "RandomGenerator.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <spdlog/fmt/fmt.h>
#include <string>
#include <utility>
#include <vector>

// Specific for SOLDAT
class pchar final
{
  public:
    pchar(const std::string &ref) : ref(ref)
    {
    }
    operator const char *()
    {
        return ref.c_str();
    }

  private:
    const std::string &ref;
};

template <typename Tp, std::int32_t StartIndex, std::int32_t EndIndex>
// FIXME disable deprecation for now
// struct [[deprecated("PascalArray can start from -1")]] PascalArray
struct PascalArray
{
  public:
    template <typename T>
    struct SourceLocation
    {
        constexpr SourceLocation(const T &v,
                                 const source_location &location = source_location::current())
            : Value{v}, Location{location}
        {
        }
        T Value;
        const source_location &Location;
    };

    inline Tp &operator[](SourceLocation<std::int32_t> n) noexcept
    {
        AssertL(n.Value >= StartIndex, n.Location);
        AssertL(n.Value <= EndIndex, n.Location);
        return m_data[n.Value - StartIndex];
    }

    inline const Tp &operator[](SourceLocation<std::int32_t> n) const noexcept
    {
        AssertL(n.Value >= StartIndex, n.Location);
        AssertL(n.Value <= EndIndex, n.Location);
        return m_data[n.Value - StartIndex];
    }

    constexpr std::size_t size() const
    {
        return m_data.size();
    }

    constexpr std::int32_t StartIdx() const
    {
        return StartIndex;
    }

    constexpr std::int32_t EndIdx() const
    {
        return EndIndex;
    }

    auto begin() noexcept
    {
        return m_data.begin();
    }

    auto end() noexcept
    {
        return m_data.end();
    }


    auto begin() const noexcept
    {
        return m_data.begin();
    }

    auto end() const noexcept
    {
        return m_data.end();
    }

  private:
    std::array<Tp, EndIndex - StartIndex + 1> m_data;
};

static inline float strtofloat(const std::string &s)
{
    return std::stof(s);
}

static inline std::int32_t strtointdef(const std::string &s, std::int32_t def)
{
    try
    {
        return std::stoi(s);
    }
    catch (const std::invalid_argument &a)
    {
        return def;
    }
}

static inline float strtofloatdef(const std::string &s, float def)
{
    try
    {
        return std::stof(s);
    }
    catch (const std::invalid_argument &a)
    {
        return def;
    }
}

static inline char chr(std::int32_t v)
{
    return (char)(v);
}

static inline std::string inttostr(std::int32_t v)
{
    return std::to_string(v);
}

static inline std::int32_t strtoint(const std::string &v)
{
    return std::stoi(v);
}

template <typename T>
inline std::size_t length(const T &v)
{
    return v.size();
}

template <typename T>
inline T sqr(T v)
{
    return v * v;
}

template <typename T>
inline bool assigned(const T *v)
{
    return v != nullptr;
}

template <typename T>
inline bool assigned(T *v)
{
    return v != nullptr;
}

template <typename T>
inline void freeandnullptr(T *&v)
{
    delete v;
    v = nullptr;
}

static inline std::string trim(std::string s)
{
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
    return s;
}

static inline std::string lowercase(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

template <typename T>
static inline std::int32_t low(const T &v)
{
    return 0;
}

template <typename T>
static inline std::int32_t high(const T &v)
{
    return v.size() - 1;
}

template <typename T, std::int32_t StartIndex, std::int32_t EndIndex>
static inline std::int32_t low(const PascalArray<T, StartIndex, EndIndex> &v)
{
    return v.StartIdx();
}

template <typename T, std::int32_t StartIndex, std::int32_t EndIndex>
static inline std::int32_t high(const PascalArray<T, StartIndex, EndIndex> &v)
{
    return v.EndIdx();
}

inline void sleep(std::int32_t t)
{
    NotImplemented(NITag::OTHER);
}

class tstringlist : public std::vector<std::string>
{
  public:
    void add(const std::string &v)
    {
        push_back(v);
    }
    void savetofile(const std::string &filename)
    {
        NotImplemented(NITag::FILESYSTEM, "lack of save for tstringlist");
    }
    void loadfromfile(const std::string &filename)
    {
        NotImplemented(NITag::FILESYSTEM, "lack of load for tstringlist");
    }
};

static inline void fillchar(void *s, std::size_t count, int c)
{
    std::memset(s, c, count);
}

template <typename T>
static inline T min(const T &v1, const T &v2)
{
    return std::min(v1, v2);
}

template <typename T>
static inline T max(const T &v1, const T &v2)
{
    return std::max(v1, v2);
}

template <typename T>
static inline void setlength(std::vector<T> &array, std::size_t size)
{
    array.resize(size);
}

template <typename T>
static inline std::int32_t ord(T ref)
{
    return std::int32_t(ref);
}

static inline std::string formatfloat(const std::string &format, float f)
{
    NotImplemented(NITag::OTHER);
    return std::to_string(f);
}

static inline float degtorad(float degree_angle)
{
    return degree_angle / 180.f;
}

static inline float power(float base, float exp)
{
    return powf(base, exp);
}

template <typename... Args>
static inline std::string wideformat(const std::string_view fmt, Args &&...args)
{
    return fmt::vformat(std::locale("en_US.UTF-8"), std::string_view(fmt),
                        fmt::make_format_args(args...));
}

enum DummyPort
{
    fffixed
};

static inline std::string floattostrf(float p, DummyPort, std::int32_t base, std::int32_t exp)
{
    NotImplemented(NITag::OTHER, "no floattostrf");
    return std::to_string(p);
}

template <typename T>
static inline bool isInRange(const T &value, const T &min, const T &max)
{
    return value >= min && value <= max;
}

template <typename T>
static inline std::int32_t sign(const T &v1)
{
    if (v1 == 0.0)
    {
        return 0;
    }
    if (v1 < 0.0)
    {
        return -1;
    }
    return 1;
}

static inline bool fileexists(const std::string &path)
{
    NotImplemented(NITag::OTHER);
    return false;
}
