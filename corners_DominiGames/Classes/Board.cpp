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

			//std::cerr << "Human: Move to x = " << move_to.x << ", y = " << move_to.y << '\n';

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
		std::cerr << "Game over!" << std::endl;
		return;
	}

	size_t random_index;
	if (black_pawns[bypassed_index].is_bypass) random_index = bypassed_index;
	else random_index = GetRandomNumber(0, non_blocked.size() -1);
	std::cerr << "random_index = " << random_index << std::endl;
	if (!is_advance) bypassed_index = random_index;

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

std::unordered_map<char, std::pair<int, int>> Board::CheckRedZones(const CellStatus color) const {
	std::unordered_map<char, std::pair<int, int>> count;
	const int END_LINE_X = 7;
	const int PRE_END_LINE_X = 6;
	const int END_LINE_Y = 0;
	const int PRE_END_LINE_Y = 1;
	for (int y = 0; y < BOARD_SIZE; ++y) {
		for (int x = 0; x < BOARD_SIZE; ++x) {
			// собираем количество черных пешек в двух крайних линиях по x и по y
			if (board[x][y].status == color) {
				if (x == END_LINE_X) count['y'].first++;
				else if (x == PRE_END_LINE_X) count['y'].second++;
				if (y == END_LINE_Y) count['x'].first++;
				else if (y == PRE_END_LINE_Y) count['x'].second++;
			}
		}
	}
	return std::move(count);
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
		std::unordered_map<char, std::pair<int, int>> count_black = CheckRedZones(CellStatus::BLACK);
		std::unordered_map<char, std::pair<int, int>> count_white = CheckRedZones(CellStatus::WHITE);
		const int END_LINE_X = 7;
		const int PRE_END_LINE_X = 6;
		const int END_LINE_Y = 0;
		const int PRE_END_LINE_Y = 1;

		// проверка заполненности крайних линий
		const int MAX_PAWNS_IN_LINE = 3;
		if ((forward_x == END_LINE_X && count_black['y'].first == MAX_PAWNS_IN_LINE && count_white['x'].first == 0) ||
				(forward_x == PRE_END_LINE_X && count_black['y'].second == MAX_PAWNS_IN_LINE && count_white['x'].second == 0)) {
			pawn.is_move_right = false;
		}
		if ((forward_y == END_LINE_Y && count_black['x'].first == MAX_PAWNS_IN_LINE && count_white['y'].first == 0) ||
				(forward_y == PRE_END_LINE_Y && count_black['x'].second == MAX_PAWNS_IN_LINE && count_white['y'].second == 0)) {
			pawn.is_move_down = false;
		}
		// Третья стадия:
		// проверка на плохую позицию
		// запоминаем плохую позицию
		if (!pawn.is_move_right && !pawn.is_move_down) {
			pawn.is_bypass = true;
			SetBadPosition(pawn);
		}
		if (pawn.is_bypass) {
			if (forward_x == pawn.last_bad_pos.x) pawn.is_move_right = false;
			if (forward_y == pawn.last_bad_pos.y) pawn.is_move_down = false;
			if (pawn.is_move_right || pawn.is_move_down) {
				pawn.is_bypass = false;
				ResetBadPosition(pawn);
			}
		}
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
	std::vector<std::pair<Cell, Move>> tmp;
	using std::cerr;
	using std::endl;
	cerr << "========================================" << endl;
	// суть алгоритма выбора годной пешки для обхода белой пешки такова:
	// 1. запускаем главный цикл поиска
	// 2. берем все черные пешки, отсеивая тех, кто уже "дома" или уже не годных для обхода
	// 3. ищем две самые ближайшие к "дому" пешки по принципу max(x) или min(y)
	//    при помощи сортировки сперва по y(!), затем по x
	// 4. проверяем, чтобы не выйти за границы поля и с какой стороны препятствие
	// 5. если ни одна из этих пешек не может эффективно походить, возвращаемся к п.1
	// 6. находим по координатам в векторе черных пешек и вычисляем их индекс
	while (result.size() < 1) { // крутим главный цикл пока не будет результата
		cerr << "========================================" << endl;
		cerr << "Bypass step 1: tmp.size() == " << tmp.size() << endl;
		if (tmp.size() >= pawns.size()) {
			cerr << "No possible moves. tmp.size() == " << tmp.size() << endl;
			break;
		}

		std::vector<Cell> cells_whith_black_pawn;
		// пробежимся по доске и возьмем все черные, которые еще не "дома"

		for (int y = 0; y < BOARD_SIZE; ++y) {
			for (int x = 0; x < BOARD_SIZE; ++x) {
				const Cell& cell = board[x][y];
				// исключаем всех, кто уже "дома" и тех, кто в конце главного цикла попал в tmp,
				// но оказался не годным для хода
				if (x < 5 || y > 2) {
					if (cell.status == CellStatus::BLACK) {
						if (tmp.size() == 0) cells_whith_black_pawn.push_back(cell);
						else {
							bool is_good = true;
							for (const auto it : tmp) {
								if (cell.position_on_map.x == it.first.position_on_map.x &&
										cell.position_on_map.y == it.first.position_on_map.y) continue;
								else is_good = false;
							}
							// если не совпала ни с одним из tmp пушим в cells_whith_black_pawn
							// для последующих проверок
							if (is_good) cells_whith_black_pawn.push_back(cell);
						}
					}
				}
			}
		}
		cerr << "cells_with_black_pawn:" <<endl;
		for (const auto& it : cells_whith_black_pawn) {
			cerr << 'x' << it.position_on_map.x << ", y" << it.position_on_map.y << endl;
		}
		cerr << "Bypass step 2 sorting by y:" << endl;
		// сортируем по min_y и берем первый элемент в массив tmp
		std::stable_sort(begin(cells_whith_black_pawn), end(cells_whith_black_pawn),
										 [](const Cell& lhs, const Cell& rhs) {
			return lhs.position_on_map.y < rhs.position_on_map.y;
		});
		for (const auto& it : cells_whith_black_pawn) {
			cerr << 'x' << it.position_on_map.x << ", y" << it.position_on_map.y << endl;
		}
		cerr << "Bypass step 3 sorting by x:" << endl;
		tmp.push_back(std::make_pair(cells_whith_black_pawn.front(), Move::BLOCKED));

		// теперь сортируем по max_x и снова берем первый элемент
		std::stable_sort(begin(cells_whith_black_pawn), end(cells_whith_black_pawn),
										 [](const Cell& lhs, const Cell& rhs) {
			return lhs.position_on_map.x > rhs.position_on_map.x;
		});
		for (const auto& it : cells_whith_black_pawn) {
			cerr << 'x' << it.position_on_map.x << ", y" << it.position_on_map.y << endl;
		}
		if (cells_whith_black_pawn.size() > 2) tmp.push_back(std::make_pair(cells_whith_black_pawn.front(), Move::BLOCKED));
		else tmp.push_back(std::make_pair(cells_whith_black_pawn.back(), Move::BLOCKED));
		cerr << "tmp: " << endl;
		for (const auto& it : tmp) {
			cerr << 'x' << it.first.position_on_map.x << ", y" << it.first.position_on_map.y << endl;
		}
		// проверяем валидность выбранных двух пешек
		cerr << "Bypass step 4:" << endl;
		if (tmp.size() == 0) return result;
		for (auto& it : tmp) {
			// ищем возможность обхода
			it.second = GetMoveForBypass(it.first.position_on_map);
			std::string move = "BLOCKED";
			if (it.second == Move::UP) move = "UP";
			else if (it.second == Move::LEFT) move = "LEFT";
			cerr << "Pawn x" << it.first.position_on_map.x << ", y" << it.first.position_on_map.y << " move == " << move << endl;
			// если текущая пешка годная, находим по координатам клетки ее индекс в std::vector<Pawn> black_pawns
			// чтобы вернуть результат
			if (it.second != Move::BLOCKED) {
				for (int i = 0; i < pawns.size(); ++i) {
					if (pawns[i].pos.x == it.first.position_on_map.x &&
							pawns[i].pos.y == it.first.position_on_map.y) {
						result.push_back(i);
					}
				}
			}
		}
	}

	return std::move(result);
}

void Board::SetBadPosition(Pawn& pawn) {
	pawn.last_bad_pos = pawn.pos;
}
void Board::ResetBadPosition(Pawn& pawn) {
	pawn.last_bad_pos = Vec2(255, 255);
}

Board::Move Board::GetMoveDirection(const Pawn& pawn, const bool is_advance) const {
	Move move = BLOCKED;
	if (is_advance) {
		// если можно ходить и вправо и вниз, тогда возвращаем рандомный Move
		if (pawn.is_move_right && pawn.is_move_down) {
			if (rand() % 2) move = Move::RIGHT;
			else move = Move::DOWN;
		}
		else if (pawn.is_move_right) move = Move::RIGHT;
		else move = Move::DOWN;
	}
	else {
		move = GetMoveForBypass(pawn.pos);
	}
	return move;
}

Board::Move Board::GetMoveForBypass(const Vec2& pos) const {
	const int right_x = pos.x + 1;
	const int up_y = pos.y + 1;
	const int left_x = pos.x - 1;
	const int down_y = pos.y - 1;
	Move result = BLOCKED;
	// проверяем диагональную клетку (вверх+вправо) на выход за границы доски
	if (right_x < BOARD_SIZE && up_y < BOARD_SIZE) {
		// свободна ли она
		if (board[right_x][up_y].status != CellStatus::WHITE) {
			// можно ли будет обойти сверху?
			if (board[pos.x][up_y].status == CellStatus::FREE) {
				// если препятствие справа, обходим через верх
				if (board[right_x][pos.y].status != CellStatus::FREE
						&& (pos.y > 3 || down_y < 0 || pos.x == 4)) {
					result = Move::UP;
				}
			}
		}
	}
	// проверяем диагональную клетку (вниз+влево) на выход за границы доски
	if (left_x >= 0 && down_y >= 0) {
		// свободна ли она
		if (board[left_x][down_y].status != CellStatus::WHITE) {
			// можно ли будет обойти свлева?
			if (board[left_x][pos.y].status == CellStatus::FREE) {
				// если препятствие снизу, обходим слева
				if (board[pos.x][down_y].status != CellStatus::FREE &&
						(pos.x < 4 || right_x == BOARD_SIZE || pos.y == 3))	{
					result = Move::LEFT;
				}
			}
		}
	}
	return result;
}
