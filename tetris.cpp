#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <Windows.h>

using namespace std;


int nScreenWidth = 80;			// 세로
int nScreenHeight = 30;			// 가로
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int Rotate(int x, int y, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0: // 0 도 회전				// 0  1  2  3
		pi = y * 4 + x;					// 4  5  6  7
		break;								// 8  9 10 11
												//12 13 14 15

	case 1:// 90 도 회전				//12  8  4  0
		pi = 12 + y - (x * 4);		//13  9  5  1
		break;								//14 10  6  2
												//15 11  7  3

	case 2: // 180 도 회전			//15 14 13 12
		pi = 15 - (y * 4) - x;			//11 10  9  8
		break;								// 7  6  5  4
												// 3  2  1  0

	case 3: // 270 도 회전			// 3  7 11 15
		pi = 3 - y + (x * 4);			// 2  6 10 14
		break;								// 1  5  9 13
	}											// 0  4  8 12

	return pi;
}

bool DoesPieceFit(int nT, int nR, int nPX, int nPY)
{

	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
		{
			
			int pi = Rotate(x, y, nR);

			
			int fi = (nPY + y) * nFieldWidth + (nPX + x);

			
			if (nPX + x >= 0 && nPX + x < nFieldWidth)
			{
				if (nPY + y >= 0 && nPY + y < nFieldHeight)
				{
					
					if (tetromino[nT][pi] != L'.' && pField[fi] != 0)
						return false; 
				}
			}
		}

	return true;
}

int main()
{
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// tetromino = 정사각형 4개 이용
	tetromino[0].append(L"..X...X...X...X."); // 이걸 4개씩 끊어서 하면 ㅣ 모양
	tetromino[1].append(L"..X..XX...X....."); // ㄱㄴ 모양 ( 합친거 )
	tetromino[2].append(L".....XX..XX....."); // 1번 반대모양
	tetromino[3].append(L"..X..XX..X......"); // ㅁ 모양
	tetromino[4].append(L".X...XX...X....."); // ㅏ 모양
	tetromino[5].append(L".X...X...XX....."); // ㄴ 모양
	tetromino[6].append(L"..X...X..XX....."); // 5번 반대 모양

	pField = new unsigned char[nFieldWidth * nFieldHeight]; 
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	// 로직
	bool bKey[4];
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver)
	{
		// 시간
		this_thread::sleep_for(50ms);
		bForceDown = (nSpeedCount == nSpeed);

		
		for (int k = 0; k < 4; k++)							
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		
		if (bKey[3])
		{
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
			bRotateHold = true;

		
		if (bForceDown)
		{
			
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;

			
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				
				for (int x = 0; x < 4; x++)
					for (int y = 0; y < 4; y++)
						if (tetromino[nCurrentPiece][Rotate(x, y, nCurrentRotation)] != L'.')
							pField[(nCurrentY + y) * nFieldWidth + (nCurrentX + x)] = nCurrentPiece + 1;

				
				for (int y = 0; y < 4; y++)
					if (nCurrentY + y < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int x = 1; x < nFieldWidth - 1; x++)
							bLine &= (pField[(nCurrentY + y) * nFieldWidth + x]) != 0;

						if (bLine)
						{
							
							for (int x = 1; x < nFieldWidth - 1; x++)
								pField[(nCurrentY + y) * nFieldWidth + x] = 8;
							vLines.push_back(nCurrentY + y);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

		
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				if (tetromino[nCurrentPiece][Rotate(x, y, nCurrentRotation)] != L'.')
					screen[(nCurrentY + y + 2) * nScreenWidth + (nCurrentX + x + 2)] = nCurrentPiece + 65;

		
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		
		if (!vLines.empty())
		{
			
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); 

			for (auto& v : vLines)
				for (int x = 1; x < nFieldWidth - 1; x++)
				{
					for (int y = v; y > 0; y--)
						pField[y * nFieldWidth + x] = pField[(y - 1) * nFieldWidth + x];
					pField[x] = 0;
				}

			vLines.clear();
		}
	
	
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	
	CloseHandle(hConsole);
	cout << "Game Over!! Score:" << nScore << endl;
	system("pause");
	return 0;
}
