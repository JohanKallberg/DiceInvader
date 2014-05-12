#include <windows.h>
#include <cassert>
#include <cstdio>
#include "DiceInvaders.h"

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
	ISprite* bomb = system->createSprite("data/bomb.bmp");
	float horizontalPosition = screenX/2;
	float lastTime = system->getElapsedTime();
	float rocketHorizontalPos[6];
	float rocketVerticalPos[6];
	int nbrRockets = 0;
	static int maxNbrRockets = 6;

	int nbrAliens = 5;
	float aliensHorizontalPos = nbrAliens*64;
	float aliensVerticalPos = 0;
	bool alienDirectionRight = true; 
	static int maxNbrAliens = 5;

	int nbrBombs = 0;
	float bombHorizontalPos[6];
	float bombVerticalPos[6];


	float lastRocketFireTime = lastTime;
	while (system->update())
	{
		sprite->draw(int(horizontalPosition), screenY - 32);
		for (int i = 0; i < nbrRockets; i++){
			rocket->draw(int(rocketHorizontalPos[i]), rocketVerticalPos[i]);
		}
		for (int i = 1; i<= nbrAliens; i++){
			alien1->draw(int(aliensHorizontalPos - i * 64), int(aliensVerticalPos));
		}
		for (int i = 0; i < nbrBombs; i++){
			bomb->draw(int(bombHorizontalPos[i]), bombVerticalPos[i]);
		}
		float newTime = system->getElapsedTime();
		float move = (newTime - lastTime) * 160.0f;
		lastTime = newTime;

		IDiceInvaders::KeyStatus keys;
		system->getKeyStatus(keys);
		if (keys.right){
			if (horizontalPosition +move +32 < screenX)
				horizontalPosition += move;
		}
		else if (keys.left){
			if (horizontalPosition -move > 0)
				horizontalPosition -= move;
		}
		//move rockets (before instantiating new ones)
		for (int i = 0; i < nbrRockets; i++){
			rocketVerticalPos[i] -= move;
			if (rocketVerticalPos[i] < 0){//remove the rocket at position i
				for (int j = i; j < nbrRockets; j++){
 					rocketVerticalPos[j] = rocketVerticalPos[j + 1];
					rocketHorizontalPos[j] = rocketHorizontalPos[j + 1];
					
				}
				nbrRockets--;
			}
			
		}
		if (keys.fire){
			if (lastRocketFireTime + 0.5f < newTime){
				lastRocketFireTime = newTime;
				if (nbrRockets < maxNbrRockets){
					nbrRockets++;
					rocketHorizontalPos[nbrRockets] = horizontalPosition;
					rocketVerticalPos[nbrRockets] = screenY - 64;
				}
			}
		}


		//move aliens
		if (alienDirectionRight)
			aliensHorizontalPos += move;
		else
			aliensHorizontalPos -= move;
		if (aliensHorizontalPos - 32 > screenX){
			aliensHorizontalPos -= move;
			alienDirectionRight = false;
			aliensVerticalPos += 32;
		}
		else if(aliensHorizontalPos-nbrAliens*64<0){
			aliensHorizontalPos += move;
			alienDirectionRight = true;
			aliensVerticalPos += 32;
		}
		if (aliensVerticalPos <= 0){
			//game over
		}

		//spawn bombs
		if (rand() % 100000000 == 1 && nbrBombs<6){
			nbrBombs++;
			bombHorizontalPos[nbrBombs] = aliensHorizontalPos;//specify wich alien
			bombVerticalPos[nbrBombs] = aliensVerticalPos+32;
		}
		
		//move bombs
		for (int i = 0; i < nbrBombs; i++){
			bombVerticalPos[i] += move;
			if (bombVerticalPos[i] > screenY){//remove the rocket at position i
				for (int j = i; j < nbrBombs; j++){
					bombVerticalPos[j] = bombVerticalPos[j + 1];
					bombVerticalPos[j] = bombVerticalPos[j + 1];
				}
				nbrBombs--;
			}

		}
	}
	alien1->destroy();
	rocket->destroy();
	sprite->destroy();
	system->destroy();

	return 0;
}



