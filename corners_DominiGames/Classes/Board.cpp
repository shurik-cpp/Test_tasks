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
	for (int y = 0; y < BOARD_SIZE; ++y) {
		for (int x = 0; x < BOARD_SIZE; ++x) {
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

	black_pawns_pos = ArrangeCheckers(CellStatus::BLACK);
	white_pawns_pos = ArrangeCheckers(CellStatus::WHITE);

	// добавим подписи для каждой клетки (для удобства, потом УДАЛИТЬ)
	for (int y = 0; y < BOARD_SIZE; ++y) {
		for (int x = 0; x < BOARD_SIZE; ++x) {
			board[x][y].label = Label::create();
			board[x][y].label->setColor(Color3B::GRAY);
			board[x][y].label->setAnchorPoint(Vec2(0.5, 0.5));
			board[x][y].RefreshLable();
			board[x][y].label->setPosition(board[x][y].cell_sprite->getPosition());
		}
	}
	return board;
}

std::vector<Vec2> Board::ArrangeCheckers(const CellStatus color) {
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
	std::vector<Vec2> pawns;
	for (int y = y__; y < y__ + 3; ++y) {
		for (int x = x__; x < x__ + 3; ++x) {
			auto& cell = board[x][y];
			cell.pawn_sprite = cocos2d::Sprite::create(file_name);
			cell.pawn_sprite->setPosition(cell.cell_sprite->getPosition());
			cell.position_on_map = Vec2(x, y);
			cell.status = color;
			pawns.push_back(Vec2(x, y));
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
	for (int y = 0; y < BOARD_SIZE; ++y) {
		for (int x = 0; x < BOARD_SIZE; ++x) {
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
	size_t random_index;
	Move move;
	// рандомно подбираем пешку, которой можно походить
	do {
		random_index = rand() % black_pawns_pos.size();
		move = IsCanAiMove(black_pawns_pos[random_index]);
	} while(!static_cast<bool>(move));

	// запоминаем текущую позицию пешки
	const Vec2 last_pos = black_pawns_pos[random_index];
	// двигаем позицию в векторе позиций черных пешек
	if (move == Move::RIGHT)	black_pawns_pos[random_index].x++;
	else black_pawns_pos[random_index].y--;

	// запоминаем новую позицию для спрайта, относительно новой клетки
	const Vec2 new_pos = black_pawns_pos[random_index];
	const Vec2 new_sprite_pos =	board[new_pos.x][new_pos.y].cell_sprite->getPosition();
	// перемещаем сам спрайт пешки и указатель на него в новую клетку
	board[new_pos.x][new_pos.y].pawn_sprite = board[last_pos.x][last_pos.y].pawn_sprite;
	board[last_pos.x][last_pos.y].pawn_sprite = nullptr;
	board[new_pos.x][new_pos.y].pawn_sprite->setPosition(new_sprite_pos);
	// не забываем поменять статусы клеток на поле
	board[last_pos.x][last_pos.y].status = CellStatus::FREE;
	board[last_pos.x][last_pos.y].RefreshLable(); //(для удобства, потом УДАЛИТЬ)
	board[new_pos.x][new_pos.y].status = CellStatus::BLACK;
	board[new_pos.x][new_pos.y].RefreshLable(); //(для удобства, потом УДАЛИТЬ)

	this->ChangePlayer();
}

Board::Move Board::IsCanAiMove(const Vec2& random_pawn_pos) const {
	// порверка можно ли шагать вправо или вниз
	int next_x = random_pawn_pos.x + 1;
	int next_y = random_pawn_pos.y - 1;
	bool is_right = true;
	bool is_down = true;
	{ // Первая стадия проверки возможности хода:
		// Следим чтобы не вышли за пределы игрового поля и проверяем клетку на свободность
		if (next_x == BOARD_SIZE || board[next_x][random_pawn_pos.y].status != CellStatus::FREE) is_right = false;
		if (next_y < 0 || board[random_pawn_pos.x][next_y].status != CellStatus::FREE) is_down = false;
		if (!is_right && !is_down)	return Move::NO;
	}
	{ // Вторая стадия проверки возможности хода (посложнее)
		// проверка крайних таргет-линий (левый нижний угол, 2 ряда)
		// если в крайней линии уже собралось 3 черных пешки, значит туда ходить не надо
		// char - идентефикатор оси, pair.first - это линии по краю, pair.second - это вторая от края линия
		std::unordered_map<char, std::pair<int, int>> count;
		const int END_LINE_X = 7;
		const int PRE_END_LINE_X = 6;
		const int END_LINE_Y = 0;
		const int PRE_END_LINE_Y = 1;
		for (int y = 0; y < BOARD_SIZE; ++y) {
			for (int x = 0; x < BOARD_SIZE; ++x) {
				// собираем количество черных пешек в двух крайних линиях по x и по y
				if (board[x][y].status == CellStatus::BLACK) {
					if (x == END_LINE_X) count['y'].first++;
					else if (x == PRE_END_LINE_X) count['y'].second++;
					if (y == END_LINE_Y) count['x'].first++;
					else if (y == PRE_END_LINE_Y) count['x'].second++;
				}
			}
		}
//		std::cerr << "count['x']: END=" << count['x'].first << " PRE=" << count['x'].second << '\n';
//		std::cerr << "count['y']: END=" << count['y'].first << " PRE=" << count['y'].second << '\n';
		// проверка заполненности крайних линий
		const int MAX_PAWNS_IN_LINE = 3;
		if ((next_x == END_LINE_X && count['y'].first == MAX_PAWNS_IN_LINE) ||
				(next_x == PRE_END_LINE_X && count['y'].second == MAX_PAWNS_IN_LINE)) {
			is_right = false;
		}
		if ((next_y == END_LINE_Y && count['x'].first == MAX_PAWNS_IN_LINE) ||
				(next_y == PRE_END_LINE_Y && count['x'].second == MAX_PAWNS_IN_LINE)) {
			is_down = false;
		}
		if (!is_right && !is_down) return Move::NO;
	}
	// если можно ходить и влево и вниз, тогда возвращаем рандомный Move, исключая Move::NO
	if (is_right && is_down) return static_cast<Move>(rand() % 3 + 1);
	else if (is_right) return Move::RIGHT;
	return Move::DOWN;
}

