#include "Chess.h"
#include <math.h> 
#include<mmsyscom.h>
#pragma comment(lib, "winmm.lib")
#include <conio.h>

// a function for removing the non-transparent backgroud of chess piece pictures. EasyX itself des not support the transparent background.
void putimagePNG(int x, int y, IMAGE* picture) //x为载入图片的X坐标，y为Y坐标
{
	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + x) + (iy + y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}

Chess::Chess(int gradeSize, int marginX, int marginY, float chessSize) :
	gradeSize(gradeSize), margin_x(marginX), margin_y(marginY), chessSize(chessSize)
{
	this->playerFlag = CHESS_BLACK;
	for (int i = 0; i < gradeSize; i++) {
		vector<int> row;
		for (int j = 0; j < gradeSize; j++) {
			row.push_back(0);
		}
		chessMap.push_back(row);
	}
}

void Chess::init()
{
	// create game window by easyX function
	initgraph(897, 895, EW_SHOWCONSOLE);  // the arguments depend on the size of the board picture

	// show the board picture
	loadimage(0, "res/board2.jpg");
	// play the starting music
	mciSendString("play res/start.mp3", 0, 0, 0);
	
	// load piece pictures
	loadimage(&chessBlackImg, "res/black.png", chessSize, chessSize, true);
	loadimage(&chessWhiteImg, "res/white.png", chessSize, chessSize, true);

	// clear the chessboard
	for (int i = 0; i < chessMap.size(); i++) {
		for (int j = 0; j < chessMap.size(); j++) {
			chessMap[i][j] = 0;
		}
	}

	playerFlag = true;  // black goes the first
}

bool Chess::clickBoard(int x, int y, ChessPos* pos) {
	int col = (x - margin_x) / chessSize;  
	int row = (y - margin_y) / chessSize;
	int offset = chessSize * 0.4;  // the error tolerance 
	int len;  // the distance between the selected position and the chessboard effective points.

	// left top
	int leftTopPosX = margin_x + chessSize * col;
	int leftTopPosY = margin_y + chessSize * row;
	len = sqrt((x - leftTopPosX) * (x - leftTopPosX) +
		(y - leftTopPosY) * (y - leftTopPosY));
	if (len < offset) {
		pos->row = row;
		pos->col = col;

		if (!chessMap[pos->row][pos->col]) {
			return true;  // if no piece in the selected position
		}
	}

	// right top	
	int rightTopPosX = leftTopPosX + chessSize;
	int rightTopPosY = leftTopPosY;
	len = sqrt((x - rightTopPosX) * (x - rightTopPosX) +
		(y - rightTopPosY) * (y - rightTopPosY));
	if (len < offset) {
		pos->row = row;
		pos->col = col + 1;

		if (!chessMap[pos->row][pos->col]) {
			return true;  // if no piece in the selected position
		}
	}

	// left bottom
	int leftBottomPosX = leftTopPosX;
	int leftBottomPosY = leftTopPosY + chessSize;
	len = sqrt((x - leftBottomPosX) * (x - leftBottomPosX) +
		(y - leftBottomPosY) * (y - leftBottomPosY));
	if (len < offset) {
		pos->row = row + 1;
		pos->col = col;

		if (!chessMap[pos->row][pos->col]) {
			return true;  // if no piece in the selected position
		}
	}

	// right bottom
	int rightBottomPosX = leftTopPosX + chessSize;
	int rightBottomPosY = leftTopPosY + chessSize;
	len = sqrt((x - rightBottomPosX) * (x - rightBottomPosX) +
		(y - rightBottomPosY) * (y - rightBottomPosY));
	if (len < offset) {
		pos->row = row + 1;
		pos->col = col + 1;

		if (!chessMap[pos->row][pos->col]) {
			return true;  // if no piece in the selected position
		}
	}
	return false;
}

void Chess::chessDown(ChessPos* pos, chess_kind_t kind)
{
	mciSendString("play res/down7.WAV", 0, 0, 0);
	int x = margin_x + chessSize * (pos->col - 0.5);
	int y = margin_y + chessSize * (pos->row - 0.5);

	if (kind == CHESS_WHITE) {
		putimagePNG(x, y, &chessWhiteImg);
	}
	else {
		putimagePNG(x, y, &chessBlackImg);
	}

	updateGameMap(pos);
}

int Chess::getGradeSize()
{
	return gradeSize;
}

int Chess::getChessData(int row, int col)
{
	int res = chessMap[row][col];
	return res;
}

int Chess::getChessData(ChessPos* pos)
{
	int res = chessMap[pos->row][pos->col];
	return res;
}

bool Chess::checkOver()
{
	// check if any side wins
	if (checkWin()) {
		Sleep(1500);
		if (!playerFlag) {
			// true: black's turn, false: white's turn
			// so if playerFlag is false when game ends, we know black(Player) wins.
			mciSendString("play res/PlayerWins.mp3", 0, 0, 0);
			loadimage(0, "res/win.jpg");
		}
		else {
			mciSendString("play res/Playerloses.mp3", 0, 0, 0);
			loadimage(0, "res/lose.jpg");
		}
		_getch();  // pause on the window.
		return true;
	}

	return false;
}

int Chess::getMarginX()
{
	return margin_x;
}

int Chess::getMarginY()
{
	return margin_y;
}

int Chess::getChessSize()
{
	return chessSize;
}

void Chess::updateGameMap(ChessPos* pos)
{
	lastPos = *pos;
	chessMap[pos->row][pos->col] =
		playerFlag ? CHESS_BLACK : CHESS_WHITE;
	playerFlag = !playerFlag;  // change the side
}

bool Chess::checkWin()
{
	int row = lastPos.row;
	int col = lastPos.col;
	int starting = chessMap[row][col];

	// check horizontal direction
	for (int i = 0; i < 5; i++) {
		if (col - i < 0) break;
		if (col - i + 4 >= gradeSize) break;
		
		int cur = chessMap[row][col - i]; 
		if (cur != starting) break;

		int seqCount = 1;  // initial value 1, bacause current position already have one piece
		for (int j = 1; j <= 4; j++) {
			if (seqCount < 5 && starting == chessMap[row][col - i + j]) {
				seqCount++;
			}
			else if (seqCount < 5 && starting != chessMap[row][col - i + j]) {
				break;
			}
		}
		if (seqCount == 5) return true;
	}

	// check vertical direction
	for (int i = 0; i < 5; i++) {
		if (row - i < 0) break;
		if (row - i + 4 >= gradeSize) break;

		int cur = chessMap[row - i][col];
		if (cur != starting) break;

		int seqCount = 1;  // initial value 1, bacause current position already have one piece
		for (int j = 1; j <= 4; j++) {
			if (seqCount < 5 && starting == chessMap[row - i + j][col]) {
				seqCount++;
			}
			else if (seqCount < 5 && starting != chessMap[row - i + j][col]) {
				break;
			}
		}
		if (seqCount == 5) return true;

		
	}

	// check left diagnal direction "\"
	for (int i = 0; i < 5; i++) {
		if (row - i < 0 || col - i < 0) break;
		if (row - i + 4 >= gradeSize || col - i + 4 >= gradeSize) break;

		int cur = chessMap[row - i][col - i];
		if (cur != starting) break;

		int seqCount = 1;  // initial value 1, bacause current position already have one piece
		for (int j = 1; j <= 4; j++) {
			if (seqCount < 5 && starting == chessMap[row - i + j][col - i + j]) {
				seqCount++;
			}
			else if (seqCount < 5 && starting != chessMap[row - i + j][col - i + j]) {
				break;
			}
		}
		if (seqCount == 5) return true;
	}

	// check right diagnal direction "/"
	for (int i = 0; i < 5; i++) {
		if (row - i < 0 || col + i >= gradeSize) break;
		if (row - i + 4 >= gradeSize || col + i - 4 < 0) break;

		int cur = chessMap[row - i][col + i];
		if (cur != starting) break;

		int seqCount = 1;  // initial value 1, bacause current position already have one piece
		for (int j = 1; j <= 4; j++) {
			if (col + i - j < 0) break;
			if (seqCount < 5 && starting == chessMap[row - i + j][col + i - j]) {
				seqCount++;
			}
			else if (seqCount < 5 && starting != chessMap[row - i + j][col + i - j]) {
				break;
			}
		}
		if (seqCount == 5) return true;
	}

	
	return false;
}
