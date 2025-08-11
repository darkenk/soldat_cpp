#pragma once
#include "Sprites.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include <algorithm>
#include <array>

#ifndef SERVER
#include "../../client/Client.hpp"
#endif // SERVER

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

  auto GetSprite(const SpriteId id) -> TSprite &;

  std::vector<TSprite> &GetSprites()
  {
    return Sprites;
  }

  auto GetActiveSprites() -> TActiveSprites &;

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


  // NOLINTBEGIN(soldat-*)
  #ifndef SERVER
  TSprite& GetPlayerSprite() requires std::is_same_v<TSprite, Sprite<Config::CLIENT_MODULE>>
  {
    return GetSprite(gGlobalStateClient.mysprite);
  }

  bool IsPlayerSpriteValid() requires std::is_same_v<TSprite, Sprite<Config::CLIENT_MODULE>>
  {
    return gGlobalStateClient.mysprite > 0;
  }

  bool IsPlayerSprite(std::uint8_t spriteId) requires std::is_same_v<TSprite, Sprite<Config::CLIENT_MODULE>>
  {
    return spriteId == gGlobalStateClient.mysprite;
  }
  #endif // SERVER
  // NOLINTEND(soldat-*)

protected:
  TSpriteSystem();

private:
  std::vector<TSprite> Sprites;
  TActiveSprites ActiveSprites;
  particlesystem spriteparts;
  std::uint8_t mysprite;

  friend GlobalSubsystem<TSpriteSystem>;
};

using SpriteSystem = TSpriteSystem<>;
