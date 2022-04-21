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
	int x__ = 0;
	for (const auto& x : board->BuildBoard()) {
		int y__ = 0;
		for (const auto& y : x) {
			// рисуем игровую доску
			this->addChild(y.sprite, SLayer::BOARD);
			// добавим подписи для каждой клетки
			std::stringstream ss;
			ss << static_cast<char>(x__ + 'a') << y__ + 1;
			Label* label = Label::create();
			label->setString(ss.str());
			label->setColor(Color3B::GRAY);
			label->setAnchorPoint(Vec2(0.5, 0.5));
			label->setPosition(y.sprite->getPosition());
			this->addChild(label, SLayer::LABEL);
			++y__;
		}
		++x__;
	}

	white_player = std::make_shared<Player>(false, board);
	for (const auto& it : white_player->ArrangeCheckers()) {
		this->addChild(it, SLayer::CHECKER);
	}

	black_player = std::make_shared<Player>(true, board);
	for (const auto& it : black_player->ArrangeCheckers()) {
		this->addChild(it, SLayer::CHECKER);
	}

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	// Запускаем игровой цикл
	// Как только игровой цикл открывается, он вызывает функцию GameScene::update(float delta)
	this->scheduleUpdate();

	return true;
}

bool GameScene::onTouchBegan(Touch* touch, Event* unused_event) {

	Vec2 touchLocation = touch->getLocation();

	for (auto sprite : white_player->ShareCheckers()) {
		auto diff = touchLocation - sprite->getPosition();

		if (abs(diff.x) <= sprite->getContentSize().height / 2 &&
				abs(diff.y) <= sprite->getContentSize().width / 2) {
			white_player->ResetChoise();
			white_player->SetChoised(sprite);
		}
	}


	return true;
}

void GameScene::update(float delta) {
	if (board->IsAiMove()) {
		black_player->Tick();
		board->ChangePlayer();
	}
	else {
		white_player->Tick();
//		if (white_player->Tick())) {
//			board->ChangePlayer();
//		}
	}
}



void Player::Tick() {

}
bool Player::Tick(const Event& event) {

	return false;
}

// BoardMap - это std::vector<std::vector<Cell>>
BoardMap Board::BuildBoard() {
	map.resize(8);
	for (auto& it : map) {
		it.resize(8);
	}
	// создаем игровое поле 8х8
	bool is_green = true;
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			Sprite* cell = Sprite::create("cell.png");
			const Vec2 pos(cell->getContentSize().width * x + cell->getContentSize().width / 2,
							 cell->getContentSize().height * y + cell->getContentSize().height / 2);

			cell->setPosition(pos);
			if (is_green) { // меняем цвет клетки на зеленый
				cell->setColor(Color3B(158, 208, 6));
			}
			is_green = !is_green;
			// у каждой клетки свой индекс в двумерном векторе
			// по индексу нужно будет достать cell.sprite->getPosition() чтобы знать куда перемещать шашку,
			// и вообще иметь полную картину хода игры
			map[x][y].sprite = cell;
		}
		is_green = !is_green;
	}
	return map;
}

std::vector<cocos2d::Sprite*> Player::ArrangeCheckers() {
	int x__ = 0;
	int y__ = 0;
	std::string file_name;
	if (is_ai) {
		file_name = "black_checker.png";
		y__ += 5;
	}
	else {
		file_name = "white_checker.png";
		x__ += 5;
	}
	for (int y = y__; y < y__ + 3; ++y) {
		for (int x = x__; x < x__ + 3; ++x) {
			cocos2d::Sprite* checker = cocos2d::Sprite::create(file_name);
			const auto cellXY = map->GetCell(x, y).sprite;
			checker->setPosition(cellXY->getPosition());

			// отмечаем какие где расположены шашки на игровом поле
			if (is_ai) {
				map->ShareCell(x, y).status = CellStatus::BLACK;
			}
			else {
				map->ShareCell(x, y).status = CellStatus::WHITE;
			}
			player_checkers.push_back(checker);
		}
	}
	return player_checkers;
}
