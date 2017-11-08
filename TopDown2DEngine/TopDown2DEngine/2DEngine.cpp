#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <stdio.h>
//#include <io.h>    // for _setmode()
//#include <fcntl.h> // for _O_U16TEXT
#include <conio.h>
#include <Windows.h>
#include <chrono>
#include <fstream>

using namespace std;
using namespace std::chrono;

//enum keyCode {
//	KEY_UP = 72, KEY_DOWN = 80, KEY_LEFT = 75, KEY_RIGHT = 77,
//	KEY_W = 119, KEY_S = 115, KEY_A = 97, KEY_D = 100,
//	KEY_SPACE = 32, KEY_F = 102, KEY_ESC = 27
//};


enum vkKeyCode {
	VKKEY_UP = VK_UP, VKKEY_DOWN = VK_DOWN, VKKEY_LEFT = VK_LEFT, VKKEY_RIGHT = VK_RIGHT,
	VKKEY_W = 0x57, VKKEY_S = 0x53, VKKEY_A = 0x41, VKKEY_D = 0x44, VKKEY_E = 0x45,
	VKKEY_SPACE = VK_SPACE, VKKEY_F = 102, VKKEY_ESC = VK_ESCAPE
};

enum dirPress { UP , DOWN, LEFT, RIGHT };
int lastDirPress;

enum mapObject { 
	MAP_FLOOR = ' ',
	MAP_CHEST = 0x25A1,			// 0x25A1 = □
	MAP_CHARACTER = 0x263B,		// 0x263B = ☻
	MAP_ENEMY = 0x263A,			// 0x263A = ☺
	MAP_SPAWNPLAYER = 0x25CA,	// 0x25CA = ◊
	MAP_PORTAL1 = 0x25D9,		// 0x25D9 = ◙
	MAP_PORTAL2 = 0x25D8,		// 0x25D8 = ◘
	MAP_PORTAL3 = 0x25CF,		// 0x25CF = ●
	MAP_PORTALNEXT = 0x263C,	// 0x263C = ☼
	MAP_KEY = 0x25E6,			// 0x25E6 = ◦
	MAP_PICKUP = 0x25AB,		// 0x25AB =	▫
	MAP_LOCKED_DOOR = 0x25AC,	// 0x25AC = ▬
	MAP_WATER = 0x2592,			// 0x2592 = ▒
	MAP_DIRT = 0x2591,			// 0x2591 = ░
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
	NEXTLEVEL = -1,
	E1M0 = 0,
	E1M1,
	E1M2,
	E1M3, 
};

enum mapText{TEXT_ZERO = '0', TEXT_ONE, TEXT_TWO, TEXT_THREE, TEXT_FOUR, TEXT_FIVE, TEXT_SIX, TEXT_SEVEN, TEXT_EIGHT, TEXT_NINE, };
const int WIDTH = 100;
const int HEIGHT = 100;
const int ENGINE_MAX_LEVEL = 100;
int mapH, mapW, gatheredItem = 0, keysGathered = 0, currentLevel = 0;
wchar_t position[HEIGHT][WIDTH] = { {},{} };



long characterSpeed = 50;
__int64 characterNextMove = characterSpeed;


int character[2] = { 1,1 }; // Coordinate X, Y
bool keyPosition[HEIGHT][WIDTH] = { {},{} };

bool Attacked = false, continuePlaying = true;

random_device random;
__int64 currentFrameTime, nextFrameTime;

void attack(bool);
void changeColor(WORD,int,int);
void clearScreen();
void clearBox();
void detectOldPosition();
void drawScreen();
bool frameRate(int);
void hitCollision(bool, int, int, wchar_t);
//int keyCheck();
bool loadMap(int);
bool moveCharacter(int, int);
void showConsoleCursor(bool);
bool spwanPlayer();
bool vKControl();
void displayMessage(string);
int loadMapText();
void action();
bool loadGameInfo(string);

// Font
CONSOLE_FONT_INFOEX cfi, test;
// Windows Size
int columns, rows;

// Load map Text
string mapFileNameText[ENGINE_MAX_LEVEL] = {}; 
string mapMessages[10];
wchar_t mapMessageChar[10];
int messagePosition[HEIGHT][WIDTH] = { {},{} };

// Game Information
int gameMaxLevel = 4;
const string GAME_INFO_TEXT_FILE = "GameInfo.txt";
string mapFileName[ENGINE_MAX_LEVEL] = {};

void main()
{
	
	// Font Size
	cfi.cbSize = sizeof(cfi);
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 28;
	//lstrcpyW(cfi.FaceName, L"Lucida Console");
	lstrcpyW(cfi.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
	GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &test);
	test.FaceName;
	// Full screen
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	//_setmode(_fileno(stdout), _O_WTEXT);
	if (!loadGameInfo(GAME_INFO_TEXT_FILE))
	{
		return;
	}
	if (!loadMap(E1M0))
	{
		return;
	}
	currentLevel = 0;

	do
	{
		if (frameRate(30))
		{
			showConsoleCursor(false);

			if (!vKControl())
			{
				return;
			}
			drawScreen();
		}
	} while (continuePlaying == true);

}

void clearScreen()
{

	HANDLE hOut;
	COORD screenPosition;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	screenPosition.X = 0;
	screenPosition.Y = 0;
	SetConsoleCursorPosition(hOut, screenPosition);
	
}

void clearBox()
{
	HANDLE hOut;
	COORD screenPosition;
	DWORD Written;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	screenPosition.X = 0;
	screenPosition.Y = 0;
	FillConsoleOutputCharacter(hOut, ' ', 1000000, screenPosition, &Written);

	SetConsoleCursorPosition(hOut, screenPosition);
	
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			changeColor(15, j, i);
		}
	}
}

bool frameRate(int rate)
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

bool vKControl()
{

	if (GetAsyncKeyState(VKKEY_UP)|| GetAsyncKeyState(VKKEY_W))
	{
		lastDirPress = UP;
		if (!moveCharacter(0, -1))
		{
			return false;
		}
	}

	

	if (GetAsyncKeyState(VK_SHIFT))
	{
		characterSpeed = 50;
	}
	else
	{
		characterSpeed = 150;
	}

	if (GetAsyncKeyState(VKKEY_DOWN) || GetAsyncKeyState(VKKEY_S))
	{
		lastDirPress = DOWN;
		if(!moveCharacter(0, 1))
		{
		return false;
		}
	}

	if (GetAsyncKeyState(VKKEY_LEFT) || GetAsyncKeyState(VKKEY_A))
	{
		lastDirPress = LEFT;
		if(!moveCharacter(-1, 0))
		{
		return false;
		}
	}

	if (GetAsyncKeyState(VKKEY_RIGHT) || GetAsyncKeyState(VKKEY_D))
	{
		lastDirPress = RIGHT;
		if(!moveCharacter(1, 0))
		{
		return false;
		}
	}

	if (GetAsyncKeyState(VKKEY_SPACE))
	{
		attack(true);
	}

	if (GetAsyncKeyState(VKKEY_E))
	{
		action();
		//displayMessage("THIS IS WORKING FINE");
	}

	if (GetAsyncKeyState(VKKEY_ESC))
	{
		continuePlaying = false;
	}


	return true;
}

void drawScreen()
{
			
	//system("cls");
	clearScreen();
	HANDLE hOut;
	COORD screenPosition;
	DWORD Written;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	int locX, locY;
	for (int i = 0; i < mapH; i++)
	{
		for (int j = 0; j < mapW; j++)
		{		
			locX = j + ((columns / 2) - (mapW / 2)) + 1 /*- character[1] + (mapW / 2)*/;	// uncomment to move map around character
			locY = i + ((rows / 2) - (mapH / 2)) /*- character[0] + (mapH / 2)*/;	// uncomment to move map around character
			screenPosition.X = locX;
			screenPosition.Y = locY;
			FillConsoleOutputCharacter(hOut, position[i][j], 1, screenPosition, &Written);
			//wcout << position[i][j];

			// Apply colors
			switch (position[i][j])
			{
			case MAP_KEY:
			case MAP_LOCKED_DOOR:
				changeColor(140, locY, locX);
				break;
			case MAP_PICKUP:
			case MAP_CHEST:
				changeColor(138, locY, locX);
				break;
			case MAP_CHARACTER:
				changeColor(142, locY, locX);
				break;
			case MAP_ENEMY:
				changeColor(137, locY, locX);
				break;
			case MAP_PORTALNEXT:
				changeColor(133, locY, locX);
				break;
			case SWORD_UP: 		
			case SWORD_DOWN:
			case SWORD_LEFT:
			case SWORD_RIGHT:
				changeColor(135, locY, locX);
				break;
			case MAP_WATER:
				changeColor(25, locY, locX);
				break;
			case MAP_DIRT:
				changeColor(4, locY, locX);
				break;
			default:
				changeColor(143, locY, locX);
				break;
			}
			if (i == 0)
			{
				screenPosition.X = locX;
				screenPosition.Y = locY - 1;
				FillConsoleOutputCharacter(hOut, ' ', 1, screenPosition, &Written);
				changeColor(15, locY-1, locX);
			}
			if (i == mapH - 1)
			{
				screenPosition.X = locX;
				screenPosition.Y = locY + 1;
				FillConsoleOutputCharacter(hOut, ' ', 1, screenPosition, &Written);
				changeColor(15, locY+1, locX);
			}
			if (j == 0)
			{
				screenPosition.Y = locY;
				screenPosition.X = locX-1;
				FillConsoleOutputCharacter(hOut, ' ', 1, screenPosition, &Written);
				changeColor(15, locY, locX-1);
			}
			if (j == mapW - 1)
			{
				screenPosition.Y = locY;
				screenPosition.X = locX +1;
				FillConsoleOutputCharacter(hOut, ' ', 1, screenPosition, &Written);
				changeColor(15, locY, locX + 1);
			}
		}
		//wcout << endl;
	}
	cout << "Level: " << currentLevel << endl;
	cout << "Gather: " << gatheredItem << "\tKey(s): " << keysGathered << endl;
	cout << "Colums: " << columns << "\tCenter: " << ((columns / 2) + (mapW / 2)) + 1 << endl;
	cout << "Rows: " << rows << "\tCenter: " << ((rows / 2) - (mapH / 2)) << endl;
	//attack(false);
	if (Attacked)
	{
		attack(false);
	}

}

void changeColor(WORD colorSelected, int poxY, int posX)
{
	WORD write = colorSelected;
	COORD c = { posX, poxY };
	DWORD written;
	WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), &write, 1, c, &written);
}

//int keyCheck()
//{
//	int getKey = _getch();
//	if (getKey == 0 || getKey == 224)
//	{
//		getKey = _getch();
//	}
//	return getKey;
//}

bool spwanPlayer()
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
				return true;
			}
		}
	}

	system("cls");
	cout << endl << endl;
	cout << "\tno Spawn Point found!" << endl;
	system("pause");
	return false;
}

bool moveCharacter(int y, int x)
{
	if (currentFrameTime <= characterNextMove)
	{
		return true;
	}
	characterNextMove = currentFrameTime + characterSpeed;

	wchar_t futurePosition = position[character[0] + x][character[1] + y];
	switch (futurePosition)
	{
	case MAP_FLOOR:
		detectOldPosition();
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
		attack(true);
		break;
	case MAP_KEY:
		attack(true);
		detectOldPosition();
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		break;
	case MAP_LOCKED_DOOR:
		if (keysGathered > 0)
		{
			detectOldPosition();
			character[0] += x;
			character[1] += y;
			position[character[0]][character[1]] = MAP_CHARACTER;
			keysGathered--;
		}
		break;
	case MAP_PICKUP:
		detectOldPosition();
		character[0] += x;
		character[1] += y;
		position[character[0]][character[1]] = MAP_CHARACTER;
		gatheredItem++;
		break;
	case MAP_PORTAL1:
		loadMap(E1M1);
		spwanPlayer();
		break;
	case MAP_PORTAL2:
		loadMap(E1M2);
		spwanPlayer();
		break;
	case MAP_PORTAL3:
		loadMap(E1M3);
		spwanPlayer();
		break;
	case MAP_PORTALNEXT:	
		currentLevel++;
		if (!loadMap(NEXTLEVEL))
		{
			return false;
		}
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
	return true;
}

void detectOldPosition()
{
	if (position[character[0]][character[1]] != MAP_CEILING &&
		position[character[0]][character[1]] != MAP_CEILING_TOP_END &&
		position[character[0]][character[1]] != MAP_CEILING_BOTTOM_END)
	{
		position[character[0]][character[1]] = MAP_FLOOR;
	}
}

void hitCollision(bool sword,int dirX,int dirY, wchar_t swordDir)
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
		else if (position[character[0] + dirX][character[1] + dirY] == MAP_PICKUP)
		{
			position[character[0] + dirX][character[1] + dirY] = MAP_FLOOR;
			gatheredItem++;
		}
	}
	else
	{
		position[character[0] + dirX][character[1] + dirY] = MAP_FLOOR;
		Attacked = false;
	}
}

void attack(bool sword)
{
	switch (lastDirPress)
	{
	case UP:
		hitCollision(sword, -1, 0, SWORD_UP);
		break;
	case DOWN:
		hitCollision(sword, +1, 0, SWORD_DOWN);
		break;
	case LEFT:
		hitCollision(sword, 0, -1, SWORD_LEFT);
		break;
	case RIGHT:
		hitCollision(sword, 0, +1, SWORD_RIGHT);
		break;
	default:
		break;
	}
}

void action()
{
	switch (lastDirPress)
	{
	case UP:
		if (messagePosition[character[0] - 1][character[1]] != 0 )
		{
			displayMessage(mapMessages[(messagePosition[character[0] - 1][character[1]])- 1]);
		}
		break;
	case DOWN:
		if (messagePosition[character[0] + 1][character[1]] != 0)
		{
			displayMessage(mapMessages[(messagePosition[character[0] + 1][character[1]]) - 1]);
		}
		break;
	case LEFT:
		if (messagePosition[character[0]][character[1]- 1] != 0)
		{
			displayMessage(mapMessages[(messagePosition[character[0]][character[1] - 1]) - 1]);
		}
		break;
	case RIGHT:
		if (messagePosition[character[0]][character[1] + 1] != 0)
		{
			displayMessage(mapMessages[(messagePosition[character[0]][character[1] + 1]) - 1]);
		}
		break;
	default:
		break;
	}
}

void showConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(out, &cursorInfo);
}

void displayMessage(string message)
{
	int messageLenght = message.length();

	//clearScreen();
	HANDLE hOut;
	COORD screenPosition;
	DWORD Written;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	int locX, locY;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < messageLenght; j++)
		{
			locX = j + ((columns / 2) - (messageLenght / 2)) + 1;
			locY = i + (rows / 2);	
			screenPosition.X = locX;
			screenPosition.Y = locY;
			if (i == 1)
			{
				FillConsoleOutputCharacter(hOut, message[j], 1, screenPosition, &Written);
				changeColor(15, locY, locX);
			}

			if (i == 0 || i == 2)
			{
				FillConsoleOutputCharacter(hOut, MAP_FLOOR, 1, screenPosition, &Written);
				changeColor(15, locY, locX);

			}
			
			if (j == 0 )
			{
				screenPosition.X -= 1;
				FillConsoleOutputCharacter(hOut, MAP_FLOOR, 1, screenPosition, &Written);
				changeColor(15, locY, locX-1);
			}
			else if (j == messageLenght - 1)
			{
				screenPosition.X += 1;
				FillConsoleOutputCharacter(hOut, MAP_FLOOR, 1, screenPosition, &Written);
				changeColor(15, locY, locX+1);

			}
		}
	}
	system("pause");
	system("cls");
}

bool loadGameInfo(string gameInfoTextFile)
{
	string line;
	ifstream gameListFile(gameInfoTextFile);
	if (gameListFile.is_open())
	{
		getline(gameListFile, line);
		gameMaxLevel = stoi(line);

		for (int i = 0; i < gameMaxLevel; i++)
		{
			getline(gameListFile, line); // mapFileName[ENGINE_MAX_LEVEL] , mapFileNameText[ENGINE_MAX_LEVEL]
			mapFileName[i] = line;
		}

		int lineNumber = 0;
		while (getline(gameListFile, line))
		{
			mapFileNameText[lineNumber++] = line;
		}

		//for (int i = 0; i < gameMaxLevel; i++)
		//{
		//	getline(gameListFile, line); // mapFileName[ENGINE_MAX_LEVEL] , mapFileNameText[ENGINE_MAX_LEVEL]
		//	mapFileNameText[i] = line;
		//}
		gameListFile.close();
	}
	else
	{
		cout << "Missing " << gameInfoTextFile << " to load game." << endl;
		system("pause");
		return 0;
	}

	return true;
}

bool loadMap(int fileName)
{	
	system("cls");
	FILE * gameMapFile;
	errno_t err;
	if (fileName == -1)
	{
		if (currentLevel >= gameMaxLevel)
		{
			currentLevel = 0;
		}
		fileName = currentLevel;
	}
	int qtyOfMessage = loadMapText();
	clearBox();
	// Open map file to load
	err = fopen_s(&gameMapFile, mapFileName[fileName].c_str(), "r, ccs=UNICODE");
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

				if (qtyOfMessage != 0)
				{
					if (position[i][j] == TEXT_ZERO)
					{
						position[i][j] = mapMessageChar[0];
						messagePosition[i][j] = 1;
					}
					else if (position[i][j] == TEXT_ONE)
					{
						position[i][j] = mapMessageChar[1];
						messagePosition[i][j] = 2;
					}
					else if (position[i][j] == TEXT_TWO)
					{
						position[i][j] = mapMessageChar[2];
						messagePosition[i][j] = 3;
					}
					else if (position[i][j] == TEXT_THREE)
					{
						position[i][j] = mapMessageChar[3];
						messagePosition[i][j] = 4;
					}
					else if (position[i][j] == TEXT_FOUR)
					{
						position[i][j] = mapMessageChar[4];
						messagePosition[i][j] = 5;
					}
					else if (position[i][j] == TEXT_FIVE)
					{
						position[i][j] = mapMessageChar[5];
						messagePosition[i][j] = 6;
					}
					else if (position[i][j] == TEXT_SIX)
					{
						position[i][j] = mapMessageChar[6];
						messagePosition[i][j] = 7;
					}
					else if (position[i][j] == TEXT_SEVEN)
					{
						position[i][j] = mapMessageChar[7];
						messagePosition[i][j] = 8;
					}
					else if (position[i][j] == TEXT_EIGHT)
					{
						position[i][j] = mapMessageChar[8];
						messagePosition[i][j] = 9;
					}
					else if (position[i][j] == TEXT_NINE)
					{
						position[i][j] = mapMessageChar[9];
						messagePosition[i][j] = 10;
					}
				}

			}
		}

		fclose(gameMapFile);
		if (!spwanPlayer())
		{
			return false;
		}
	}
	else
	{
		cout << "Unable to open " << (mapFileName[fileName] !=""? mapFileName[fileName]:"next level") << " file!" << endl;
		system("pause");
		return false;
	}
	return true;
	//SetWindowPos(GetConsoleWindow(), 0, 0, 0, mapW*25, (mapH+2)*25, SWP_SHOWWINDOW | SWP_NOMOVE);
}

int loadMapText()
{
	// Reset map Text variable
	for (int i = 0; i < 10; i++)
	{
		mapMessages[i] = "";
		mapMessageChar[i] = ' ';
	}
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			messagePosition[i][j] = 0;
		}
	}

	int qtyOfMessage;
	string line;
	ifstream gameListFile(mapFileNameText[currentLevel]);

	if (gameListFile.is_open())
	{
		getline(gameListFile, line);
		qtyOfMessage = stoi(line);

		for (int i = 0; i < qtyOfMessage; i++)
		{
			getline(gameListFile, line);
			mapMessageChar[i] = stoi(line, nullptr, 16);
			getline(gameListFile, line);
			mapMessages[i] = line;
		}
		gameListFile.close();
	}
	else
	{
		cout << "Unable to open " << (mapFileNameText[currentLevel] != "" ? mapFileNameText[currentLevel] : "next level message") << " file!" << endl;
		system("pause");
		return 0;
	}

	
	return qtyOfMessage;
}