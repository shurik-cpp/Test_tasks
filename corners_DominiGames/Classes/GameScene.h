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

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>


enum CellStatus { BLACK = -1, FREE, WHITE };

struct Cell {
	Cell() {}
	Cell(cocos2d::Sprite* sprt, CellStatus stat) : sprite(sprt), status(stat) { }

	cocos2d::Sprite* sprite;
	CellStatus status = FREE;
};

using BoardMap = std::vector<std::vector<Cell>>;

class Board {
public:
	explicit Board() {}

	BoardMap BuildBoard();

	Cell getCell(const int x, const int y) const { return map[x][y];	}
	Cell& shareCell(const int x, const int y) { return map[x][y];	}

	cocos2d::Vec2 getCellSize() const {
		return {map[0][0].sprite->getContentSize().width, map[0][0].sprite->getContentSize().height};
	}

private:

	BoardMap map; // map[x][y]

};

class Player {
public:
	explicit Player(const bool ai, std::shared_ptr<Board> bd)
		: is_ai(ai)
		, map(bd) {}

	std::vector<cocos2d::Sprite*> ArrangeCheckers();

	~Player() { }

private:
	const int NUMBER_OF_CHECKERS = 9;
	bool is_ai;
	std::vector<cocos2d::Sprite*> player_checkers;
	std::shared_ptr<Board> map;



};


class GameScene : public cocos2d::Scene {
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(GameScene);

private:
	std::shared_ptr<Board> board;

	std::unordered_map<std::string, Cell> board_map;

	std::shared_ptr<Player> white_player;
	std::shared_ptr<Player> black_player;



};



#endif // __HELLOWORLD_SCENE_H__
