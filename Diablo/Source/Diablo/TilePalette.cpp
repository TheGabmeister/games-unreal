#include "TilePalette.h"

const FTilePaletteEntry* UTilePalette::FindEntry(EDungeonTileType TileType) const
{
	return Entries.FindByPredicate([TileType](const FTilePaletteEntry& Entry)
	{
		return Entry.TileType == TileType;
	});
}
