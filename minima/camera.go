// © 2012 the Minima Authors under the MIT license. See AUTHORS for the list of authors.

package main

import (
	"code.google.com/p/min-game/ui"
)

type Camera struct {
	cam ui.Point
}

func (c *Camera) Move(v ui.Point) {
	c.cam.Add(v)
}

func (c *Camera) Center(v ui.Point) {
	c.cam.X = ScreenDims.X/2.0 - v.X
	c.cam.Y = ScreenDims.Y/2.0 - v.Y
}

func (c *Camera) Pos() ui.Point {
	return c.cam
}

func (c *Camera) Draw(d Drawer, x interface{}, p ui.Point) (ui.Point, error) {
	return d.Draw(x, p.Sub(c.cam))
}
