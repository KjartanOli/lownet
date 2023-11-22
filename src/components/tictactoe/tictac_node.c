/*
 * This file will contain the student's implementations for P4 (Fall 2023)
 *
 * - isolated so that the other team can work on the game protocols
 *
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "tictactoe.h"

#ifndef STANDALONE
#include "utility.h"
#else
	// Usage: int_min(A, B)
	// Pre:   None, other than those imposed by the type system
	// Value: The smaller of A and B
	inline int int_min(int a, int b) {return (a <= b) ? a : b;}

	// Usage: int_max(A, B)
	// Pre:   None, other than those imposed by the type system
	// Value: The larger of A and B
	inline int int_max(int a, int b) {return (a >= b) ? a : b;}
#endif


/*
 * Identical to lownet_crc()
 *
 * - there is no need to touch this!
 */
uint32_t crc24(const uint8_t* buf, size_t len)
{
	uint32_t reg = 0x00777777; // shift register
	static const uint32_t poly = 0x1800463ul;  // G(x)

	void process_byte(uint8_t b)
	{
		for(int i=0; i<8; i++)
			{
				reg = (reg<<1) | (b&1);
				b = b>>1;
				if ( reg & 0x1000000ul )
					reg = (reg^poly); // take mod G(x)
			}
	}

	for(size_t i = 0; i < len; ++i)
		process_byte(buf[i]);
	return reg;
}

uint32_t tictac_checksum(const tictactoe_t* board)
{
	return crc24((const uint8_t*) &board->data, sizeof(tictactoe_t));
}

/*
 * Milestone II:
 *
 * - Implement some super duper strategy for player s!
 * - Time budget is given in time_ms (milliseconds).
 * You can assume that it is in [2,10] seconds
 * - Specify the move by updating *x and *y (pointers)
 * - Return value 0 on making a succesful move (always!)
 */
typedef struct
{
	uint8_t x;
	uint8_t y;
} point_t;

typedef struct
{
	point_t top_left;
	uint8_t width;
	uint8_t height;
} attention_field_t;

uint8_t left_bound(const attention_field_t* field)
{
	return field->top_left.x;
}

uint8_t right_bound(const attention_field_t* field)
{
	return left_bound(field) + field->width;
}

uint8_t upper_bound(const attention_field_t* field)
{
	return field->top_left.y;
}

uint8_t lower_bound(const attention_field_t* field)
{
	return upper_bound(field) + field->height;
}

size_t field_squares(const attention_field_t* field)
{
	return (right_bound(field) - left_bound(field)) * (lower_bound(field) - upper_bound(field));
}

void extend_left(attention_field_t* field)
{
	field->top_left.x -= 1;
	field->width += 1;
}

void extend_right(attention_field_t* field)
{
	field->width += 1;
}

void extend_up(attention_field_t* field)
{
	field->top_left.y -= 1;
	field->height += 1;
}

void extend_down(attention_field_t* field)
{
	field->height += 1;
}

// Usage: square_free(BOARD, X, Y)
// Pre:   0 <= X < TICTACTOE_BOARD
//        0 <= Y < TICTACTOE_BOARD
// Value: true if square X,Y is free on BOARD
bool square_free(const tictactoe_t* board, uint8_t x, uint8_t y)
{
	return tictac_get(board, x, y) == EMPTY;
}

void field_expand_maybe(const tictactoe_t* board, attention_field_t* attention)
{
	size_t free_squares = 0;
	for (uint8_t i = upper_bound(attention), l = lower_bound(attention); i < l; ++i)
		for (uint8_t j = left_bound(attention), r = right_bound(attention); j < r; ++j)
			if (square_free(board, i, j))
				++free_squares;

	if ((float) free_squares / field_squares(attention) < 0.5)
		{
			if (left_bound(attention) != 0)
				extend_left(attention);
			if (right_bound(attention) != TICTACTOE_BOARD)
				extend_right(attention);
			if (upper_bound(attention) != 0)
				extend_up(attention);
			if (lower_bound(attention) != TICTACTOE_BOARD)
				extend_down(attention);
		}
}

void init_attention_field(const tictactoe_t* board, attention_field_t* attention)
{
	attention->width = 8;
	attention->height = 8;
	for (uint8_t i = 0; i < TICTACTOE_BOARD; ++i)
		{
			for (uint8_t j = 0; j < TICTACTOE_BOARD; ++j)
				{
					if (!square_free(board, i, j))
						{
							attention->top_left.x = (j == 0) ? j : j - 1;
							attention->top_left.y = (i == 0) ? i : i - 1;
							return;
						}
				}
		}
}

// Usage: maximise(BOARD, ATTENTION, MAX_DEPTH, DEPTH, PLAYER)
// Pre:   BOARD != NULL
//        ATTENTION != NULL
//        DEPTH <= MAX_DEPTH
//        PLAYER == PLAYER1 or PLAYER == PLAYER2
// Value: The maximum value achievable from BOARD, while only paying
//        attention to moves within ATTENTION
int maximise(const tictactoe_t* board,
						 attention_field_t* attention,
						 uint8_t max_depth,
						 uint8_t depth,
						 square_value_t player,
						 int alpha,
						 int beta);

// Usage: minimise(BOARD, ATTENTION, MAX_DEPTH, DEPTH, PLAYER)
// Pre:   BOARD != NULL
//        ATTENTION != NULL
//        DEPTH <= MAX_DEPTH
//        PLAYER == PLAYER1 or PLAYER == PLAYER2
// Value: The minimum value achievable from BOARD, while only paying
//        attention to moves within ATTENTION
int minimise(const tictactoe_t* board,
						 attention_field_t* attention,
						 uint8_t max_depth,
						 uint8_t depth,
						 square_value_t player,
						 int alpha,
						 int beta);

#define MAX_VALUE 1000
#define MIN_VALUE (-MAX_VALUE)

int maximise(const tictactoe_t* board,
						 attention_field_t* attention,
						 uint8_t max_depth,
						 uint8_t depth,
						 square_value_t player,
						 int alpha,
						 int beta)
{
	// TODO determinate whether MIN_VALUE or 0 produces better results
	if (depth == max_depth)
		return MIN_VALUE;
		// return 0;

	{
		int winner;
		if ((winner = tictac_game_over(board)))
			{
				if (winner == player)
					return MAX_VALUE - depth;
				else
					return MIN_VALUE + depth;
			}
	}

	tictactoe_t move;
	memcpy(&move, board, sizeof move);
	square_value_t next_player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
	int best = MIN_VALUE;

	field_expand_maybe(board, attention);

	for (uint8_t i = upper_bound(attention), l = lower_bound(attention); i < l; ++i)
		for (uint8_t j = left_bound(attention), r = right_bound(attention); j < r; ++j)
			if (square_free(board, i, j))
				{
					tictac_set(&move, i, j, player);
					int value = minimise(&move, attention, max_depth, depth + 1, next_player, alpha, beta);
					tictac_set(&move, i, j, EMPTY);

					best = int_max(best, value);
					alpha = int_max(alpha, best);

					if (beta <= alpha)
						return best;
				}

	return best;
}

int minimise(const tictactoe_t* board,
						 attention_field_t* attention,
						 uint8_t max_depth,
						 uint8_t depth,
						 square_value_t player,
						 int alpha,
						 int beta)
{
	// TODO determinate whether MAX_VALUE or 0 produces better results
	if (depth == max_depth)
		return MAX_VALUE;
	//return 0;

	{
		int winner;
		if ((winner = tictac_game_over(board)))
			{
				if (winner == player)
					return MIN_VALUE + depth;
				else
					return MAX_VALUE - depth;
			}
	}
	tictactoe_t move;
	memcpy(&move, board, sizeof move);
	square_value_t next_player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
	int best = MAX_VALUE;

	field_expand_maybe(board, attention);

	for (uint8_t i = upper_bound(attention), l = lower_bound(attention); i < l; ++i)
		for (uint8_t j = left_bound(attention), r = right_bound(attention); j < r; ++j)
			if (square_free(board, i, j))
				{
					tictac_set(&move, i, j, player);
					int value = maximise(&move, attention, max_depth, depth + 1, next_player, alpha, beta);
					tictac_set(&move, i, j, EMPTY);

					best = int_min(best, value);
					beta = int_min(beta, best);

					if (beta <= alpha)
						return best;
				}

	return best;
}

int tictac_move(const tictactoe_t* board, int* xp, int* yp, uint8_t s, uint32_t time_ms)
{
	/*
	 * TODO: figure out a better move!
	 */

	return tictac_auto(board, xp, yp, s);
}
