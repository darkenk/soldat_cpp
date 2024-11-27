#pragma once
#include "Sprites.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include <algorithm>
#include <array>

template <class TSprite = tsprite>
class TSpriteSystem;

class SpriteId
{
public:
  constexpr SpriteId(const SpriteId &ref) : Id{ref.Id}
  {
  }

  constexpr SpriteId(const std::uint8_t id) : Id{id}
  {
  }

  static constexpr SpriteId Invalid()
  {
    return SpriteId{255};
  }

  constexpr bool operator==(const SpriteId &ref) const {
    return ref.Id == Id;
  }

private:
  const std::uint8_t Id;

  [[nodiscard]] auto GetId() const -> decltype(Id)
  {
    return Id;
  }
  friend class TSpriteSystem<>;
};

template <class TSprite>
class TSpriteSystem : public GlobalSubsystem<TSpriteSystem<TSprite>>
{
public:
  using ActiveSpritesStorage = std::vector<TSprite>;
  class TActiveSprites
  {
    using InternalIterator = typename ActiveSpritesStorage::iterator;

  public:
    class Iterator;
    using value_type = TSprite;
    using const_iterator = Iterator;

    class Iterator
    {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = TSprite;
      using pointer = TSprite *;
      using reference = TSprite &;

      Iterator &operator++()
      {
        do
        {
          Iter++;
        } while (Iter != End && !(*Iter).IsActive());
        return *this;
      }
      friend bool operator==(const Iterator &a, const Iterator &b)
      {
        return a.Iter == b.Iter;
      };
      friend bool operator!=(const Iterator &a, const Iterator &b)
      {
        return a.Iter != b.Iter;
      };
      reference operator*() const
      {
        return *Iter;
      }

    private:
      friend class TActiveSprites;
      explicit Iterator(InternalIterator currIter, InternalIterator end) : Iter{currIter}, End{end}
      {

        while (Iter != End && !(*Iter).IsActive())
        {
          Iter++;
        }
      }
      InternalIterator Iter;
      InternalIterator End;
    };

    explicit TActiveSprites(ActiveSpritesStorage &container) : Sprites{container}
    {
    }

    constexpr Iterator begin() const noexcept
    {
      return Iterator(Sprites.begin(), Sprites.end());
    };
    constexpr Iterator end() const noexcept
    {
      return Iterator(Sprites.end(), Sprites.end());
    };

  private:
    ActiveSpritesStorage &Sprites;
  };

  auto CreateSprite(const SpriteId reuseSpriteId = SpriteId::Invalid()) -> TSprite &;

  TSprite &GetSprite(const SpriteId id);

  std::vector<TSprite> &GetSprites()
  {
    return Sprites;
  }

  TActiveSprites &GetActiveSprites();

  tvector2 &GetSpritePartsPos(std::int32_t spriteId)
  {
    return spriteparts.pos[spriteId];
  }
  tvector2 &GetSpritePartsOldPos(std::int32_t spriteId)
  {
    return spriteparts.pos[spriteId];
  }
  void SetSpritePartsOldPos(std::int32_t spriteId, const tvector2 &pos)
  {
    spriteparts.oldpos[spriteId] = pos;
  }
  tvector2 &GetVelocity(std::int32_t spriteId)
  {
    return spriteparts.velocity[spriteId];
  }
  tvector2 &GetForces(std::int32_t spriteId)
  {
    return spriteparts.forces[spriteId];
  }
  void CreateSpritePart(const tvector2 &start, const tvector2 &vel, const float mass,
                        const std::int32_t num);
  void DestroySpritePart(const std::int32_t spriteId)
  {
    spriteparts.active[spriteId] = false;
  }

  void ResetSpriteParts();

  void UpdateSpriteParts();

protected:
  TSpriteSystem();

private:
  std::vector<TSprite> Sprites;
  TActiveSprites ActiveSprites;
  particlesystem spriteparts;

  friend GlobalSubsystem<TSpriteSystem>;
};

using SpriteSystem = TSpriteSystem<>;
