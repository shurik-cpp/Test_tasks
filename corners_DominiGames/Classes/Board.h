#ifndef BOARD_H
#define BOARD_H

#include "cocos2d.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <exception>

enum CellStatus { BLACK = -1, FREE = 0, WHITE, DEAD_HEAT };

struct Cell {
	cocos2d::Sprite* cell_sprite = nullptr; // спрайт клетки
	cocos2d::Sprite* pawn_sprite = nullptr; // спрайт пешки
	cocos2d::Label* label = nullptr;        // подпись клетки (для удобства, потом УДАЛИТЬ)
	CellStatus status = FREE;
	cocos2d::Vec2 position_on_map;  // хранит свои координаты на игровом поле (x, y)
	bool choised = false;

	// добавим подпись клетки (для удобства, потом УДАЛИТЬ)
	void RefreshLable() {
		std::string status;
		if (this->status == CellStatus::BLACK) status = "black";
		else if (this->status == CellStatus::WHITE) status = "white";
		else status = "free";
		std::stringstream ss;
		ss << 'x' << position_on_map.x << ", " << 'y' << position_on_map.y << '\n' << status;
		this->label->setString(ss.str());
	}
};

using BoardMap = std::vector<std::vector<Cell>>;

class Board {
public:
	explicit Board() {}

	BoardMap BuildBoard();

	//const Cell& GetCell(const int x, const int y) const { return board[x][y]; }
	//const Cell* GetChoised() const { return choised_pawn; }

	const Cell& GetCellByTouch(const cocos2d::Vec2& touchLocation) const;

	bool IsInGame() { return !is_game_over; }
	std::string GetWinner() const;
	bool IsWinner();

	bool IsChoised() const { return choised_pawn; }
	void SetChoised(const cocos2d::Vec2& coordinates);
	void CancelChoise(const cocos2d::Vec2& coordinates);

	void MoveIsPosibleTo(const cocos2d::Vec2& move_to);

	// true - ходит компьютер
	// false - ходит человек
	bool IsAiMove() const { return ai_move; }
	void AiMove();

private:
	const int BOARD_SIZE = 8;
	// using BoardMap = std::vector<std::vector<Cell>>;
	BoardMap board; // board[x][y] - x и y это позиция одной клетки

	struct Pawn {
		Pawn(const cocos2d::Vec2& p)
			: pos(p)
			, last_bad_pos(cocos2d::Vec2(255, 255)) {}

		cocos2d::Vec2 pos;
		cocos2d::Vec2 last_bad_pos;

		bool is_move_right = true;
		bool is_move_down = true;
		bool is_move_left = true;
		bool is_move_up = true;
		bool is_bypass = false;
	};

	std::vector<Pawn> black_pawns;
	std::vector<Pawn> white_pawns;
	Cell* choised_pawn = nullptr;
	bool is_game_over = false;
	CellStatus winner = CellStatus::FREE;
	bool ai_move = false; // хранит чей сейчас ход
	size_t bypassed_index = 0;

	cocos2d::Vec2 GetCellSize() const {
		return {board[0][0].cell_sprite->getContentSize().width, board[0][0].cell_sprite->getContentSize().height};
	}
	void ChangePlayer() { ai_move = !ai_move; }
	std::vector<Pawn> ArrangeCheckers(const CellStatus color = WHITE);

	enum Move {RIGHT, DOWN, BLOCKED, LEFT = 3, UP = 4};

	int GetRandomNumber(const int min, const int max) const;
	std::vector<size_t> GetNonBlockedPawnsForAdvance(const std::vector<Pawn>& pawns) const;
	std::vector<size_t> GetNonBlockedPawnsForBypass(const std::vector<Board::Pawn>& pawns) const;
	std::unordered_map<char, std::pair<int, int>> CheckRedZones(const CellStatus color) const;
	void SetBadPosition(Pawn& pawn);
	void ResetBadPosition(Pawn& pawn);
	Move GetMoveDirection(const Pawn& pawn, const bool is_advance) const;
	Move GetMoveForBypass(const cocos2d::Vec2& pawn) const;
	void ResetAllPawnsFlags(std::vector<Pawn>& pawns);
	void SetPawnsFlags(std::vector<Pawn>& pawns);



};

#endif // BOARD_H
