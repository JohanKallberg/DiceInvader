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

struct Alien
{
	Alien(Vec2f posin, const ISprite &spritein) :pos(posin), sprite(spritein){};
	Vec2f pos;
	const ISprite &sprite;
	
};
class AlienArmy{ //we don't have a matrix, lines are hard to keep track of
public:
	
	std::vector<Vec2f> alienPos;
	AlienArmy(int aliensRow = 8,int aliensCol = 2){
		for (int i = 0; i < aliensCol; i++){
			for (int j = 0; j < aliensRow; j++){
				alienPos.push_back(Vec2f(j * 64, i * 64));
			}
		}
	}
	void CalculareMovement(){}
	void collisionDetection(){}
	//speed
	int rightAlien = alienPos.size() - 1;
	int leftAlien = 0;
	float alienDirection = 1.0f;
	Vec2f alienMovement = Vec2f(0, 0);
};

static void CreateArmy(std::vector<Alien> &aliens, const ISprite& alien1, const ISprite& alien2){
	for (int i = 0; i < 4; i++){//row
		for (int j = 0; j < 8; j++){//col
			if (i<2)
				aliens.push_back(Alien(Vec2f(j * 64, i * 64),alien2));
			else
				aliens.push_back(Alien(Vec2f(j * 64, i * 64), alien1));
		}
	}
}
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
	
	//initiate player
	Vec2f playerPos = Vec2f(screenX / 2, screenY - 32);
	float lastTime = system->getElapsedTime();
	std::vector<Vec2f> rocketPos;
	static int maxNbrRockets = 6;

	//initiate alienArmy
	int aliensRow = 8;
	int aliensCol =2;
	std::vector<Vec2f> alienPos;
	std::vector<Alien> aliens;
	for (int i = 0; i < aliensCol; i++){
		for (int j = 0; j < aliensRow; j++){
			alienPos.push_back(Vec2f(j * 64, i * 64));
		}
	}
	CreateArmy(aliens,*alien1,*alien2);
	int rightAlien = alienPos.size() - 1;
	int leftAlien = 0;
	float alienDirection = 1.0f;
	Vec2f alienMovement = Vec2f(0,0);
	

	//int nbrBombs = 0;
	std::vector<Vec2f> bombPos;
	static int maxNbrBombs = 6;
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
		for (int i = 0; i < bombPos.size(); i++){
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
			}

		}
		if (keys.fire&&rocketPos.size() < maxNbrRockets){
			if (lastRocketFireTime + 0.5f < newTime){
				lastRocketFireTime = newTime;
					rocketPos.push_back(Vec2f(playerPos.x, playerPos.y - 32));
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
		for (auto it = alienPos.begin(); it < alienPos.end(); ++it){
			//move
			(*it) = (*it) + alienMovement;
			if ((*it).y > screenY){
				//game over
				break;
			}
		}
		//Split loops as a fix to a bug where the position whould randomly change on collision
		for (auto it = alienPos.begin(); it < alienPos.end(); ++it){
			//detect collision
			for (int j = 0; j< rocketPos.size(); j++){
				if (rocketPos[j].x + 16 >(*it).x &&    rocketPos[j].x + 16 < (*it).x + 32
					&& rocketPos[j].y + 16 > (*it).y && rocketPos[j].y + 16 <(*it).y + 32){
					it =alienPos.erase(it); 
					rocketPos.erase(rocketPos.begin() + j);
				}
			}
			if (it != alienPos.end()&&(playerPos.x + 16 > (*it).x &&    playerPos.x + 16 < (*it).x + 32
				&& playerPos.y< (*it).y))
			{
				lives--;
				it = alienPos.erase(it);
			}
			if (it == alienPos.end())
				break;//if we removed last object in vector
		}
		

		//spawn bombs
		int random = rand()%100000;
		if (random == 1 && bombPos.size()<maxNbrBombs){
			int alien = rand() % alienPos.size();
			bombPos.push_back(alienPos[alien]);
		}
	
		//move bombs
		for (int i = 0; i < bombPos.size(); i++){
			bombPos[i].y += move;

			//collision detection
			if (bombPos[i].x + 16 > playerPos.x &&    bombPos[i].x + 16 < playerPos.x + 32
				&& bombPos[i].y + 16 > playerPos.y && bombPos[i].y + 16 < playerPos.y + 32){
				lives--;
				bombPos.erase(bombPos.begin() + i);
			}

			else if (bombPos[i].y > screenY ){//remove rockets outside screen
				bombPos.erase(bombPos.begin() + i);
			}

			
		}

	}
	alien1->destroy();
	rocket->destroy();
	sprite->destroy();
	system->destroy();
	return 0;
}



