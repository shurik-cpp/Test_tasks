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
			// по индексу нужно будет достать cell.sprite->getPosition() чтобы знать куда перемещать пешку,
			// и вообще иметь полную картину хода игры
			board[x][y].cell_sprite = cell;
			board[x][y].position_on_map = Vec2(x, y);
		}
		is_green = !is_green;
	}

	black_pawns = ArrangeCheckers(CellStatus::BLACK);
	white_pawns = ArrangeCheckers(CellStatus::WHITE);

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

std::vector<Board::Pawn> Board::ArrangeCheckers(const CellStatus color) {
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
	std::vector<Pawn> pawns;
	for (int y = y__; y < y__ + 3; ++y) {
		for (int x = x__; x < x__ + 3; ++x) {
			auto& cell = board[x][y];
			cell.pawn_sprite = cocos2d::Sprite::create(file_name);
			cell.pawn_sprite->setPosition(cell.cell_sprite->getPosition());
			cell.position_on_map = Vec2(x, y);
			cell.status = color;
			pawns.push_back(Pawn(Vec2(x, y)));
		}
	}
	return std::move(pawns);
}

void Board::SetChoised(const cocos2d::Vec2& pos) {
	choised_pawn = &board[pos.x][pos.y];
	choised_pawn->choised = true;
	choised_pawn->pawn_sprite->setColor(cocos2d::Color3B(cocos2d::Color3B::RED));
}

void Board::CancelChoise(const cocos2d::Vec2& pos) {
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
	throw std::out_of_range("Touch is not possible\n");
}

void Board::MoveIsPosibleTo(const Vec2& move_to) {
	Vec2 pawn = choised_pawn->position_on_map;
	Cell& target_cell = board[move_to.x][move_to.y];

	if (target_cell.status == CellStatus::FREE) {
		if (((target_cell.position_on_map.x == pawn.x + 1 || target_cell.position_on_map.x == pawn.x - 1) && target_cell.position_on_map.y == pawn.y) ||
				((target_cell.position_on_map.y == pawn.y + 1 || target_cell.position_on_map.y == pawn.y - 1) && target_cell.position_on_map.x == pawn.x)) {

			//std::cerr << "Move to x = " << move_to.x << ", y = " << move_to.y << '\n';

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
			std::cerr << "No possible :(\n";
		}
	}
}

int Board::GetRandomNumber(int min, int max) {
	// Установим начальную точку генерирования последовательности относительно time(NULL)
	srand(time(NULL));
	return min + rand() % (max - min + 1);
}

void Board::AiMove() {

	ResetAllPawnsFlags(black_pawns);
	SetPawnsFlags(black_pawns);

	std::vector<size_t> non_blocked = GetNonBlockedPawnsForAdvance(black_pawns);
	Move move;

	bool is_advance;
	if (non_blocked.size() > 0) {
		is_advance = true;
		std::cerr << "Advanced non_blocked.size() = " << non_blocked.size() << '\n';
	}
	else {
		is_advance = false;
		non_blocked = GetNonBlockedPawnsForBypass(black_pawns);
		std::cerr << "Bypassed non_blocked.size() = " << non_blocked.size() << '\n';
	}
	if (non_blocked.size() == 0) {
		is_game_over = true;
		return;
	}

	const size_t random_index = GetRandomNumber(0, non_blocked.size() -1);
	std::cerr << "random_index = " << random_index << std::endl;
	const size_t index = non_blocked[random_index];
	std::cerr << "Non_blocked indexes:\n";
	for (const auto it : non_blocked) {
		std::cerr << it << '\t';
	}
	std::cerr << std::endl << "Choised index = " << index << std::endl;

	move = GetMoveDirection(black_pawns[index], is_advance);
	// если обходим препятствие, запоминаем плохую позицию
	if (!is_advance) SetBadPosition(black_pawns[index]);

	// запоминаем текущую позицию пешки
	const Vec2 last_pos = black_pawns[index].pos;

	std::cerr << "Pawn move X = " << black_pawns[index].pos.x
						<< " Y = " << black_pawns[index].pos.y << " --> ";
	// двигаем позицию в векторе позиций черных пешек
	if (move == Move::RIGHT) black_pawns[index].pos.x++;
	else if (move == Move::DOWN) black_pawns[index].pos.y--;
	else if (move == Move::LEFT) black_pawns[index].pos.x--;
	else if (move == Move::UP) black_pawns[index].pos.y++;
	std::cerr << "X = " << black_pawns[index].pos.x
						<< " Y = " << black_pawns[index].pos.y  << "\n===========================" << std::endl;

	// запоминаем новую позицию для спрайта, относительно новой клетки
	const Vec2 new_pos = black_pawns[index].pos;
	const Vec2 new_sprite_pos =	board[new_pos.x][new_pos.y].cell_sprite->getPosition();
	// перемещаем спрайт пешки и указатель на него в новую клетку на доске
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

void Board::ResetAllPawnsFlags(std::vector<Pawn>& pawns) {
	for (auto& pawn : pawns) {
		pawn.is_move_down = true;
		pawn.is_move_left = true;
		pawn.is_move_right = true;
		pawn.is_move_up = true;
	}
}

void Board::SetPawnsFlags(std::vector<Pawn>& pawns) {
	for (auto& pawn : pawns) {
		// Первая стадия проверки:
		// можно ли шагать вправо/вниз
		const int forward_x = pawn.pos.x + 1;
		const int forward_y = pawn.pos.y - 1;
		// Следим чтобы не вышли за пределы игрового поля и смотрим, чтобы не растоптать другие пешки
		if (forward_x != BOARD_SIZE) {
			if (board[forward_x][pawn.pos.y].status != CellStatus::FREE) pawn.is_move_right = false;
		}
		else pawn.is_move_right = false;
		if (forward_y >= 0) {
			if (board[pawn.pos.x][forward_y].status != CellStatus::FREE) pawn.is_move_down = false;
		}
		else pawn.is_move_down = false;
		// Вторая стадия проверки возможности хода (посложнее)
		// проверка крайних линий (левый нижний угол, 2 ряда)
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
		// проверка заполненности крайних линий
		const int MAX_PAWNS_IN_LINE = 3;
		if ((forward_x == END_LINE_X && count['y'].first == MAX_PAWNS_IN_LINE) ||
				(forward_x == PRE_END_LINE_X && count['y'].second == MAX_PAWNS_IN_LINE)) {
			pawn.is_move_right = false;
		}
		if ((forward_y == END_LINE_Y && count['x'].first == MAX_PAWNS_IN_LINE) ||
				(forward_y == PRE_END_LINE_Y && count['x'].second == MAX_PAWNS_IN_LINE)) {
			pawn.is_move_down = false;
		}
		// Третья стадия:
		// проверка на плохую позицию
		if (forward_x == pawn.last_bad_pos.x) pawn.is_move_right = false;
		if (forward_y == pawn.last_bad_pos.y) pawn.is_move_down = false;
		// запоминаем плохую позицию
		if (!pawn.is_move_right && !pawn.is_move_down) SetBadPosition(pawn);
		// Четвертая стадия:
		// можем ли ходить вверх/влево
		const int back_x = pawn.pos.x - 1;
		const int back_y = pawn.pos.y + 1;
		// Следим чтобы не вышли за пределы игрового поля и смотрим, чтобы не растоптать другие пешки
		if (back_x >= 0) {
			if (board[back_x][pawn.pos.y].status != CellStatus::FREE) pawn.is_move_left = false;
		}
		else pawn.is_move_left = false;
		if (back_y != BOARD_SIZE) {
			if (board[pawn.pos.x][back_y].status != CellStatus::FREE) pawn.is_move_up = false;
		}
		else pawn.is_move_up = false;
	}
}

std::vector<size_t> Board::GetNonBlockedPawnsForAdvance(const std::vector<Board::Pawn>& pawns) const {
	std::vector<size_t> result;
	for (size_t index = 0; index < pawns.size(); ++index) {
		if (pawns[index].is_move_right || pawns[index].is_move_down) result.push_back(index);
	}
	return std::move(result);
}

std::vector<size_t> Board::GetNonBlockedPawnsForBypass(const std::vector<Board::Pawn>& pawns) const {
	std::vector<size_t> result;
	for (size_t index = 0; index < pawns.size(); ++index) {
		if (pawns[index].is_move_left || pawns[index].is_move_up) result.push_back(index);
	}
	return std::move(result);
}

void Board::SetBadPosition(Pawn& pawn) {
	pawn.last_bad_pos = pawn.pos;
}

Board::Move Board::GetMoveDirection(const Pawn& pawn, const bool is_advance) const {
	Move move = BLOCKED;
	if (is_advance) {
		// если можно ходить и вправо и вниз, тогда возвращаем рандомный Move, исключая Move::NO, LEFT, UP
		if (pawn.is_move_right && pawn.is_move_down) {
			if (rand() % 2) move = Move::RIGHT;
			else move = Move::DOWN;
		}
		else if (pawn.is_move_right) move = Move::RIGHT;
		else move = Move::DOWN;
	}
	else {
		if (!pawn.is_move_down && pawn.is_move_left) move = Move::LEFT;
		else if (!pawn.is_move_right && pawn.is_move_up) move = Move::UP;
	}
	return move;
}
