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

#include "GameScene.h"


USING_NS_CC;



Scene* GameScene::createScene() {
	return GameScene::create();
}

// on "init" you need to initialize your instance
bool GameScene::init() {

	if (!Scene::init())	return false;

	board = std::make_shared<Board>();

	int xx = 0;
	for (const auto& x : board->BuildBoard()) {
		int yy = 0;
		for (const auto& y : x) {
			this->addChild(y.sprite, 0);
			//
			std::stringstream ss;
			ss << static_cast<char>(xx + 'a') << ", " << yy + 1;
			Label* label = Label::create();
			label->setString(ss.str());
			label->setColor(Color3B::GRAY);
			label->setAnchorPoint(Vec2(0.5, 0.5));
			label->setPosition(y.sprite->getPosition());
			this->addChild(label, 3);
			++yy;
		}
		++xx;
	}

	white_player = std::make_shared<Player>(false, board);
	for (const auto& it : white_player->ArrangeCheckers()) {
		this->addChild(it, 1);
	}

	black_player = std::make_shared<Player>(true, board);
	for (const auto& it : black_player->ArrangeCheckers()) {
		this->addChild(it, 1);
	}


	return true;
}

BoardMap Board::BuildBoard() {
	map.resize(8);
	for (auto& it : map) {
		it.resize(8);
	}
	// рисуем игровое поле
	bool is_green = true;
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			Sprite* cell = Sprite::create("cell.png");
			Vec2 pos(cell->getContentSize().width * x + cell->getContentSize().width / 2,
							 cell->getContentSize().height * y + cell->getContentSize().height / 2);

			cell->setPosition(pos);
			if (is_green) {
				cell->setColor(Color3B(158, 208, 6));
			}
			is_green = !is_green;
			map[x][y].sprite = cell;
		}
		is_green = !is_green;
	}
	return map;
}

std::vector<cocos2d::Sprite*> Player::ArrangeCheckers() {
	int x = 0;
	int y = 0;
	std::string file_name;
	if (is_ai) {
		file_name = "b_checker.png";
		y += 5;
	}
	else {
		file_name = "w_checker.png";
		x += 5;
	}
	for (int yy = y; yy < y + 3; ++yy) {
		for (int xx = x; xx < x + 3; ++xx) {
			cocos2d::Sprite* checker = cocos2d::Sprite::create(file_name);
			const auto cellXY = map->getCell(xx, yy).sprite;
			checker->setPosition(cellXY->getPosition());

			if (is_ai) {
				map->shareCell(xx, yy).status = CellStatus::BLACK;
			}
			else {
				map->shareCell(xx, yy).status = CellStatus::WHITE;
			}
			player_checkers.push_back(checker);
		}
	}
	return player_checkers;
}
