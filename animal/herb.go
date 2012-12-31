// © 2012 the Minima Authors under the MIT license. See AUTHORS for the list of authors.

package animal

import (
	"math/rand"

	"code.google.com/p/min-game/ai"
	"code.google.com/p/min-game/geom"
	"code.google.com/p/min-game/phys"
	"code.google.com/p/min-game/sprite"
	"code.google.com/p/min-game/ui"
	"code.google.com/p/min-game/world"
)

// Herbivore is a struct representing basic herding, non-agressive, herbivorous animals.
type Herbivore struct {
	Anim sprite.Anim
	Boid ai.Boid
}

type Herbivores struct {
	Info  *Info
	Herbs []*Herbivore
}

func MakeHerbivores(name string) (Herbivores, error) {
	i, err := LoadInfo(name)
	if err != nil {
		return Herbivores{}, err
	}
	return Herbivores{&i, nil}, err
}

func (hs Herbivores) Move(w *world.World) {
	for _, h := range hs.Herbs {
		h.Anim.Move(&hs.Info.Sheet, h.Boid.Body.Vel)
		h.Boid.Body.Move(w, hs.Info.Affinity)
	}
}

func (hs Herbivores) Draw(d ui.Drawer, cam ui.Camera) {
	for _, h := range hs.Herbs {
		cam.Draw(d, ui.Sprite{
			Name:   hs.Info.Sheet.Name,
			Bounds: hs.Info.Sheet.Frame(h.Anim.Face, h.Anim.Frame),
			Shade:  1.0,
		}, h.Boid.Body.Box.Min)
	}
}

// Spawn spawns a new Herbivore for this Herbivores collection.
func (hs *Herbivores) Spawn(p, v geom.Point) {
	sz := float64(hs.Info.Sheet.FrameSize)
	h := new(Herbivore)
	h.Boid.Body = phys.Body{
		Box: geom.Rect(p.X, p.Y, p.X+sz, p.Y+sz),
		Vel: v,
	}
	h.Boid.ThinkGroup = uint(rand.Intn(ai.NThinkGroups))
	hs.Herbs = append(hs.Herbs, h)
}

func (hs Herbivores) Len() int {
	return len(hs.Herbs)
}

func (hs Herbivores) Boid(n int) *ai.Boid {
	return &hs.Herbs[n].Boid
}

func (hs Herbivores) BoidInfo() ai.BoidInfo {
	return hs.Info.BoidInfo
}