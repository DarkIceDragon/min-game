// Copyright © 2012 the Minima Authors under the MIT license. See AUTHORS for the list of authors.
#include "ui.hpp"
#include "game.hpp"
#include "world.hpp"
#include "opengl.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stack>
#include <utility>

class KeyHandler {
public:
	KeyHandler(){}

	//returns the number of pressed keys
	int KeysDown();

	//is the given key pressed
	bool IsPressed(int i);

	//returns the active key
	int ActiveKey();

	//handles a single Key Stroke
	int HandleStroke(SDL_Event &sdle, bool keydown);

	//returns english thing for key
	std::string KeyName(int k);

private:
	bool keyState[Event::NumKeys];
	std::stack<int> pressedOrder;

	/* in the event that more than n-keys are depressed, we need to start
	   polling keyboard state because modern keyboards suck. */
	void PollKeyboard();

	// fixes the activeKey by assuring top of the stack is still pressed
	void FixStack();

	//does key k go to the stack?
	bool IsStackable(int k);
};

struct Ui::Impl {
	Vec2 cam;
	SDL_Surface *win;
	KeyHandler kh;
	OpenGLUi gl;
	Impl(Fixed w, Fixed h, const std::string &t);
};

struct SdlImg : public OpenGLImg {
	Vec2 sz;
	SdlImg(SDL_Surface*);
	virtual ~SdlImg() { }
	virtual Vec2 Size() const { return sz; }
};

struct SdlFont : public Font {
	TTF_Font *font;
	Color color;

	SdlFont(const std::string&, int, Color);
	virtual ~SdlFont();
	virtual Img *Render(const std::string&);
};

static SDL_Surface *init_sdl(Fixed w, Fixed h){
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		throw Failure("Failed to initialized SDL video");

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Surface *win = SDL_SetVideoMode(w.whole(), h.whole(), 0, SDL_OPENGL);
	if (!win)
		throw Failure("Failed to set SDL video mode");
	return win;
}

Ui::Impl::Impl(Fixed w, Fixed h, const std::string &title)
	: cam(Fixed(0), Fixed(0)), win(init_sdl(w, h)), gl(w, h) {
}

Ui::Ui(Fixed w, Fixed h, const std::string &title)
	: impl(new Impl(w, h, title)), width(w), height(h) {
	fprintf(stderr,"Vendor: %s\nRenderer: %s\nVersion: %s\nShade Lang. Version: %s\n",
	glGetString(GL_VENDOR),
	glGetString(GL_RENDERER),
	glGetString(GL_VERSION),
	glGetString(GL_SHADING_LANGUAGE_VERSION));

	int imgflags = IMG_INIT_PNG;
	if ((IMG_Init(imgflags) & imgflags) != imgflags)
		throw Failure("Failed to initialize png support: " + std::string(IMG_GetError()));

	if (TTF_Init() == -1)
		throw Failure("Failed to initialize SDL_ttf: " + std::string(TTF_GetError()));
}

Ui::~Ui(){
}

void Ui::DrawLine(const Vec2 &a, const Vec2 &b, const Color &c){
	impl->gl.DrawLine(a, b, c);
}

void Ui::FillRect(const Vec2 &a, const Vec2 &b, const Color &c){
	impl->gl.FillRect(a, b, c);
}

void Ui::DrawRect(const Vec2 &a, const Vec2 &b, const Color &c){
	impl->gl.DrawRect(a, b, c);
}

void Ui::Draw(const Vec2 &p, Img *img, float shade){
	impl->gl.Draw(p, img, shade);
}

void Ui::Draw(const Vec2 &p, const TileView &tv){
	impl->gl.Draw(p, tv);
}

void Ui::MoveCam(Vec2 v){
	impl->cam += v;
}

void Ui::CenterCam(Vec2 v){
	impl->cam.x = v.x - ScreenDims.x/Fixed{2};
	impl->cam.y = v.y - ScreenDims.y/Fixed{2};
}

Vec2 Ui::CamPos() const{
	return impl->cam;
}

void Ui::DrawCam(Vec2 p, Img *i, float shade){
	this->Draw(p - impl->cam, i, shade);
}

void Ui::Flip() {
	SDL_GL_SwapBuffers();
}

void Ui::Clear(){
	impl->gl.Clear();
}

void Ui::Delay(unsigned long msec) {
	SDL_Delay(msec);
}

unsigned long Ui::Ticks() {
	return SDL_GetTicks();
}

static bool getbutton(SDL_Event &sdle, Event &e) {
	switch (sdle.button.button) {
	case SDL_BUTTON_LEFT:
		e.button = Event::MouseLeft;
		break;
	case SDL_BUTTON_RIGHT:
		e.button = Event::MouseRight;
		break;
	case SDL_BUTTON_MIDDLE:
		e.button = Event::MouseCenter;
		break;
	default:
		return false;
	};
	return true;
}


bool Ui::PollEvent(Event &e) {
	SDL_Event sdle;
	bool keydown;
	static bool simulatedLast;

	while (SDL_PollEvent(&sdle)) {
		switch (sdle.type) {
		case SDL_QUIT:
			e.type = Event::Closed;
			simulatedLast = false;
			return true;

		case SDL_MOUSEBUTTONDOWN:
			e.type = Event::MouseDown;
			e.x = sdle.button.x;
			e.y = sdle.button.y;
			if (!getbutton(sdle, e))
				continue;
			simulatedLast = false;
			return true;

		case SDL_MOUSEBUTTONUP:
			e.type = Event::MouseUp;
			e.x = sdle.button.x;
			e.y = sdle.button.y;
			if (!getbutton(sdle, e))
				continue;
			simulatedLast = false;
			return true;

		case SDL_MOUSEMOTION:
			e.type = Event::MouseMoved;
			e.x = sdle.motion.x;
			e.y = sdle.motion.y;
			simulatedLast = false;
			return true;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			keydown = (sdle.type == SDL_KEYDOWN)? true : false;
			e.button = impl->kh.HandleStroke(sdle,keydown);
			e.type = keydown ? Event::KeyDown : Event::KeyUp;
			simulatedLast = false;
			return true;

		default:
			// ignore
			break;
		}
	}

	if(impl->kh.KeysDown() > 0 && !simulatedLast){
		e.button = impl->kh.ActiveKey();
		e.type = Event::KeyDown;
		simulatedLast = true;
		return true;
	}

	return false;
}

SdlImg::SdlImg(SDL_Surface *surf) : sz(Fixed(surf->w), Fixed(surf->h)) {
	GLint pxSz = surf->format->BytesPerPixel;
	GLenum texFormat = GL_BGRA;
	switch (pxSz) {
	case 4:
		if (surf->format->Rmask == 0xFF)
			texFormat = GL_RGBA;
		break;
	case 3:
		if (surf->format->Rmask == 0xFF)
			texFormat = GL_RGB;
		else
			texFormat = GL_BGR;
		break;
	default:
		throw Failure("Bad image color type… apparently");
	}

	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, pxSz, surf->w, surf->h, 0,
		texFormat, GL_UNSIGNED_BYTE, surf->pixels);
}

SdlFont::SdlFont(const std::string &path, int sz, Color c)
		: color(c) {
	font = TTF_OpenFont(path.c_str(), sz);
	if (!font)
		throw Failure("Failed to load font " + path + ": " + TTF_GetError());
}

SdlFont::~SdlFont() {
	TTF_CloseFont(font);
}

Img *SdlFont::Render(const std::string &s) {
	SDL_Color c{};
	c.r = color.r;
	c.g = color.g;
	c.b = color.b;
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, s.c_str(), c);
	if (!surf)
		throw Failure("Failed to render text: " + std::string(TTF_GetError()));

	Img *img = new SdlImg(surf);
	SDL_FreeSurface(surf);
	return img;
}

std::unique_ptr<Img> LoadImg(const std::string &path) {
	SDL_Surface *surf = IMG_Load(path.c_str());
	if (!surf)
		throw Failure("Failed to load image " + path);
	Img *img = new SdlImg(surf);
	SDL_FreeSurface(surf);
	return std::unique_ptr<Img>(img);
}

std::unique_ptr<Font> LoadFont(const std::string &path, int sz, Color c) {
	return std::unique_ptr<Font>(new SdlFont(path, sz, c));
}

int KeyHandler::KeysDown(){
	return pressedOrder.size();
}

bool KeyHandler::IsPressed(int i){
	return i >= 0 && i < Event::NumKeys && keyState[i];
}

int KeyHandler::ActiveKey(){
	if(pressedOrder.empty())
		return Event::None;
	else
		return pressedOrder.top();
}

std::string KeyHandler::KeyName(int k){
	switch (k){
	case Event::UpArrow:
		return "UP";
	case Event::DownArrow:
		return "DOWN";
	case Event::LeftArrow:
		return "LEFT";
	case Event::RightArrow:
		return "RIGHT";
	case Event::LShift:
	case Event::RShift:
		return "SHIFT";
	case Event::None:
		return "No Key!";
	default:
		return "Invalid Key!";
	}
}

int KeyHandler::HandleStroke(SDL_Event &sdle, bool keydown){
	int key = Event::None;

	switch(sdle.key.keysym.sym){
        case SDLK_UP:
		key = Event::UpArrow;
		break;
	case SDLK_DOWN:
		key = Event::DownArrow;
		break;
	case SDLK_LEFT:
		key = Event::LeftArrow;
		break;
	case SDLK_RIGHT:
		key = Event::RightArrow;
		break;
	case SDLK_RSHIFT:
		key = Event::RShift;
		break;
	case SDLK_LSHIFT:
		key = Event::LShift;
		break;
	case SDLK_f:
		key = Event::Action;
		break;
	default:
		return Event::None;
	}

	if (key < Event::NumKeys) keyState[key] = keydown;
	if(keydown && IsStackable(key)) pressedOrder.push(key);
	else FixStack();
	return key;
}

void KeyHandler::PollKeyboard(){
	Uint8 *sdlkeys = SDL_GetKeyState(NULL);

	keyState[Event::LShift] = sdlkeys[SDLK_LSHIFT];
	keyState[Event::RShift] = sdlkeys[SDLK_RSHIFT];
	keyState[Event::RightArrow] = sdlkeys[SDLK_RIGHT];
	keyState[Event::LeftArrow] = sdlkeys[SDLK_LEFT];
	keyState[Event::UpArrow] = sdlkeys[SDLK_UP];
	keyState[Event::DownArrow] = sdlkeys[SDLK_DOWN];
}

void KeyHandler::FixStack(){
	//assumes that the keyState array is correct
	while(not pressedOrder.empty() &&
	       not keyState[pressedOrder.top()])
	       pressedOrder.pop();
}

bool KeyHandler::IsStackable(int k){
	return k != Event::LShift && k != Event::RShift;
}
