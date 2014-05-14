#include <windows.h>
#include <cassert>
#include <cstdio>
#include "DiceInvaders.h"
#include <vector>

class DiceInvadersLib
{
public:
	explicit DiceInvadersLib(const char* libraryPath)
	{
		m_lib = LoadLibrary(libraryPath);
		assert(m_lib);

		DiceInvadersFactoryType* factory = (DiceInvadersFactoryType*)GetProcAddress(
			m_lib, "DiceInvadersFactory");
		m_interface = factory();
		assert(m_interface);
	}

	~DiceInvadersLib()
	{
		FreeLibrary(m_lib);
	}

	IDiceInvaders* get() const
	{
		return m_interface;
	}

private:
	DiceInvadersLib(const DiceInvadersLib&);
	DiceInvadersLib& operator=(const DiceInvadersLib&);

private:
	IDiceInvaders* m_interface;
	HMODULE m_lib;
};

struct Vec2f{
	Vec2f(){};
	Vec2f(float xin, float yin) : x(xin), y(yin){};
	Vec2f(int xin, int yin) : x(float(xin)), y(float(yin)){};
	Vec2f operator+ (const Vec2f other){
		return Vec2f(x + other.x, y + other.y);
	}
	Vec2f operator- (const Vec2f other){
		return Vec2f(x - other.x, y - other.y);
	}
	float size(){ 
		return (x + y) / 2.0f;
	}
	float x;
	float y;
};

class AlienArmy{
public:
	int maxNbrAliensRow = 8;
	int maxNbrAliensCol = 2;
	
	Vec2f aliensPos[8][2];// *64;
	//float aliensBottomVerticalPos = 0;
	float alienDirection = 1.0f;



	int nbrBombs = 0;
	float bombHorizontalPos[6];
	float bombVerticalPos[6];
};


int APIENTRY WinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	LPSTR commandLine,
	int commandShow)
{
	DiceInvadersLib lib("DiceInvaders.dll");
	IDiceInvaders* system = lib.get();
	static int screenX = 640;
	static int screenY = 480;
	system->init(screenX, screenY);

	ISprite* sprite = system->createSprite("data/player.bmp");
	ISprite* rocket = system->createSprite("data/rocket.bmp");
	ISprite* alien1 = system->createSprite("data/enemy1.bmp");
	ISprite* alien2 = system->createSprite("data/enemy2.bmp");
	ISprite* bomb = system->createSprite("data/bomb.bmp");

	Vec2f playerPos = Vec2f(screenX / 2, screenY - 32);
	float lastTime = system->getElapsedTime();
	std::vector<Vec2f> rocketPos;
	static int maxNbrRockets = 6;


	int aliensRow = 8;
	int aliensCol =2;

	std::vector<Vec2f> alienPos;
	for (int i = 0; i < aliensCol; i++){
		for (int j = 0; j < aliensRow; j++){
			alienPos.push_back(Vec2f(j * 64, i * 64));
		}
	}
	int rightAlien = alienPos.size() - 1;
	int leftAlien = 0;
	float alienDirection = 1.0f;
	Vec2f alienMovement = Vec2f(0,0);


	int nbrBombs = 0;
	Vec2f bombPos[6];

	int lives = 3;

	float lastRocketFireTime = lastTime;
	while (system->update())
	{

		
		//draw everything
		//lives
		system->drawText(screenX - 142, 8, "Lives: ");
		for (int i = 1;i <= lives; i++){
			sprite->draw(screenX - i * 32, 0);
		}
		//player
		sprite->draw(int(playerPos.x), int(playerPos.y));
		for (int i = 0; i < rocketPos.size(); i++){
			rocket->draw(int(rocketPos[i].x), int(rocketPos[i].y));
		}
		//alien1->draw(screenX - 32, screenY - 32);
		for (int i = 0; i < alienPos.size(); i++){
			
			alien1->draw(int(alienPos[i].x), int(alienPos[i].y));
		}
		for (int i = 0; i < nbrBombs; i++){
			bomb->draw(int(bombPos[i].x), bombPos[i].y);
		}
		//get player movement
		float newTime = system->getElapsedTime();
		float move = (newTime - lastTime) * 160.0f;
		float alienSpeed = (newTime - lastTime) * 120.0f;
		lastTime = newTime;
		IDiceInvaders::KeyStatus keys;
		system->getKeyStatus(keys);
		if (keys.right){
			if (playerPos.x + move + 32 <= screenX)
				playerPos.x += move;
		}
		else if (keys.left){
			if (playerPos.x - move >= 0)
				playerPos.x -= move;
		}

		//move rockets (before instantiating new ones)
		for (int i = 0; i < rocketPos.size(); i++){
			rocketPos[i].y -= move;
			if (rocketPos[i].y < 0){//remove the rocket outside screen
				rocketPos.erase(rocketPos.begin() + i);
				//nbrRockets--;
				//rocketPos[i].y = rocketPos[nbrRockets].y;
				//rocketPos[i].x = rocketPos[nbrRockets].x;
			}

		}
		if (keys.fire){
			if (lastRocketFireTime + 0.5f < newTime){
				lastRocketFireTime = newTime;
				if (rocketPos.size() < maxNbrRockets){
					rocketPos.push_back(Vec2f(playerPos.x, playerPos.y + 32));
					//	rocketPos[nbrRockets].x = playerPos.x;
				//	rocketPos[nbrRockets].y = screenY - 64;
				//	nbrRockets++;
				}
			}
		}


		//calculate alien movement
		if (alienPos[alienPos.size() - 1].x + 32 >= screenX&&alienDirection == 1.0f){
			alienDirection = -1.0f;
			alienMovement = Vec2f(-alienSpeed, 32.0f);
		}
		else if (alienPos[leftAlien].x <= 0 && alienDirection == -1.0f){
			alienDirection = 1.0f;
			alienMovement = Vec2f(alienSpeed, 32.0f);
		}
		else{
			alienMovement = Vec2f(alienSpeed*alienDirection, 0.0f);
		}
		//process aliens
		for (int i = alienPos.size() - 1; i >= 0; i--){
			//move
			alienPos[i] = alienPos[i]+ alienMovement;

			//detect colision
			for (int j = 0; j< rocketPos.size(); j++){
				if (rocketPos[j].x + 16 > alienPos[i].x &&    rocketPos[j].x + 16 < alienPos[i].x + 32
					&& rocketPos[j].y + 16 > alienPos[i].y && rocketPos[j].y + 16 < alienPos[i].y + 32){
					alienPos.erase(alienPos.begin() + i);
					rocketPos.erase(rocketPos.begin() + j);
				//	nbrRockets--;
				//	rocketPos[i] = rocketPos[nbrRockets];
					
				}
			}
			
		}
		
		if (alienPos[alienPos.size()-1].y <= 0){
			//game over
		}
		

		//spawn bombs
		int random = rand()/10000;
		if (random == 1 && nbrBombs<6){		
			int alien = rand() % alienPos.size();
			bombPos[nbrBombs].x = alienPos[alien].x;//specify wich alien
			bombPos[nbrBombs].y = alienPos[alien].y + 64;
			nbrBombs++;
		}
	
		//move bombs
	/*	for (int i = 0; i < nbrBombs; i++){
			bombPos[i].y += move;
			if (bombPos[i].y > screenY ){//remove rockets outside screen
					nbrBombs--;
					bombPos[i] = bombPos[nbrBombs];
			}

			//collision detection
			if (bombPos[i].x + 16 > playerPos.x &&    bombPos[i].x + 16 < playerPos.x + 32
				&& bombPos[i].y + 16 > playerPos.y && bombPos[i].y + 16 < playerPos.y + 32){
				lives--;
				bombPos[i] = bombPos[nbrBombs - 1];
				nbrBombs--;
			}
		}
		
		/* Detect collisions
		rocket alien
		player alien 
		bomb player*/



	}
	alien1->destroy();
	rocket->destroy();
	sprite->destroy();
	system->destroy();
	return 0;
}



