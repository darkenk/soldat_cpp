#pragma once

#include <type_traits>
#include <utility>

template <typename T>
struct EnableFlagSet
{
    static constexpr bool enable = false;
};

template <typename T>
class FlagSet
{
    using EnumStorageType = typename std::underlying_type<T>::type;

  public:
    FlagSet() : FlagStorage{}
    {
    }

    FlagSet(const T &flag) : FlagStorage(static_cast<EnumStorageType>(flag))
    {
    }

    void Set(const T &flag)
    {
        FlagStorage |= static_cast<EnumStorageType>(flag);
    }

    bool Test(const T &flag) const
    {
        return FlagStorage & static_cast<EnumStorageType>(flag);
    }

    template <typename E>
    friend FlagSet<E> operator&(FlagSet<E> &left, const E &right);

  private:
    EnumStorageType FlagStorage;
};

template <typename T>
static FlagSet<T> &&operator|(FlagSet<T> &&ref, const T &right)
{
    ref.Set(right);
    return std::move(ref);
}

template <typename T>
FlagSet<typename std::enable_if<EnableFlagSet<T>::enable, T>::type> operator|(const T &left,
                                                                                const T &right)
{
    FlagSet<T> flags{left};
    flags.Set(right);
    return flags;
}

template <typename T>
FlagSet<T> operator&(FlagSet<T> &left, const T &right)
{
    FlagSet<T> flags(left);
    flags.FlagStorage &= static_cast<typename FlagSet<T>::EnumStorageType>(right);
    return flags;
}
