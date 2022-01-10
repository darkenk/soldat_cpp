#pragma once
#include "common/misc/GlobalSubsystem.hpp"
#include "Sprites.hpp"
#include <algorithm>
#include <array>

template<class TSprite = tsprite>
class TSpriteSystem : public GlobalSubsystem<TSpriteSystem<TSprite>>
{
  public:
    using ActiveSpritesStorage = PascalArray<TSprite, 1, max_sprites>;
    class TActiveSprites
    {
        using InternalIterator = typename std::array<TSprite, max_sprites>::iterator;
      public:
        class Iterator;
        using value_type        = TSprite;
        using const_iterator    = Iterator;

        class Iterator
        {
          public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = TSprite;
            using pointer           = TSprite*;
            using reference         = TSprite&;

            Iterator& operator++()
            {
                do
                {
                    Iter++;
                } while(!(*Iter).IsActive() && Iter != End);
                return *this;
            }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.Iter == b.Iter; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.Iter != b.Iter; };
            reference operator*() const {return *Iter;}
          private:
            friend class TActiveSprites;
            explicit Iterator(InternalIterator currIter, InternalIterator end): Iter{currIter}, End{end} {
                while (!(*Iter).IsActive() && Iter != End)
                {
                    Iter++;
                }
            }
            InternalIterator Iter;
            InternalIterator End;
        };

        explicit TActiveSprites(ActiveSpritesStorage& container):Sprites{container}{}

        constexpr Iterator begin() const noexcept { return Iterator(Sprites.begin(), Sprites.end());};
        constexpr Iterator end() const noexcept { return Iterator(Sprites.end(), Sprites.end());};
      private:
        ActiveSpritesStorage& Sprites;
    };

    TSprite& GetSprite(std::int32_t i);

    PascalArray<TSprite, 1, max_sprites>& GetSprites() { return Sprites;}

    TActiveSprites& GetActiveSprites();


    tvector2& GetSpritePartsPos(std::int32_t spriteId) { return spriteparts.pos[spriteId]; }
    tvector2& GetSpritePartsOldPos(std::int32_t spriteId) { return spriteparts.pos[spriteId]; }
    void SetSpritePartsOldPos(std::int32_t spriteId, const tvector2& pos) { spriteparts.oldpos[spriteId] = pos; }
    tvector2& GetVelocity(std::int32_t spriteId) { return spriteparts.velocity[spriteId]; }
    tvector2& GetForces(std::int32_t spriteId) { return spriteparts.forces[spriteId]; }
    void CreateSpritePart(const tvector2& start, tvector2& vel, const float mass, const std::int32_t num);
    void DestroySpritePart(const std::int32_t spriteId) { spriteparts.active[spriteId] = false; }

    void ResetSpriteParts();

    void UpdateSpriteParts();

  protected:
    TSpriteSystem();

  private:
    PascalArray<TSprite, 1, max_sprites> Sprites;
    TActiveSprites ActiveSprites;
    particlesystem spriteparts;

    friend GlobalSubsystem<TSpriteSystem>;
};

using SpriteSystem = TSpriteSystem<>;
