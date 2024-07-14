#include <memory>
#include <iostream>
#include <list>
#include <algorithm>


class Mass {
public:
	enum status {
		BLANK,
		PLAYER,
		ENEMY,
	};
private:
	status s_ = BLANK;
public:
	void setStatus(status s) { s_ = s; }
	status getStatus() const { return s_; }
	status getBlank() const { return BLANK; }
	bool put(status s) {
		if (s_ != BLANK) return false;
		s_ = s;
		return true;
	}

	bool operator==(const Mass& other) const
	{
		return this->s_ == other.s_;
	}
};

class Board;

class AI {
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type {
		TYPE_ORDERED = 0,
		TYPE_NEGA_MAX = 1,
		TYPE_ALPHA_BATA = 2,
		TYPE_NEGA_SCOUT = 3,
		TYPE_ADD_ORDER = 4,
	};

	static AI* createAi(type type);
};

// 順番に打ってみる
class AI_ordered : public AI {
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

class AI_nega_max : public AI {
private:
    int evaluate(Board &b, Mass::status current,int& best_x, int& best_y);

public:
	AI_nega_max() {}
	~AI_nega_max() {}

	bool think(Board& b);
};

class AI_alpha_bata :public AI {
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
		AI_alpha_bata() {}
	    ~AI_alpha_bata() {}

		bool think(Board& b);
};

class AI_nega_scout : public AI {
private:
	int evaluate(int limit, int alpha, int beta, Board &b, Mass::status current, int& best_x, int& best_y);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board &b);
};

class AI_add_order : public AI {
private:
	int evaluate(int limit,int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y, std::list<Board>& addlist);
public:
	AI_add_order() {}
	~AI_add_order() {}

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_NEGA_MAX:
		return new AI_nega_max();
		break;
	case TYPE_ALPHA_BATA:
		return new AI_alpha_bata();
		break;
	case TYPE_NEGA_SCOUT:
		return new AI_nega_scout();
		break;
	case TYPE_ADD_ORDER:
		return new AI_add_order();
		break;
	default:// case TYPE_ORDERED:
		return new AI_ordered();
		break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;
	friend class AI_nega_max;
	friend class AI_alpha_bata;
	friend class AI_nega_scout;
	friend class AI_add_order;
public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		 
	}
	Mass GetMass(Mass mass_)
	{
		return mass_;
	}

	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER) winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const {
		std::cout << "　　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << " " << x + 1 << "　";
		}
		std::cout << "\n　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << "＋－";
		}
		std::cout << "＋\n";
		for (int y = 0; y < BOARD_SIZE; y++) {
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "｜";
				switch (mass_[y][x].getStatus()) {
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "×";
					break;
				case Mass::BLANK:
					std::cout << "　";
					break;
				default:
//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
					std::cout << "　";
				}
			}
			std::cout << "｜\n";
			std::cout << "　";
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "＋－";
			}
			std::cout << "＋\n";
		}
	}

	bool operator==(const Board& other) const 
	{
		return this->mass_ == other.mass_;
	}
	bool SameCeack(std::list<Board>& addlist, const Board B)
	{
		for (const auto& item : addlist)
		{
			bool Ceack = true;
			for (int y = 0; y < Board::BOARD_SIZE; y++)
			{
				for (int x = 0; x < Board::BOARD_SIZE; x++)
				{
					auto a = item.mass_[y][x];
					auto b = B.mass_[y][x];
					if (a == b) 
					{
						continue;
					}
					else
					{
						Ceack = false;
						break;
					}
				}

				if(!Ceack)
					break;
			}

			if (!Ceack)
				continue;
			else
				return true;
		}
		return false;
	}
	void symmetryBoard(std::list<Board>& addlist, const Board B)
	{
		Board current = B;

		for (int i = 0; i < Board::BOARD_SIZE; i++)
		{
			current = RotateSymmetry(current);
			if(!SameCeack(addlist , current))
				addlist.push_back(current);
		}

		current = LineSymmetry(B);

		for (int i = 0; i < Board::BOARD_SIZE; i++)
		{
			current = RotateSymmetry(current);
			if (!SameCeack(addlist, current))
				addlist.push_back(current);
		}
	}
	Board LineSymmetry(const Board B)
	{
		Board lineSymmetry;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			for (int x = 0; x < Board::BOARD_SIZE; x++)
			{
				lineSymmetry.mass_[y][Board::BOARD_SIZE - 1 - x] = B.mass_[y][x];
			}
		}
		return lineSymmetry;
	}
	Board RotateSymmetry(const Board B)
	{
		Board Rotate_90;
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			for (int x = 0; x < Board::BOARD_SIZE; x++)
			{
				Rotate_90.mass_[x][Board::BOARD_SIZE - 1 - y] = B.mass_[y][x];
			}
		}
		return Rotate_90;
	}
	int NextCount(int C, const Board B)
	{
		int count = C;
		auto blank = B.mass_[0][0].getBlank();
		for (int y = 0; y < Board::BOARD_SIZE; y++)
		{
			for (int x = 0; x < Board::BOARD_SIZE; x++)
			{
				if (B.mass_[y][x].getStatus() != blank)
					count--;
			}
		}
		return count;
	}
};

bool AI_ordered::think(Board& b)
{
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}

int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	//集配設定
	int r = b.calc_result();
	if (r == current) return 10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -10000 - 1;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass &m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
		    m.setStatus(Mass::BLANK);

			if (score_max < score)
			{
				score_max = score;
				best_x = x;
				best_y = y;

			}
		}
	}
	return score_max;
}

bool AI_nega_max::think(Board &b)
{
	int best_x = -1, best_y;
	evaluate(b, Mass::ENEMY, best_x, best_y);
		if (best_x < 0) return false; //打つ手なし
		return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_alpha_bata::evaluate(int alpha,int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	//集配設定
	int r = b.calc_result();
	if (r == current) return 10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -10000 - 1;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass &m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(-beta, -alpha , b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (beta < score) {
				return (score_max < score) ? score : score_max;
			}

			if (score_max < score)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;
				best_x = x;
				best_y = y;

			}
		}
	}
	return score_max;
}

bool AI_alpha_bata::think(Board& b)
{
	int best_x, best_y;
	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999) return false; //打つ手なし
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board &board, Mass::status current, int& best_x, int& best_y) 
{
	if (limit-- == 0) return 0;
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	
	int r = board.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int a = alpha, b = beta;
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {

			Mass &m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(limit ,-b, -a, board, next, dummy, dummy);
			if (a < score && score < beta && !(x == 0 && y == 0) && limit <= 2)
			{
				a = -evaluate(limit, -beta, -score, board, next, dummy, dummy);
			}
			m.setStatus(Mass::BLANK);

			if (a < score)
			{
				a = score;
				best_x = x;
				best_y = y;
			}

			if (beta <= a) {
				return a;
			}
			b = a + 1;
		}
	}
	return a;
	
}

bool AI_nega_scout::think(Board &b)
{
	int best_x, best_y;
	if (evaluate(7, -10000, 10000, b, Mass::ENEMY, best_x, best_y) < -9999)
		return false; //打つ手なし

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
	
}

int AI_add_order::evaluate(int limit,int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y, std::list<Board>& addlist)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	int r = board.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int y[9] = { 1,0,0,2,2,0,1,1,2 };
	int x[9] = { 1,0,2,0,2,1,0,2,1 };

	int score_max = -10000 - 1;

	for (int i = 0; i < Board::BOARD_SIZE * Board::BOARD_SIZE; i++)
	{
			Mass& m = board.mass_[y[i]][x[i]];
			if (m.getStatus() != Mass::BLANK) continue;
	
			if (limit-- > 0) 
			{
				const Board B = board;
				if (board.SameCeack(addlist, B)) 
				{
					return 0;
				}

				addlist.push_back(board);
				board.symmetryBoard(addlist, B);
			}
				
			m.setStatus(current);
			int dummy;
			int score = -evaluate(limit ,-beta, -alpha, board, next, dummy, dummy ,addlist);
			m.setStatus(Mass::BLANK);

			if (beta < score)
			{
				return (score_max < score) ? score : score_max;
			}

			if (score_max < score)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha;
				best_x = x[i];
				best_y = y[i];
			}
	}
	return score_max;
}

bool AI_add_order::think(Board& b) 
{
	int best_x, best_y;
	Board& list = b;
	std::list<Board> addlist;
	int NextCount = b.NextCount(4, b);
	//std::cout << NextCount << std::endl;

	if (evaluate(NextCount, -10000, 10000, b, Mass::ENEMY, best_x, best_y, addlist) <= -9999)
		return false; //打つ手なし

	//std::cout << addlist.size() << std::endl;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

class Game
{
private:
	const AI::type ai_type = AI::TYPE_ADD_ORDER;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game() {
		pAI_ = AI::createAi(ai_type);
	}
	~Game() {
		delete pAI_;
	}

	bool put(int x, int y) {
		bool success = board_.put(x, y);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think() {
		bool success = pAI_->think(board_);
		if (success) winner_ = board_.calc_result();
		return success;
	}

	Board::WINNER is_finised() {
		return winner_;
	}

	void show() {
		board_.show();
	}
};

void show_start_message()
{
	std::cout << "========================" << std::endl;
	std::cout << "       GAME START       " << std::endl;
	std::cout << std::endl;
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER) {
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
	{
		std::cout << "You lose..." << std::endl;
	}
	else {
		std::cout << "Draw" << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	for (bool end = false;end ==false;) {// 無限ループ
		show_start_message();

		// initialize
		unsigned int turn = 0;
		std::shared_ptr<Game> game(new Game());

		while (1) {
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (0 == turn) {
				// user input
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row;				
					std::cin >> col;					
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}

	return 0;
}