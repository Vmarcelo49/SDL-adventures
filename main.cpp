#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

//Vars iniciais
int windowWidth = 854;
int windowHeight = 480;
int points = 0;
Uint64 start = 0;
Uint64 end = 0;
double delta = 0.0f;
std::string title = "Asteroids!";
SDL_Window* window;
SDL_Renderer* renderer;


SDL_Texture* loadTexture(const char* filePath) {
	SDL_Texture* texture = nullptr;
	texture = IMG_LoadTexture(renderer, filePath);
	if (texture == NULL) { std::cout << "Fail to load texture: " << filePath << " SDL ERROR: " << SDL_GetError() << std::endl; }
	return texture;
}
//Classes
struct Vector2 {
	float x = 0.0f, y = 0.0f;
};



class Entity {
private:
	std::string name;
	std::string texturePath;
	SDL_Texture* texture;
	SDL_Rect rect;
	float angle;
	float velocity;

public:
	Entity() : texture(nullptr), angle(0.0f), velocity(0.0f) {
		rect = { 0, 0, 0, 0 }; }
	//todo arrume o behavior da movimentação da nave
	~Entity() {
		if (texture) {
			SDL_DestroyTexture(texture);
		}
	}

	void setTexture(/*SDL_Renderer* _renderer,*/ const std::string& path) {
		texturePath = path;
		SDL_Surface* surface = IMG_Load(texturePath.c_str());
		if (!surface) {
			// Handle error
			std::cout << "Fail to load surface from image: " << path << " SDL ERROR: " << SDL_GetError() << std::endl;
		}
		else {
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (!texture) {
				// Handle error
				std::cout << "Fail to create texture: " << path << " SDL ERROR: " << SDL_GetError() << std::endl;
			}
			rect.w = surface->w;
			rect.h = surface->h;
			SDL_FreeSurface(surface);
		}
	}
	SDL_Rect getRect() {
		return rect;
	}
	void setName(const std::string& _name) {
		name = _name;
	}

	std::string getName() const {
		return name;
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
		setName("player");
		//setTexture("img/player.png");
		setPosition({static_cast<float>(windowWidth) / 2, static_cast<float>(windowHeight) / 2 });
	}
	void process() {
		moveToAngle(getVelocity());
		allowWarp();
		// Collision check is in another place!!!
	}
};

class Bullet : public Entity {
public:
	Bullet(SDL_Rect _rect , float _angle) {
		setName("Bullet");
		setTexture("img/bullet.png");
		setPosition({ static_cast<float>(_rect.x + (_rect.w / 2)), static_cast<float>(_rect.y + (_rect.h/2)) });
		setAngle(_angle);
	}
	void process() {
		//applyInertia();
		moveToAngle(5.0f);
	}
};

class Asteroid : public Entity {
public:
	Asteroid() {
		setName("Asteroid");
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
		// check collision
	}

};
//aqui as instancias de player e asteroids
Player player = Player();
std::vector<std::shared_ptr<Asteroid>> asteroids;
std::vector<std::shared_ptr<Bullet>> bullets;

void initPlayerAndAsteroid() {
	for (int i = 0; i < 5;i++){
		asteroids.push_back(std::make_shared<Asteroid>());
	}
	int min = 0, max = windowWidth;
	for (auto& asteroid : asteroids) {
		asteroid->setTexture("img/asteroid.png");
		float aste_x = rand() % (max - min) + min; //random location longe do player se possivel ;-; // good luck lmao
		float aste_y = rand() % windowHeight;
		asteroid->setPosition({ aste_x, aste_y });
		int randAngle = rand() % 360; //TRETA
		asteroid->setAngle(randAngle);
		asteroid->setVelocity(2.0f);
	}
	player.setTexture("img/player.png");
	player.changeRectSize({0.5f,0.5f});

}

void changeWindowRes() {
	static int mode;
	if (mode == 0) {
		SDL_SetWindowSize(window, 1280, 720);
		mode = 1;
	}
	else if (mode == 1) {
		SDL_SetWindowSize(window, 1920, 1080);
		mode = 2;
	}
	else if (mode == 2) {
		SDL_SetWindowSize(window, 854, 480);
		mode = 0;
	}
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

void print(const char* text) {
	std::cout << text << std::endl;
}
void clean() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void logic() {
	player.process();
	SDL_Rect playerRect = player.getRect();
	const SDL_Rect* plRectPTR = &playerRect;

	if (bullets.size() > 0) {
		for (auto& bullet : bullets) {
			bullet->process();
		}
	}
	if (asteroids.size() > 0){
		for (auto& asteroid : asteroids) {
			asteroid->process();
			SDL_Rect asteRect = asteroid->getRect();
			const SDL_Rect* asteRectPTR = &asteRect;
			if (SDL_HasIntersection(asteRectPTR, plRectPTR)) {
				//reset game
				//SDL_Delay(50); //3frames?
				break;
			}
			// Não gosto como tem esse loop dentro do outro, não parece eficiente
			if (bullets.size() > 0) {
				for (auto& bullet : bullets) {
					
					SDL_Rect bullRect = bullet->getRect();
					const SDL_Rect* bullRectPTR = &bullRect;
					//talvez melhor criar uma função para fazer isso TODO
					if (SDL_HasIntersection(asteRectPTR, bullRectPTR)) {
						//points += 10;
						//bullet.reset();
						//Bullet* bullet_ptr = new Bullet(player.getRect(), player.getAngle());
						//delete bullet_ptr;
						std::cout << "COLISAO" << std::endl;
						bullet->setPosition({ -5000.0f,-1000.0f });
						asteroid->setPosition({ -1001.0f,-1001.0f });
						asteroid->ded = true;
					}
				}
			
		}
	}
	
	}
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
	//todo Programe a colisão aqui, o metodo de exlusão está acima e checagem de colisão em SDL2 é SDL_HasIntersection(const SDL_Rect * A, const SDL_Rect * B);
	//Colisão da bala com um asteroid:

}

void playerInput() {

	SDL_Event event;
	//SDL_PollEvent(&event);
	
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			clean();
			exit(0);
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_F10) {
				changeWindowRes();
			}
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



void renderStuff(std::vector<std::shared_ptr<Asteroid>> _asteroids, Player& _player, std::vector<std::shared_ptr<Bullet>> _bullets) {
	SDL_RenderClear(renderer); //Limpa o ultimo frame
	
	for (const auto& asteroid : _asteroids) {
		asteroid->render(renderer);
	}
	for (const auto& bullet : _bullets) {
		bullet->render(renderer);
	}
	_player.render(renderer);
	SDL_RenderPresent(renderer); // exibe tudo que esta no buffer do frame
}

void mainloop() {
	while (true) {
		start = SDL_GetPerformanceCounter();
		SDL_Delay(floor(16.66666f - delta));


		playerInput();
		logic();
		//checkCollision();
		renderStuff(asteroids, player, bullets); //Tenho que re implementar como atirar


		end = SDL_GetPerformanceCounter();
		delta = (end - start) / (float)SDL_GetPerformanceFrequency();

		//debug
		//std::cout << player.getAllInfo() << " Numero de Asteroides: " << asteroids.size() << "\n";

		title = "Asteroids! FPS: " + std::to_string((int)(1.0f / delta));
		SDL_SetWindowTitle(window, title.c_str());

	}
}


int run() {
	//SDL stuff
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
	initPlayerAndAsteroid();
	mainloop();
	return 0;
}

int main(int argc, char** argv) {
	run();
	return 0;
}