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
		for (const auto& cell : x) {
			// рисуем игровую доску
			this->addChild(cell.cell_sprite, SLayer::BOARD);
			if (cell.pawn_sprite) this->addChild(cell.pawn_sprite, SLayer::PAWN);
			// подписываем клетки (для удобства, потом УДАЛИТЬ)
			this->addChild(cell.label, SLayer::LABEL);
			++y__;
		}
		++x__;
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
	if (board->IsInGame()) {
		if (!board->IsAiMove()) {
			Vec2 touchLocation = touch->getLocation();
			bool is_choise = false;
			int index = 0;

			const Cell& hit_cell = board->GetCellByTouch(touchLocation);
			const Vec2& cell_pos = hit_cell.position_on_map;
			//std::cerr << "hit_cell_pos.x = " << cell_pos.x << "\nhit_cell_pos.y = " << cell_pos.y << '\n';
			// проверка, попали ли пальцем в свою шашку
			if (hit_cell.status == CellStatus::WHITE) { // || hit_cell.status == CellStatus::BLACK) {
				if (board->IsChoised()) board->CancelChoise(cell_pos);
				board->SetChoised(cell_pos);
			}
			else if (hit_cell.status == CellStatus::FREE && board->IsChoised()) {
				board->MoveIsPosibleTo(cell_pos);
			}
		}
	}

	return true;
}

void GameScene::update(float delta) {
	if (board->IsInGame()) {
		if (board->IsAiMove()) {
			board->AiMove();
		}
	}
}


