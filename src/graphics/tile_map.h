#ifndef __tile_map_h
#define __tile_map_h
#include <lcthw/hashmap.h>
#include <lcthw/darray.h>
#include "graphics.h"

//TODO: objectgroup
//TODO: properties

extern const unsigned FLIPPED_HORIZONTALLY_FLAG;
extern const unsigned FLIPPED_VERTICALLY_FLAG;
extern const unsigned FLIPPED_DIAGONALLY_FLAG;

// Only PNG
// Doesn't support: format, trans
typedef struct Tileset {
    int spacing;
    int margin;
    GfxSize tile_size;
    uint32_t first_gid;
    char *name;
    char *img_src;
    GfxTexture *texture;
} Tileset;

void Tileset_destroy(Tileset *tileset);

typedef struct TilesetTile {
  uint32_t gid;
  Tileset *tileset;
  VPoint tl;
  GfxSize size;
} TilesetTile;

typedef struct TileMapLayer {
    char *name;
    double opacity;
    int visible;
    int gid_count;
    uint32_t *tile_gids;
    GfxTexture *atlas;
    uint8_t *raw_atlas;
    Tileset *tileset;
} TileMapLayer;

TileMapLayer *TileMapLayer_create();
void TileMapLayer_destroy(TileMapLayer *layer);

// Only orthogonal
typedef struct TileMap {
  int rows;
  int cols;
  GfxSize tile_size;
  DArray *tilesets;
  DArray *layers;
} TileMap;

TileMap *TileMap_create();
void TileMap_destroy(TileMap *map);
void TileMap_render(TileMap *map, Graphics *graphics, int pixels_per_cell);
TilesetTile *TileMap_resolve_tile_gid(TileMap *map, uint32_t gid);

void TileMapLayer_draw(TileMapLayer *layer, TileMap *map, Graphics *graphics);

#endif
