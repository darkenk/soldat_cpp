// automatically converted
#include "NetworkServerThing.hpp"
#include "../../server/Server.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "common/Calc.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

#ifdef SERVER
void serverthingsnapshot(std::uint8_t tonum)
{
  tmsg_serverthingsnapshot thingmsg;
  std::int32_t i, j;
  bool send;
  auto &things = GS::GetThingSystem().GetThings();

  for (i = 1; i <= max_things; i++)
  {
    auto &thing = things[i];
    if ((thing.active) && (thing.style != object_parachute) &&
        ((!thing.statictype) or
         ((thing.style < object_ussocom) || (thing.style == object_stationary_gun))) &&
        ((GS::GetGame().pointvisible(thing.skeleton.pos[1].x, thing.skeleton.pos[1].y, tonum)) or
         (thing.style < object_ussocom)))
    {
      thingmsg.header.id = msgid_serverthingsnapshot;
      // assign thing values to ThingMsg
      thingmsg.num = i;
      for (j = 1; j <= 4; j++)
      {
        [[deprecated("indexing")]] auto jminus1 = j - 1;
        thingmsg.pos[jminus1].x = thing.skeleton.pos[j].x;
        thingmsg.pos[jminus1].y = thing.skeleton.pos[j].y;
        thingmsg.oldpos[jminus1].x = thing.skeleton.oldpos[j].x;
        thingmsg.oldpos[jminus1].y = thing.skeleton.oldpos[j].y;
      }

      thingmsg.owner = thing.owner;
      thingmsg.style = thing.style;
      thingmsg.holdingsprite = thing.holdingsprite;

      // send only if moving
      send = false;
      if ((distance(thingmsg.pos[0].x, thingmsg.pos[0].y, thingmsg.oldpos[0].x,
                    thingmsg.oldpos[0].y) > minmovedelta) ||
          (distance(thingmsg.pos[1].x, thingmsg.pos[1].y, thingmsg.oldpos[1].x,
                    thingmsg.oldpos[1].y) > minmovedelta))
        send = true;

      if (GS::GetGame().GetMainTickCounter() % 2 == 0)
      {
        if (thing.style < object_ussocom)
          send = true;
        if (thing.style == object_rambo_bow)
          send = true;
      }

      if (send)
      {
        GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                                     SpriteSystem::Get().GetSprite(tonum).player->peer,
                                     k_nSteamNetworkingSend_Unreliable);
      }
    }
  }
}

void serverthingmustsnapshot(const std::uint8_t i)
{
  tmsg_serverthingmustsnapshot thingmsg;
  std::int32_t j;

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  if (thing.style == object_parachute)
    return;

  thingmsg.header.id = msgid_serverthingmustsnapshot;
  // assign thing values to ThingMsg
  thingmsg.num = i;
  for (j = 1; j <= 4; j++)
  {
    thingmsg.pos[j - 1].x = thing.skeleton.pos[j].x;
    thingmsg.pos[j - 1].y = thing.skeleton.pos[j].y;
    thingmsg.oldpos[j - 1].x = thing.skeleton.oldpos[j].x;
    thingmsg.oldpos[j - 1].y = thing.skeleton.oldpos[j].y;
  }
  thingmsg.timeout = thing.timeout;
  if (thing.timeout < 1)
    thingmsg.timeout = 1;
  thingmsg.owner = thing.owner;
  thingmsg.style = thing.style;
  thingmsg.holdingsprite = thing.holdingsprite;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite.player->peer,
                                   k_nSteamNetworkingSend_Unreliable);
    }
  }
}
#endif

void serverthingmustsnapshotonconnect(const std::uint8_t tonum)
{
  tmsg_serverthingmustsnapshot thingmsg;
  std::int32_t i, j;

  auto &things = GS::GetThingSystem().GetThings();
  for (i = 1; i <= max_things; i++)
  {
    auto &thing = things[i];
    if (thing.active)
      if (((thing.style < object_ussocom) || (thing.style > object_minigun)) &&
          (thing.style != object_parachute))
      {
        thingmsg.header.id = msgid_serverthingmustsnapshot;
        // assign thing values to ThingMsg
        thingmsg.num = i;
        for (j = 1; j <= 4; j++)
        {
          thingmsg.pos[j - 1].x = thing.skeleton.pos[j].x;
          thingmsg.pos[j - 1].y = thing.skeleton.pos[j].y;
          thingmsg.oldpos[j - 1].x = thing.skeleton.oldpos[j].x;
          thingmsg.oldpos[j - 1].y = thing.skeleton.oldpos[j].y;
        }
        thingmsg.timeout = std::int16_t(thing.timeout);
        if (thing.timeout < 1)
          thingmsg.timeout = 1;
        thingmsg.owner = thing.owner;
        thingmsg.style = thing.style;
        thingmsg.holdingsprite = thing.holdingsprite;

#ifdef SERVER
        GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                                     SpriteSystem::Get().GetSprite(tonum).player->peer,
                                     k_nSteamNetworkingSend_Unreliable);
#else
        GS::GetDemoRecorder().saverecord(thingmsg, sizeof(thingmsg));
#endif
      }
  }
}

#ifdef SERVER
void serverthingmustsnapshotonconnectto(const std::uint8_t i, const std::uint8_t tonum)
{
  tmsg_serverthingmustsnapshot thingmsg;
  std::int32_t j;

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  if (thing.style == object_parachute)
    return;

  thingmsg.header.id = msgid_serverthingmustsnapshot;
  // assign thing values to ThingMsg
  thingmsg.num = i;
  for (j = 1; j <= 4; j++)
  {
    thingmsg.pos[j].x = thing.skeleton.pos[j].x;
    thingmsg.pos[j].y = thing.skeleton.pos[j].y;
    thingmsg.oldpos[j].x = thing.skeleton.oldpos[j].x;
    thingmsg.oldpos[j].y = thing.skeleton.oldpos[j].y;
  }
  thingmsg.timeout = thing.timeout;
  if (thing.timeout < 1)
    thingmsg.timeout = 1;
  thingmsg.owner = thing.owner;
  thingmsg.style = thing.style;
  thingmsg.holdingsprite = thing.holdingsprite;

  GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                               SpriteSystem::Get().GetSprite(tonum).player->peer,
                               k_nSteamNetworkingSend_Unreliable);
}

void serverthingtaken(const std::uint8_t i, const std::uint8_t w)
{
  tmsg_serverthingtaken thingmsg;

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  thingmsg.header.id = msgid_thingtaken;
  thingmsg.num = thing.num;
  thingmsg.who = w;

  thingmsg.style = thing.style;
  thingmsg.ammocount = thing.ammocount;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite.player->peer,
                                   k_nSteamNetworkingSend_Unreliable);
    }
  }
}

void serverhandlerequestthing(SteamNetworkingMessage_t *netmessage)
{
  pmsg_requestthing msg;
  tplayer *player;

  if (!verifypacket(sizeof(tmsg_requestthing), netmessage->m_cbSize, msgid_requestthing))
    return;
  msg = pmsg_requestthing(netmessage->m_pData);
  player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
  serverthingmustsnapshotonconnectto(msg->thingid, player->spritenum);
}
#endif
