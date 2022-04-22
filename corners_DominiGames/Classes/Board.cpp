#include "Board.h"

USING_NS_CC; // using namespace cocos2d

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
			board[x][y].position_on_map = Vec2(x, y);
		}
		is_green = !is_green;
	}

	black_pawns = ArrangeCheckers(CellStatus::BLACK);
	white_pawns = ArrangeCheckers(CellStatus::WHITE);

	// добавим подписи для каждой клетки (для удобства, потом УДАЛИТЬ)
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			board[x][y].label = Label::create();
			board[x][y].label->setColor(Color3B::GRAY);
			board[x][y].label->setAnchorPoint(Vec2(0.5, 0.5));
			board[x][y].RefreshLable();
			board[x][y].label->setPosition(board[x][y].cell_sprite->getPosition());
		}
	}
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
			cell.status = color;
			pawns.push_back(cell);
		}
	}
	return std::move(pawns);
}

void Board::SetChoised(const cocos2d::Vec2& coordinates) {
	choised_pawn = &board[coordinates.x][coordinates.y];
	choised_pawn->choised = true;
	choised_pawn->pawn_sprite->setColor(cocos2d::Color3B(cocos2d::Color3B::RED));
}

void Board::CancelChoise(const cocos2d::Vec2& coordinates) {
	choised_pawn->choised = false;
	choised_pawn->pawn_sprite->setColor(cocos2d::Color3B(cocos2d::Color3B::WHITE));
	choised_pawn = nullptr;
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
	// warning - не наше..
	throw std::out_of_range("Touch in out of posible\n");
}

void Board::MoveIsPosibleTo(const Vec2& move_to) {
	Vec2 pawn = choised_pawn->position_on_map;
	Cell& target_cell = board[move_to.x][move_to.y];

	if (target_cell.status == CellStatus::FREE) {
		if (((target_cell.position_on_map.x == pawn.x + 1 || target_cell.position_on_map.x == pawn.x - 1) && target_cell.position_on_map.y == pawn.y) ||
			((target_cell.position_on_map.y == pawn.y + 1 || target_cell.position_on_map.y == pawn.y - 1) && target_cell.position_on_map.x == pawn.x)) {

			std::cerr << "Move to x = " << move_to.x << ", y = " << move_to.y << '\n';

			target_cell.pawn_sprite = choised_pawn->pawn_sprite;
			target_cell.pawn_sprite->setPosition(target_cell.cell_sprite->getPosition());
			target_cell.status = choised_pawn->status;
			target_cell.RefreshLable(); // обновим подпись клетки (для удобства, потом УДАЛИТЬ)
			choised_pawn->pawn_sprite->setColor(Color3B(Color3B::WHITE));
			choised_pawn->status = CellStatus::FREE;
			choised_pawn->RefreshLable(); // обновим подпись клетки (для удобства, потом УДАЛИТЬ)
			choised_pawn->pawn_sprite = nullptr;
			choised_pawn->choised = false;
			choised_pawn = nullptr;

			this->ChangePlayer();
		}
		else {
			std::cerr << "No posible :(\n";
		}
	}
}

void Board::AiMove() {

	this->ChangePlayer();
}
