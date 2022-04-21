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
			// добавим подписи для каждой клетки
			std::stringstream ss;
			ss << static_cast<char>(x__ + 'a') << y__ + 1;
			Label* label = Label::create();
			label->setString(ss.str());
			label->setColor(Color3B::GRAY);
			label->setAnchorPoint(Vec2(0.5, 0.5));
			label->setPosition(cell.cell_sprite->getPosition());
			this->addChild(label, SLayer::LABEL);
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

	Vec2 touchLocation = touch->getLocation();
	bool is_choise = false;
	int index = 0;

	const Cell& hit_cell = board->GetCellByTouch(touchLocation);
	const Vec2& cell_pos = hit_cell.position_on_map;
	// проверка, попали ли пальцем в свою шашку
	if (hit_cell.status == CellStatus::WHITE) {
		if (board->IsChoised()) board->CancelChoise(cell_pos);
		board->SetChoised(cell_pos);
	}
	else if (hit_cell.status == CellStatus::FREE && board->IsChoised()) {
		board->MoveIsPosibleTo(cell_pos);
	}

	return true;
}

void GameScene::update(float delta) {
	if (board->IsAiMove()) {
		board->AiMove();
		board->ChangePlayer();
	}
}


// BoardMap - это std::vector<std::vector<Cell>>
BoardMap Board::BuildBoard() {
	board.resize(8);
	for (auto& it : board) {
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
			board[x][y].cell_sprite = cell;
		}
		is_green = !is_green;
	}

	black_pawns = ArrangeCheckers(CellStatus::BLACK);
	white_pawns = ArrangeCheckers(CellStatus::WHITE);
	return board;
}

std::vector<Cell> Board::ArrangeCheckers(const CellStatus color) {
	int x__ = 0;
	int y__ = 0;
	std::string file_name;
	if (color == CellStatus::BLACK) {
		file_name = "black_checker.png";
		y__ += 5;
	}
	else {
		file_name = "white_checker.png";
		x__ += 5;
	}
	std::vector<Cell> pawns;
	for (int y = y__; y < y__ + 3; ++y) {
		for (int x = x__; x < x__ + 3; ++x) {
			auto& cell = board[x][y];
			cell.pawn_sprite = cocos2d::Sprite::create(file_name);
			cell.pawn_sprite->setPosition(cell.cell_sprite->getPosition());
			cell.position_on_map = Vec2(x, y);
			cell.status = color;
			pawns.push_back(cell);
		}
	}
	return std::move(pawns);
}

const Cell& Board::GetCellByTouch(const Vec2& touchLocation) const {
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			auto diff = touchLocation - board[x][y].cell_sprite->getPosition();

			if ((abs(diff.x) <= board[x][y].cell_sprite->getContentSize().height / 2)
					&& (abs(diff.y) <= board[x][y].cell_sprite->getContentSize().width / 2)) {
				return board[x][y];
			}
		}
	}
	return std::move(Cell());
}

void Board::MoveIsPosibleTo(const Vec2& move_to) {
	Vec2 pawn = choised_pawn->position_on_map;
	Cell& target_cell = board[move_to.x][move_to.y];

	std::cerr << "MoveIsPosibleTo..\n";

	if (target_cell.status == CellStatus::FREE) {
		if (((target_cell.position_on_map.x == pawn.x + 1 || target_cell.position_on_map.x == pawn.x - 1) && target_cell.position_on_map.y == pawn.y) ||
			((target_cell.position_on_map.y == pawn.y + 1 || target_cell.position_on_map.y == pawn.y - 1) && target_cell.position_on_map.x == pawn.x)) {

			std::cerr << "Move to x = " << move_to.x << ", y = " << move_to.y << '\n';

			target_cell.pawn_sprite = choised_pawn->pawn_sprite;
			target_cell.pawn_sprite->setPosition(target_cell.cell_sprite->getPosition());
			target_cell.status = choised_pawn->status;
			choised_pawn->pawn_sprite = nullptr;
			choised_pawn->choised = false;
			choised_pawn->status = CellStatus::FREE;
			choised_pawn = nullptr;
		}
		else {
			std::cerr << "No posible :(\n";
		}
	}
}
