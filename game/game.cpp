// Copyright © 2012 the Minima Authors under the MIT license. See AUTHORS for the list of authors.
#include "game.hpp"
#include "world.hpp"
#include "ui.hpp"
#include <cstdarg>
#include <cstdio>

Failure::Failure(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
}

ExploreScreen::ExploreScreen(Ui &win, World &w)
	: world(w), scroll(Vec2::Zero), mul(1), x0(0), y0(0), drag(false) {
	world.Center(win, world.x0, world.y0);
	win.InitTiles((win.width/World::TileW).whole() + 2,
		(win.height/World::TileH).whole() + 3,
		World::TileW.whole(),
		World::TileH.whole(),
		std::unique_ptr<Img>(LoadImg("resrc/tiles.png")));
}

ExploreScreen::~ExploreScreen() { }

void ExploreScreen::Update(ScreenStack&) {
	Vec2 s = scroll*mul;
	world.Scroll(s.x, s.y);
}

void ExploreScreen::Draw(Ui &win) {
	win.Clear();
	world.Draw(win);
	win.Flip();
}

void ExploreScreen::Handle(ScreenStack&, Event &e) {
	Fixed amt(0);
	switch (e.type) {
	case Event::MouseDown:
		scroll = Vec2::Zero;
		drag = true;
		x0 = e.x;
		y0 = e.y;
		break;

	case Event::MouseUp:
		drag = false;
		break;

	case Event::MouseMoved:
		if (!drag)
			break;
		world.Scroll(Fixed(e.x - x0), Fixed(y0 - e.y));
		x0 = e.x;
		y0 = e.y;
		break;

	case Event::KeyDown:
	case Event::KeyUp:
		if (e.type == Event::KeyDown) amt = Fixed(ScrollSpd);
		else amt = Fixed(0);
		switch (e.button) {
		case Event::DownArrow:
			scroll.y = amt;
			break;
		case Event::UpArrow:
			scroll.y = -amt;
			break;
		case Event::LeftArrow:
			scroll.x = amt;
			break;
		case Event::RightArrow:
			scroll.x = -amt;
			break;
		case Event::LShift:
		case Event::RShift:
			if (e.type == Event::KeyDown)
				mul = Fixed(5);
			else
				mul = Fixed(1);
		case Event::None:
			scroll.x = Fixed(0);
			scroll.y = Fixed(0);
		default:
			// ignore
			break;
		}
		break;

	default:
		// ignore
		break;
	}

}