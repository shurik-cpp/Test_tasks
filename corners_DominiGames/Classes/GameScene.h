/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include <iostream>
#include <memory>
#include <vector>


enum CellStatus { BLACK = -1, FREE, WHITE };

struct Cell {
	cocos2d::Sprite* cell_sprite = nullptr; // спрайт клетки
	cocos2d::Sprite* pawn_sprite = nullptr; // спрайт клетки
	CellStatus status = FREE;
	cocos2d::Vec2 position_on_map;
	bool choised = false;
};

using BoardMap = std::vector<std::vector<Cell>>;

class Board {
public:
	explicit Board() {}

	BoardMap BuildBoard();
	std::vector<Cell> ArrangeCheckers(const CellStatus color = WHITE);

	const Cell& GetCell(const int x, const int y) const { return board[x][y]; }

	bool IsChoised() const { return choised_pawn; }
	void SetChoised(const cocos2d::Vec2& coordinates) {
		choised_pawn = &board[coordinates.x][coordinates.y];
		choised_pawn->choised = true;
		choised_pawn->pawn_sprite->setColor(cocos2d::Color3B(cocos2d::Color3B::RED));
	}
	void CancelChoise(const cocos2d::Vec2& coordinates) {
		choised_pawn->choised = false;
		choised_pawn->pawn_sprite->setColor(cocos2d::Color3B(cocos2d::Color3B::WHITE));
		choised_pawn = nullptr;
	}
	const Cell* GetChoised() const { return choised_pawn; }

	const Cell& GetCellByTouch(const cocos2d::Vec2& touchLocation) const;

	cocos2d::Vec2 GetCellSize() const {
		return {board[0][0].cell_sprite->getContentSize().width, board[0][0].cell_sprite->getContentSize().height};
	}
	void MoveIsPosibleTo(const cocos2d::Vec2& move_to);

	// true - ходит компьютер
	// false - ходит человек
	bool IsAiMove() const { return ai_move; }
	void ChangePlayer() { ai_move = !ai_move; }

	void AiMove() {}

private:
	BoardMap board; // board[x][y] - x и y это позиция одной клетки
	std::vector<Cell> black_pawns;
	std::vector<Cell> white_pawns;
	bool ai_move = false; // хранит чей сейчас ход
	Cell* choised_pawn = nullptr;

};


class GameScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();
	// инициализация игровой сцены
	virtual bool init() override;
	// игровой цикл
	virtual void update(float delta) override;
	// обработка событий касания
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);

	// implement the "static create()" method manually
	CREATE_FUNC(GameScene);

private:
	enum SLayer {BOARD, PAWN, LABEL};

	std::shared_ptr<Board> board;

//	std::shared_ptr<Player> white_player;
//	std::shared_ptr<Player> black_player;

};



#endif // __GAME_SCENE_H__
