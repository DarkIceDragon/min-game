#include "world.hpp"
#include "fixed.hpp"
#include "game.hpp"
#include "ui.hpp"
#include <limits>

const Fixed World::TileW(16);
const Fixed World::TileH(16);
const Vec2 World::TileSz(TileW, TileH);

World::Terrain::Terrain(char c, const char *resrc) : ch(c) {
	img = LoadImg(resrc);
	if (!img)
		throw Failure("Failed to load %s", resrc);
}

World::TerrainType::TerrainType() {
	t.resize(255);
	t['w'] = Terrain('w', "resrc/Water.png");
	t['g'] = Terrain('g', "resrc/Grass.png");
	t['m'] = Terrain('m', "resrc/Mountain.png");

	auto f = LoadFont("resrc/retganon.ttf", 12, 128, 128, 128);
	htImg.resize(World::MaxHeight+1);
	for (int i = 0; i <= World::MaxHeight; i++)
		htImg[i] = f->Render("%d", i);
}

World::World(FILE *in) : size(Fixed(0), Fixed(0)), xoff(0), yoff(0) {
	int n = fscanf(in, "%d %d\n", &width, &height);
	if (n != 2)
		throw Failure("Failed to read width and height");
	if (width <= 0 || height <= 0)
		throw Failure("%d by %d is an invalid world size", width, height);
	if (std::numeric_limits<int>::max() / width < height)
		throw Failure("%d by %d is too big", width, height);

	size = Vec2(Fixed(width), Fixed(height));

	locs.resize(width*height);
	for (int i = 0; i < width*height; i++) {
		char c;
		int h, d;
		n = fscanf(in, " %c %d %d", &c, &h, &d);
		if (n != 3)
			throw Failure("Failed to read a location %u", i);
		if (h > MaxHeight || h < 0)
			throw Failure("Location %u has invalid height %d", i, h);
		if (d < 0 || d > h)
			throw Failure("Location %u of height %d has invalid depth %d", i, h, d);
		if (!terrain[c].ch)
			throw Failure("Unknown terrain type %c", c);
		locs[i].height = h;
		locs[i].depth = d;
		locs[i].terrain = c;
	}

	n = fscanf(in, " %d %d", &x0, &y0);
	if (n != 2)
		throw Failure("Failed to read the start location");
}

void World::Draw(std::shared_ptr<Ui> ui) {
//	extern bool drawHeights;	// main.cc
	Vec2 offs(xoff, yoff);

	ui->DrawWorld(offs);
}

// shade returns the shade value for the given location
// which is based on its height and depth.
//
// The shade value is computed by linear interpolation
// between 0=minSh and MaxHeight=1.
float World::Loc::Shade() const{
	// minSh is the minimum shade value.
	static const float minSh = 0.25;
	static const float slope = (1 - minSh) / World::MaxHeight;
	return slope*(this->height - this->depth) + minSh;
}