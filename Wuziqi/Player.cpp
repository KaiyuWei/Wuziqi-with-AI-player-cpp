#include "Player.h"
#include "Chess.h"
#include<iostream>

using std::cout;
using std::endl;

void Player::init(Chess* chess){
	this->chess = chess;
}

void Player::go()
{
	MOUSEMSG msg;  // easyX lib, for getting mouse message
	ChessPos pos;
	int mx = chess->getMarginX();
	int my = chess->getMarginY();
	int ps = chess->getChessSize();
	
	while (1) {
		msg = GetMouseMsg();
		if (msg.uMsg == WM_LBUTTONDOWN
			&& chess->clickBoard(msg.x, msg.y, &pos)) {
			int row = (msg.x - mx) / ps;
			int col = (msg.y - my) / ps;

			break;
		}
	}
	
	printf("%d, %d\n", pos.row, pos.col);
	// put chess piece
	chess->chessDown(&pos, CHESS_BLACK);
}
