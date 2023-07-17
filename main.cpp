#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <crtdbg.h>

int windowWidth = 1280;
int windowHeight = 720;
int points = 0;
double delta = 0.0;
Uint64 start = 0;
Uint64 end = 0;
SDL_Window* window;
SDL_Renderer* renderer;
int fontSize = 22;
int textX, textY = 1;
SDL_Color textoCor = { 255, 255, 255 }; //Branco
std::string textoStr = "Points: 0";
std::string title = "Asteroids!";

//Classes
struct Vector2 {
	float x = 0.0f, y = 0.0f;
};

class Entity {
private:
	SDL_Texture* texture;
	float angle;
	float velocity;

public:
	Entity() : texture(nullptr), angle(0.0f), velocity(0.0f) {
		rect = { 0, 0, 0, 0 };
	}
	//todo arrume o behavior da movimentação da nave
	~Entity() {
		if (texture) {
			SDL_DestroyTexture(texture);
		}
	}
	SDL_Rect rect;

	void setTexture(const std::string& texturePath) {
		SDL_Surface* surface = IMG_Load(texturePath.c_str());
		if (!surface) {std::cout << "Fail to load surface from image: " << texturePath << " SDL ERROR: " << SDL_GetError() << std::endl;}
		else {
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (!texture) {
				std::cout << "Fail to create texture: " << texturePath << " SDL ERROR: " << SDL_GetError() << std::endl;
			}
			rect.w = surface->w;
			rect.h = surface->h;
			SDL_FreeSurface(surface);
		}
	}
	SDL_Rect getRect() {
		return rect;
	}

	void setPosition(const Vector2& position) {
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
	}

	Vector2 getPosition() const {
		return Vector2({ static_cast<float>(rect.x),  static_cast<float>(rect.y) });
	}

	float getAngle() const {
		return angle;
	}

	void setVelocity(const float& _velocity) {
		velocity = _velocity;
	}

	float getVelocity() const {
		return velocity;
	}

	void setAngle(float _angle) {
		angle = fmod(_angle, 360.0f);
		if (angle < 0.0f) {
			angle += 360.0f;
		}
	}

	void rotate(float _angle) {
		angle += _angle;
		angle = fmod(angle, 360.0f);
		if (angle < 0.0f) {
			angle += 360.0f;
		}
	}


	void moveTo(const Vector2& direction) {
		rect.x += static_cast<int>(direction.x);
		rect.y += static_cast<int>(direction.y);
	}

	void teleportTo(const Vector2& location) {
		rect.x = static_cast<int>(location.x);
		rect.y = static_cast<int>(location.y);
	}

	void changeRectSize(Vector2 _factor) {
		rect.w = static_cast<int>(rect.w * _factor.x);
		rect.h = static_cast<int>(rect.h * _factor.y);
	}

	void moveToAngle(float distance) {
		float radians = angle * (M_PI / 180.0f);
		rect.x += static_cast<int>(distance * cos(radians));
		rect.y += static_cast<int>(distance * sin(radians));
	}

	void applyInertia() {
		moveToAngle(velocity);
	}

	void allowWarp() {
		if (rect.x > windowWidth) {
			rect.x = 0 - rect.w;
		}
		else if (rect.x + rect.w < 0) {
			rect.x = windowWidth;
		}

		if (rect.y > windowHeight) {
			rect.y = 0 - rect.h;
		}
		else if (rect.y + rect.h < 0) {
			rect.y = windowHeight;
		}
	}

	void render(SDL_Renderer* renderer) {
		SDL_RenderCopyEx(renderer, texture, nullptr, &rect, angle, nullptr, SDL_FLIP_NONE);
	}
};


class Player : public Entity {
public:
	Player() {
		setPosition({static_cast<float>(windowWidth) / 2, static_cast<float>(windowHeight) / 2 });
	}
	void process() {
		moveToAngle(getVelocity());
		allowWarp();
	}
};

class Bullet : public Entity {
public:
	Bullet(SDL_Rect _rect , float _angle) {
		setTexture("img/bullet.png");
		setPosition({ static_cast<float>(_rect.x + (_rect.w / 2)), static_cast<float>(_rect.y + (_rect.h/2)) });
		setAngle(_angle);
	}
	void process() {
		moveToAngle(5.0f);
	}
};

class Asteroid : public Entity {
public:
	Asteroid() {
		setTexture("img/asteroid.png");
		srand(time(NULL));
		setPosition({-500.0f, -500.0f });
		//changeRectSize({ 2.0f,2.0f });
	}
	int minSpeed = 0.01f, maxSpeed = 1.0f;
	bool ded = false;
	void process() {
		applyInertia();
		allowWarp();
	}

};
//As coisas são feitas aqui
Player player = Player();
std::vector<std::shared_ptr<Asteroid>> asteroids;
std::vector<std::shared_ptr<Bullet>> bullets;


void addPoint(int _pointNum) {
	points += _pointNum;
	textoStr = "Points: " + std::to_string(points);
}
//TODO deixe isso aqui menos porco:
void initPlayerAndAsteroid() {
	for (int i = 0; i < 5;i++){
		asteroids.push_back(std::make_shared<Asteroid>());
	}
	int min = 0, max = windowWidth;
	for (auto& asteroid : asteroids) {
		//asteroid->setTexture("img/asteroid.png");
		float aste_x = rand() % (max - min) + min; //random location longe do player se possivel ;-; // good luck lmao
		float aste_y = rand() % windowHeight;
		asteroid->setPosition({ aste_x, aste_y });
		int randAngle = rand() % 360; //TRETA
		asteroid->setAngle(randAngle);
		asteroid->setVelocity(2.0f);
	}
	player.setTexture("img/player.png"); //bote isso dentro do player, n sei pq mas nao tem como, sempre dá erro de renderer invalido
	player.changeRectSize({0.5f,0.5f}); //smol pureia

}


void toggleFullScreen() {
	static bool isFullScreen = false;
	if (isFullScreen) {
		SDL_SetWindowFullscreen(window, 0); // Switch to windowed mode
		isFullScreen = false;
	}
	else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN); // Switch to full screen mode
		isFullScreen = true;
	}
}

void clean() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}


void logic() {
	player.process();
	if (bullets.size() > 0) {
		for (auto& bullet : bullets) {
			bullet->process();
		}
	}
	if (asteroids.size() > 0){
		for (auto& asteroid : asteroids) {
			asteroid->process();
			if (SDL_HasIntersection(&asteroid->rect, &player.rect)) {
				//Todo reset game
				break;
			}
			// Não gosto como tem esse loop dentro do outro, não parece eficiente
			if (bullets.size() > 0) {
				for (auto& bullet : bullets) {
					//talvez melhor criar uma função para fazer isso TODO
					if (SDL_HasIntersection(&asteroid->rect, &bullet->rect)) {
						addPoint(10);
						bullet->setPosition({ -5000.0f,-1000.0f });
						asteroid->setPosition({ -1001.0f,-1001.0f });
						asteroid->ded = true;
					}
				}
			
		}
	}
	
	}
	//Deletores 
	bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
		[&](const std::shared_ptr<Bullet>& bullet) {
			return bullet->getPosition().x > windowWidth || bullet->getPosition().y > windowHeight || bullet->getPosition().x < 0 || bullet->getPosition().y < 0;
		}),
		bullets.end());
	asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(),
		[&](const std::shared_ptr<Asteroid>& asteroid) {
			return asteroid->ded;
		}),
		asteroids.end());
}

void playerInput() {
	//FOR THE LOVE OF GOD MAKE THIS FEEL BETTER
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			clean();
			exit(0);
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_F11) {
				toggleFullScreen();
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				std::cout << "Encerrando..." << std::endl;
				clean();
				exit(0);
			}
			if (event.key.keysym.sym == SDLK_d) {
				player.rotate(500.0 * delta);
			}
			if (event.key.keysym.sym == SDLK_a) {
				player.rotate(-500.0 * delta);
			}
			if (event.key.keysym.sym == SDLK_w) {
				player.setVelocity(player.getVelocity() + 1.0f);
			}
			if (event.key.keysym.sym == SDLK_s) {
				if(player.getVelocity() > 0){
				player.setVelocity(player.getVelocity() - 1.0f);
				}
			}
			if (event.key.keysym.sym == SDLK_SPACE) {
				bullets.push_back(std::make_shared<Bullet>(player.getRect(), player.getAngle()));
			}
		}

	}
}

void renderizarTexto(std::string _texto) {
	//MEMORY LEAK AAAAAAAAAAAAAAAA
	TTF_Font* font = TTF_OpenFont("font/Ubuntu-Regular.ttf", fontSize);//tire isso de dentro da func
	const char* textoConv = _texto.c_str();
	if (textoConv == nullptr) {

		printf("Text is nullptr\n");
		return;
	}
	SDL_Surface* surface = TTF_RenderText_Solid(font, textoConv, textoCor);
	if (&surface == NULL) {
		printf("Text Surface is NULL\n");
		return;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (&texture == NULL) {
		printf("Text texture is NULL\n");
		return;
	}
	SDL_Rect destRect = { textX, textY, surface->w, surface->h }; //erro aqui
	SDL_RenderCopy(renderer, texture, NULL, &destRect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void renderStuff(std::vector<std::shared_ptr<Asteroid>> _asteroids, Player& _player, std::vector<std::shared_ptr<Bullet>> _bullets,std::string _pointsTxt) {
	SDL_RenderClear(renderer); //Limpa o ultimo frame
	for (const auto& asteroid : _asteroids) {
		asteroid->render(renderer);
	}
	for (const auto& bullet : _bullets) {
		bullet->render(renderer);
	}
	_player.render(renderer);
	renderizarTexto(_pointsTxt);
	SDL_RenderPresent(renderer); // exibe tudo que esta no buffer do frame
}

void mainloop() {
	while (true) {
		start = SDL_GetPerformanceCounter();
		SDL_Delay(floor(16.66666f - delta));


		playerInput();
		logic();
		renderStuff(asteroids, player, bullets,textoStr);


		end = SDL_GetPerformanceCounter();
		delta = (end - start) / (float)SDL_GetPerformanceFrequency();

		title = "Asteroids! FPS: " + std::to_string((int)(1.0f / delta));
		SDL_SetWindowTitle(window, title.c_str());

	}
}

int initSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl; return 1;
	}
	window = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (window == NULL) { std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl; return 1; }
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) { std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl; return 1; }
	int IMGflags = IMG_INIT_JPG | IMG_INIT_PNG;
	int IMGinitted = IMG_Init(IMGflags);
	if ((IMGinitted & IMGflags) != IMGflags) {
		std::cout << "Fail to init PNG and JPG support: " << SDL_GetError() << std::endl; return 1;
	}
	TTF_Init();
	return 0;
}

int run() {
	initSDL();
	initPlayerAndAsteroid();
	mainloop();
	return 0;
}

int main(int argc, char** argv) {
	run();
	_CrtDumpMemoryLeaks();
	return 0;
}