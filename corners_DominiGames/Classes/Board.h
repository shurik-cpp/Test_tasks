#ifndef BOARD_H
#define BOARD_H

#include "cocos2d.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <exception>

enum CellStatus { BLACK = -1, FREE, WHITE };

struct Cell {
	cocos2d::Sprite* cell_sprite = nullptr; // спрайт клетки
	cocos2d::Sprite* pawn_sprite = nullptr; // спрайт пешки
	cocos2d::Label* label = nullptr;        // подпись клетки (для удобства, потом УДАЛИТЬ)
	CellStatus status = FREE;
	cocos2d::Vec2 position_on_map;
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

	bool IsChoised() const { return choised_pawn; }
	void SetChoised(const cocos2d::Vec2& coordinates);
	void CancelChoise(const cocos2d::Vec2& coordinates);

	void MoveIsPosibleTo(const cocos2d::Vec2& move_to);

	// true - ходит компьютер
	// false - ходит человек
	bool IsAiMove() const { return ai_move; }
	void AiMove();

private:
	// using BoardMap = std::vector<std::vector<Cell>>;
	BoardMap board; // board[x][y] - x и y это позиция одной клетки
	std::vector<Cell> black_pawns;
	std::vector<Cell> white_pawns;
	bool ai_move = false; // хранит чей сейчас ход
	Cell* choised_pawn = nullptr;


	cocos2d::Vec2 GetCellSize() const {
		return {board[0][0].cell_sprite->getContentSize().width, board[0][0].cell_sprite->getContentSize().height};
	}
	void ChangePlayer() { ai_move = !ai_move; }
	std::vector<Cell> ArrangeCheckers(const CellStatus color = WHITE);



};

#endif // BOARD_H
