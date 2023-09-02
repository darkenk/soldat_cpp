// automatically converted
#include "MapFile.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "port_utils/NotImplemented.hpp"
#include <cassert>

/******************************************************************************/
/*                              Helper functions                              */
/******************************************************************************/

struct tfilebuffer
{
  std::vector<std::uint8_t> data;
  std::int32_t pos = 0;
};

const std::array<std::uint32_t, 256> crctable = {
  {0,          0x4c11db7,  0x9823b6e,  0xd4326d9,  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
   0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
   0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
   0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
   0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
   0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
   0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
   0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
   0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
   0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x18aeb13,  0x54bf6a4,  0x808d07d,  0xcc9cdca,
   0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
   0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
   0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
   0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
   0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
   0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
   0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
   0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
   0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
   0x315d626,  0x7d4cb91,  0xa97ed48,  0xe56f0ff,  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
   0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
   0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
   0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
   0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
   0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
   0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
   0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x29f3d35,  0x65e2082,  0xb1d065b,  0xfdc1bec,
   0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
   0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
   0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
   0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
   0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4}};

std::uint32_t crc32(std::uint32_t crc, std::uint8_t *data, std::int32_t len)
{
  std::uint32_t result;
  result = crc;
  while (len > 0)
  {
    result = crctable[*data ^ (((std::uint32_t)result >> 24) & 0xff)] ^ (result << 8);
    data += 1;
    len -= 1;
  }
  return result;
}

bool readallbytes(FileUtility &fs, const tmapinfo &map, tfilebuffer &buffer)
{
  bool result = false;

  if (map.name == "")
    return result;

  // Load default map from base archive
  auto mapFile = std::string("/maps/") + map.name + ".pms";
  if (fs.Exists(mapFile))
  {
    buffer.data = fs.ReadFile(mapFile);
  }
  else
  {
    NotImplemented("map", "Missing code for map loading");
    // Unmount previous map
    PHYSFS_unmount("/current_map");
    // Mount new map
    if (not PHYSFS_mount(map.path.c_str(), "/current_map", true))
    {
      return result;
    }
    // Read PMS file
    buffer.data = fs.ReadFile(std::string("/current_map/maps/") + map.mapname + ".pms");
  }

  if (buffer.data.size() == 0)
  {
    return result;
  }

  buffer.pos = 0;
  result = true;
  return result;
}

void bufferread(tfilebuffer &bf, void *dest, std::int32_t size)
{
  std::memset(dest, 0, size);
  assert(bf.pos + size <= bf.data.size());
  std::memcpy(dest, &bf.data[bf.pos], size);
  bf.pos += size;
}

std::uint8_t readuint8(tfilebuffer &bf)
{
  std::uint8_t readuint8_result;
  bufferread(bf, &readuint8_result, 1);
  return readuint8_result;
}

std::uint16_t readuint16(tfilebuffer &bf)
{
  std::uint16_t readuint16_result;
  bufferread(bf, &readuint16_result, 2);
  return readuint16_result;
}

std::int32_t readint32(tfilebuffer &bf)
{
  std::int32_t readint32_result;
  bufferread(bf, &readint32_result, 4);
  return readint32_result;
}

float readfloat(tfilebuffer &bf)
{
  float readfloat_result;
  bufferread(bf, &readfloat_result, 4);
  return readfloat_result;
}

std::string readstring(tfilebuffer &bf, std::int32_t maxsize)
{
  std::int32_t n;
  std::array<char, 128> s;

  std::string readstring_result = "";
  n = readuint8(bf);

  if ((n < s.size()) && (n <= maxsize))
  {
    bufferread(bf, &s[0], maxsize);
    s[n] = '\0';
    readstring_result = &s[0];
  }
  else
  {
    bf.pos += maxsize;
  }
  return readstring_result;
}

tvector3 readvec3(tfilebuffer &bf)
{
  tvector3 result;
  result.x = readfloat(bf);
  result.y = readfloat(bf);
  result.z = readfloat(bf);
  return result;
}

tmapcolor readcolor(tfilebuffer &bf)
{
  tmapcolor result;
  result[2] = readuint8(bf);
  result[1] = readuint8(bf);
  result[0] = readuint8(bf);
  result[3] = readuint8(bf);
  return result;
}

tmapvertex readvertex(tfilebuffer &bf)
{
  tmapvertex result;
  result.x = readfloat(bf);
  result.y = readfloat(bf);
  result.z = readfloat(bf);
  result.rhw = readfloat(bf);
  result.color = readcolor(bf);
  result.u = readfloat(bf);
  result.v = readfloat(bf);
  return result;
}

tmapcolor mapcolor(std::uint32_t color)
{
  tmapcolor result;
  result[0] = ((unsigned long)color >> (0 * 8)) & 0xff;
  result[1] = ((unsigned long)color >> (1 * 8)) & 0xff;
  result[2] = ((unsigned long)color >> (2 * 8)) & 0xff;
  result[3] = ((unsigned long)color >> (3 * 8)) & 0xff;
  return result;
}

/******************************************************************************/
/*                                LoadMapFile                                 */
/******************************************************************************/

bool loadmapfile(FileUtility &fs, const tmapinfo &mapinfo, tmapfile &map)
{
  tfilebuffer bf;
  std::int32_t i, j, n, m;

  bool result = false;

  if (!readallbytes(fs, mapinfo, bf))
  {
    return result;
  }

  // header/options

  map.mapinfo = mapinfo;
  map.filename = mapinfo.name;
  map.version = readint32(bf);
  map.mapname = readstring(bf, 38);
  map.textures.push_back(readstring(bf, 24));
  map.bgcolortop = readcolor(bf);
  map.bgcolorbtm = readcolor(bf);
  map.startjet = readint32(bf);
  map.grenadepacks = readuint8(bf);
  map.medikits = readuint8(bf);
  map.weather = readuint8(bf);
  map.steps = readuint8(bf);
  map.randomid = readint32(bf);

  // polygons

  n = readint32(bf);

  if ((n > max_polys) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.polygons.push_back(tmappolygon());
    map.polygons[i].vertices[0] = readvertex(bf);
    map.polygons[i].vertices[1] = readvertex(bf);
    map.polygons[i].vertices[2] = readvertex(bf);
    map.polygons[i].normals[0] = readvec3(bf);
    map.polygons[i].normals[1] = readvec3(bf);
    map.polygons[i].normals[2] = readvec3(bf);
    map.polygons[i].polytype = readuint8(bf);
    map.polygons[i].textureindex = 0;
  }

  // sectors

  map.sectorsdivision = readint32(bf);
  map.sectorsnum = readint32(bf);

  if ((map.sectorsnum > max_sector) || (map.sectorsnum < 0))
    return result;

  n = (2 * map.sectorsnum + 1) * (2 * map.sectorsnum + 1);

  for (i = 0; i <= n - 1; i++)
  {
    map.sectors.push_back(tmapsector());
    m = readuint16(bf);

    if (m > max_polys)
    {
      return result;
    }

    for (j = 0; j <= m - 1; j++)
    {
      map.sectors[i].Polys.push_back(readuint16(bf));
    }
  }

  // props

  n = readint32(bf);

  if ((n > max_props) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.props.push_back({});
    map.props[i].active = (readuint8(bf) != 0);
    bf.pos += 1;
    map.props[i].style = readuint16(bf);
    map.props[i].width = readint32(bf);
    map.props[i].height = readint32(bf);
    map.props[i].x = readfloat(bf);
    map.props[i].y = readfloat(bf);
    map.props[i].rotation = readfloat(bf);
    map.props[i].scalex = readfloat(bf);
    map.props[i].scaley = readfloat(bf);
    map.props[i].alpha = readuint8(bf);
    bf.pos += 3;
    map.props[i].color = readcolor(bf);
    map.props[i].level = readuint8(bf);
    bf.pos += 3;
  }

  // scenery

  n = readint32(bf);

  if ((n > max_props) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.scenery.push_back(tmapscenery());
    map.scenery[i].filename = readstring(bf, 50);
    map.scenery[i].date = readint32(bf);
  }

  // colliders

  n = readint32(bf);

  if ((n > max_colliders) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.colliders.push_back(tmapcollider());
    map.colliders[i].active = (readuint8(bf) != 0);
    bf.pos += 3;
    map.colliders[i].x = readfloat(bf);
    map.colliders[i].y = readfloat(bf);
    map.colliders[i].radius = readfloat(bf);
  }

  // spawnpoints

  n = readint32(bf);

  if ((n > max_spawnpoints) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.spawnpoints.push_back({});
    map.spawnpoints[i].active = (readuint8(bf) != 0);
    bf.pos += 3;
    map.spawnpoints[i].x = readint32(bf);
    map.spawnpoints[i].y = readint32(bf);
    map.spawnpoints[i].team = readint32(bf);
  }

  // waypoints

  n = readint32(bf);

  if ((n > max_waypoints) || (n < 0))
  {
    return result;
  }

  for (i = 0; i <= n - 1; i++)
  {
    map.waypoints.push_back({});
    map.waypoints[i].active = (readuint8(bf) != 0);
    bf.pos += 3;
    map.waypoints[i].id = readint32(bf);
    map.waypoints[i].x = readint32(bf);
    map.waypoints[i].y = readint32(bf);
    map.waypoints[i].left = (readuint8(bf) != 0);
    map.waypoints[i].right = (readuint8(bf) != 0);
    map.waypoints[i].up = (readuint8(bf) != 0);
    map.waypoints[i].down = (readuint8(bf) != 0);
    map.waypoints[i].m2 = (readuint8(bf) != 0);
    map.waypoints[i].pathnum = readuint8(bf);
    map.waypoints[i].c1 = readuint8(bf);
    map.waypoints[i].c2 = readuint8(bf);
    map.waypoints[i].c3 = readuint8(bf);
    bf.pos += 3;
    map.waypoints[i].connectionsnum = readint32(bf);

    for (j = 1; j <= max_connections; j++)
    {
      map.waypoints[i].connections[j] = readint32(bf);
    }
  }

  map.hash = crc32(5381, &bf.data[0], bf.data.size());
  result = true;
  return result;
}

bool ispropactive(tmapfile &map, std::int32_t index)
{
  tmapprop *prop;

  bool ispropactive_result;
  prop = &map.props[index];
  ispropactive_result =
    prop->active && (prop->level <= 2) && (prop->style > 0) && (prop->style < map.scenery.size());
  return ispropactive_result;
}
