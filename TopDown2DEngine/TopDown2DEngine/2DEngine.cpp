#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <stdio.h>
#include <io.h>    // for _setmode()
#include <fcntl.h> // for _O_U16TEXT
#include <conio.h>
#include <Windows.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

enum keyCode {
	KEY_UP = 72, KEY_DOWN = 80, KEY_LEFT = 75, KEY_RIGHT = 77,
	KEY_W = 119, KEY_S = 115, KEY_A = 97, KEY_D = 100,
	KEY_SPACE = 32, KEY_F = 102, KEY_ESC = 27
};


enum vkKeyCode {
	VKKEY_UP = VK_UP, VKKEY_DOWN = VK_DOWN, VKKEY_LEFT = VK_LEFT, VKKEY_RIGHT = VK_RIGHT,
	VKKEY_W = 0x57, VKKEY_S = 0x53, VKKEY_A = 0x41, VKKEY_D = 0x44,
	VKKEY_SPACE = VK_SPACE, VKKEY_F = 102, VKKEY_ESC = VK_ESCAPE
};

enum dirPress { UP , DOWN, LEFT, RIGHT };
int lastDirPress;

enum mapObject { 
	MAP_FLOOR = ' ',
	MAP_CHEST = 0x25A1,			// 0x25A1 = □
	MAP_CHARACTER = 0x263A,		// 0x263A = ☺
	MAP_SPAWNPLAYER = 0x25CA,	// 0x25CA = ◊
	MAP_PORTAL1 = 0x25D9,		// 0x25D9 = ◙
	MAP_PORTAL2 = 0x25D8,		// 0x25D8 = ◘
	MAP_PORTAL3 = 0x25CF,		// 0x25CF = ●
	MAP_PORTALNEXT = 0x263C,	// 0x263C = ☼
	MAP_KEY = 0x25E6,			// 0x25E6 = ◦
	MAP_PICKUP = 0x25AB,			// 0x25AB =	▫
	MAP_LOCKED_DOOR = 0x25AC	// 0x25AC = ▬
};

enum mapCeiling {
	MAP_CEILING = 0x2588,			// 0x2588 = █
	MAP_CEILING_TOP_END = 0x2584,	// 0x2584 = ▄ 
	MAP_CEILING_BOTTOM_END = 0x2580,// 0x2580 = ▀
};

enum attack {
	SWORD_UP = 0x25B2, 			// 0x25B2 = ▲
	SWORD_DOWN = 0x25BC,		// 0x25BC =	▼
	SWORD_LEFT = 0x25C4,		// 0x25C4 =	◄
	SWORD_RIGHT = 0x25BA		// 0x25BA =	►
};

enum mapFile {
	E1M1 = 0,
	E1M2 = 1,
	E1M3 = 2,
	NEXTLEVEL = -1
};

char * mapFileName[3] = { "gameMapE1M1.txt","gameMapE1M2.txt", "gameMapE1M3.txt" };

const int WIDTH = 100;
const int HEIGHT = 100;
const int MAX_LEVEL = 3;
int mapH, mapW, gatheredItem = 0, keysGathered = 0, currentLevel = 0;
wchar_t position[HEIGHT][WIDTH] = { {},{} };
int character[2] = { 1,1 };
bool keyPosition[HEIGHT][WIDTH] = { {},{} };

bool Attacked = false, continuePlaying = true;

random_device random;
__int64 currentFrameTime, nextFrameTime;

void clearScreen();
void clearBox();
void DrawScreen();
bool FrameRate(int);
void VKControl();
void Control();
int KeyCheck();
void SpwanPlayer();
void LoadMap(int);
void MoveCharacter(int, int);
void DetectOldPosition();
void ShowConsoleCursor(bool);
void Attack(bool);
void HitCollision(bool, int, int, wchar_t);
void changeColor(WORD,int,int);

void main()
{
	_setmode(_fileno(stdout), _O_WTEXT);
	ShowConsoleCursor(false);
	LoadMap(E1M1);
	currentLevel = 0;
	SpwanPlayer();

	do
	{
		if (FrameRate(5))
		{
			VKControl();
			DrawScreen();	
		}
	
		//Control();
	} while (continuePlaying == true);
}

void clearScreen()
{

	HANDLE hOut;
	COORD Position;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	Position.X = 0;
	Position.Y = 0;
	SetConsoleCursorPosition(hOut, Position);
}

void clearBox()
{
	HANDLE hOut;
	COORD Position;
	DWORD Written;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	Position.X = 0;
	Position.Y = 0;
	FillConsoleOutputCharacter(hOut, ' ', 1000000, Position, &Written);

	SetConsoleCursorPosition(hOut, Position);
}
bool FrameRate(int rate)
{
	currentFrameTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	
	if (currentFrameTime >= nextFrameTime)
	{
		nextFrameTime = currentFrameTime + 1000 / rate;
		
		return true;
	}

	//wcout << "miss frame" << endl;
	return false;
}

void VKControl()
{

	if (GetAsyncKeyState(VKKEY_UP)|| GetAsyncKeyState(VKKEY_W))
	{
		lastDirPress = UP;
		MoveCharacter(0, -1);
	}

	if (GetAsyncKeyState(VKKEY_DOWN) || GetAsyncKeyState(VKKEY_S))
	{
		lastDirPress = DOWN;
		MoveCharacter(0, 1);
	}

	if (GetAsyncKeyState(VKKEY_LEFT) || GetAsyncKeyState(VKKEY_A))
	{
		lastDirPress = LEFT;
		MoveCharacter(-1, 0);
	}

	if (GetAsyncKeyState(VKKEY_RIGHT) || GetAsyncKeyState(VKKEY_D))
	{
		lastDirPress = RIGHT;
		MoveCharacter(1, 0);
	}

	if (GetAsyncKeyState(VKKEY_SPACE))
	{
		Attack(true);
	}

	if (GetAsyncKeyState(VKKEY_ESC))
	{
		continuePlaying = false;
	}
}

void Control()
{
	switch (KeyCheck())
	{
	case KEY_UP:
	case KEY_W:
		lastDirPress = UP;
		MoveCharacter(0, -1);
		break;
	case KEY_DOWN:
	case KEY_S:
		lastDirPress = DOWN;
		MoveCharacter(0, 1);
		break;
	case KEY_LEFT:
	case KEY_A:
		lastDirPress = LEFT;
		MoveCharacter(-1, 0);
		break;
	case KEY_RIGHT:
	case KEY_D:
		lastDirPress = RIGHT;
		MoveCharacter(1, 0);
		break;
	case KEY_SPACE:
		Attack(true);
		break;
	case KEY_F:
		break;
	case KEY_ESC:
		return;
		break;
	default:

		break;
	}
}

void DrawScreen()
{
	//system("cls");
	clearScreen();
	
	for (int i = 0; i < mapH; i++)
	{
		for (int j = 0; j < mapW; j++)
		{
			wcout << position[i][j];
			// Apply colors
			switch (position[i][j])
			{
			case MAP_KEY:
			case MAP_LOCKED_DOOR:
				changeColor(140, j, i);
				break;
			case MAP_PICKUP:
			case MAP_CHEST:
				changeColor(138, j, i);
				break;
			case MAP_CHARACTER:
				changeColor(142, j, i);
				break;
			case MAP_PORTALNEXT:
				changeColor(133, j, i);
				break;
			case SWORD_UP: 		
			case SWORD_DOWN:
			case SWORD_LEFT:
			case SWORD_RIGHT:
				changeColor(135, j, i);
				break;
			default:
				changeColor(143, j, i);
				break;
			}			
		}
		wcout << endl;
	}
	wcout << "Gather: " << gatheredItem << "\tKey(s): " << keysGathered << endl;

	//Attack(false);
	if (Attacked)
	{
		Attack(false);
	}

	/*HANDLE hOut;
	COORD Position;
	DWORD Written;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	Position.X = 11;
	Position.Y = 11;
	FillConsoleOutputCharacter(hOut, 'D', 1, Position, &Written);
	Position.X = 12;
	Position.Y = 11;
	FillConsoleOutputCharacter(hOut, 'D', 1, Position, &Written);*/
}

void changeColor(WORD colorSelected, int posX, int poxY)
{
	WORD write = colorSelected;
	COORD c = { posX, poxY };
	DWORD written;
	WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), &write, 1, c, &written);

}

int KeyCheck()
{
	int getKey = _getch();
	if (getKey == 0 || getKey == 224)
	{
		getKey = _getch();
	}
	return getKey;
}

void SpwanPlayer()
{
	int x;
	int y;

	for (int i = 0; i < mapH; i++)
	{
		for (int j = 0; j < mapW; j++)
		{
			if (position[i][j] == MAP_SPAWNPLAYER)
			{
				character[0] = i;
				character[1] = j;
				position[character[0]][character[1]] = MAP_CHARACTER;
				return;
			}
		}
	}

	system("cls");
	wcout << endl << endl;
	wcout << "\tno Spawn Point found!" << endl;
	system("pause");
}

void LoadMap(int fileName)
{
	currentLevel++;

	clearBox();
	FILE * gameMapFile;
	errno_t err;
	if (fileName == -1)
	{
		if (currentLevel >= MAX_LEVEL)
		{
			currentLevel = 0;
		}
		fileName = currentLevel;
	}
	err = fopen_s(&gameMapFile, mapFileName[fileName], "r, ccs=UNICODE");
	if (err == 0)
	{
		mapH = (fgetwc(gameMapFile) & 0xf) * (fgetwc(gameMapFile) & 0xf);
		mapW = (fgetwc(gameMapFile) & 0xf) * (fgetwc(gameMapFile) & 0xf);
		fgetwc(gameMapFile);

		for (int i = 0; i < mapH; i++)
		{
			for (int j = 0; j < mapW; j++)
			{
				position[i][j] = fgetwc(gameMapFile);
				if (position[i][j] == '\n')
				{
					position[i][j] = fgetwc(gameMapFile);
				}
				if (position[i][j] == MAP_KEY)
				{
					keyPosition[i][j] = true;
					position[i][j] = MAP_CHEST;
				}
			}
		}

		fclose(gameMapFile);
	}
	else
	{
		cout << "Unable to open file!" << endl;
		system("pause");
	}

}

void MoveCharacter(int y, int x)
{
	wchar_t futurePosition = position[character[0] + x][character[1] + y];
	switch (futurePosition)
	{
	case MAP_FLOOR:
		DetectOldPosition();
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		break;
	case MAP_CHEST:
		/*position[character[0]][character[1]] = MAP_FLOOR;
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		gatheredItem++;*/
		Attack(true);
		break;
	case MAP_KEY:
		Attack(true);
		DetectOldPosition();
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		break;
	case MAP_LOCKED_DOOR:
		if (keysGathered > 0)
		{
			DetectOldPosition();
			character[0] += x;
			character[1] += y;
			position[character[0]][character[1]] = MAP_CHARACTER;
			keysGathered--;
		}
		break;
	case MAP_PICKUP:
		DetectOldPosition();
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		gatheredItem++;
		break;
	case MAP_PORTAL1:
		LoadMap(E1M1);
		SpwanPlayer();
		break;
	case MAP_PORTAL2:
		LoadMap(E1M2);
		SpwanPlayer();
		break;
	case MAP_PORTAL3:
		LoadMap(E1M3);
		SpwanPlayer();
		break;
	case MAP_PORTALNEXT:	
		LoadMap(NEXTLEVEL);
		SpwanPlayer();
		break;
	case MAP_CEILING:
	case MAP_CEILING_TOP_END: 
	case MAP_CEILING_BOTTOM_END:
		if (position[character[0]][character[1]] == MAP_CHARACTER)
		{
			position[character[0]][character[1]] = MAP_FLOOR;
		}
		character[0] += x;
		character[1] += y;
		break;

	default:
		break;
	}
}

void DetectOldPosition()
{
	if (position[character[0]][character[1]] != MAP_CEILING &&
		position[character[0]][character[1]] != MAP_CEILING_TOP_END &&
		position[character[0]][character[1]] != MAP_CEILING_BOTTOM_END)
	{
		position[character[0]][character[1]] = MAP_FLOOR;
	}
}

void HitCollision(bool sword,int dirX,int dirY, wchar_t swordDir)
{
	if (sword)
	{
		if (position[character[0] + dirX][character[1] + dirY] == MAP_FLOOR)
		{
			position[character[0] + dirX][character[1] + dirY] = swordDir;
			Attacked = true;
		}
		else if (position[character[0] + dirX][character[1] + dirY] == MAP_CHEST)
		{
			if (keyPosition[character[0] + dirX][character[1] + dirY] == true)
			{
				position[character[0] + dirX][character[1] + dirY] = MAP_KEY;
			}
			else
			{
				position[character[0] + dirX][character[1] + dirY] = MAP_PICKUP;
			}
		}
		else if (position[character[0] + dirX][character[1] + dirY] == MAP_KEY)
		{
			position[character[0] + dirX][character[1] + dirY] = MAP_FLOOR;
			keysGathered++;
		}
	}
	else
	{
		position[character[0] + dirX][character[1] + dirY] = MAP_FLOOR;
		Attacked = false;
	}
}
void Attack(bool sword)
{
	switch (lastDirPress)
	{
	case UP:
		HitCollision(sword, -1, 0, SWORD_UP);
		break;
	case DOWN:
		HitCollision(sword, +1, 0, SWORD_DOWN);
		break;
	case LEFT:
		HitCollision(sword, 0, -1, SWORD_LEFT);
		break;
	case RIGHT:
		HitCollision(sword, 0, +1, SWORD_RIGHT);
		break;
	default:
		break;
	}
}

void ShowConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(out, &cursorInfo);
}

