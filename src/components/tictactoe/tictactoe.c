#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "tictactoe.h"

#ifdef STANDALONE
#include "tictac_node.c" // This is ugly as sin, but it works for now.
#endif

/*
 * Sample code for the tictactoe game with the aim of 5-in-a-row.
 *
 * Compile with -DSTANDALONE to produce a test binary for stdin/out:
 *
 * gcc -Wall -DSTANDALONE -o tictactoe tictactoe.c
 *
 *
 * Oct 29, 2023 -- esa@hi.is
 */

static const uint8_t MASKS[] = {
	0b00000011,
	0b00001100,
	0b00110000,
	0b11000000,
};

static const uint8_t SHIFTS[] = {0, 2, 4, 6};

// Usage: get_square(SQUARES, IDX)
// Pre:   0 <= IDX < 4
// Value: The value of square IDX within SQUARES
square_value_t get_square(uint8_t squares, uint8_t idx)
{
	return (squares & MASKS[idx]) >> SHIFTS[idx];
}

// Usage: set_square(SQUARES, IDX, VALUE)
// Pre:   0 <= IDX < 4
// Value: SQUARES updated such that square IDX has value VALUE
uint8_t set_square(uint8_t squares, uint8_t idx, square_value_t value)
{
	return (squares & (~MASKS[idx])) + (value << SHIFTS[idx]);
}

// Usage: base2_encode_squares(SQUARES)
// Pre:   SQUARES != NULL, SQUARES is of length 4
// Value: The base-2 encoding of of SQUARES
uint8_t base2_encode_squares(const uint8_t* squares)
{
	uint8_t r = 0;
	for (size_t i = 0; i < 4; ++i)
		r = set_square(r, i, squares[i]);

	return r;
}

// Usage: base2_decode_squares(SQUARES, BUFFER)
// Pre:   BUFFER != NULL, BUFFER is of length 4
// Post:  The squares encoded in SQUARES have been decoded into BUFFER
void base2_decode_squares(uint8_t squares, uint8_t* buffer)
{
	for (size_t i = 0; i < 4; ++i)
		buffer[i] = get_square(squares, i);
}

// Usage: base3_encode_squares(SQUARES)
// Pre:   SQUARES != NULL, SQUARES is of length 5
// Value: The base-3 encoding of of SQUARES
uint8_t base3_encode_squares(const uint8_t* squares)
{
	uint8_t v = 0;
	for (size_t i = 0, B = 1; i < 5; ++i, B *= 3)
			v += squares[i] * B;

	return v;
}

// Usage: base3_decode_squares(SQUARES, BUFFER)
// Pre:   BUFFER != NULL, BUFFER is of length 5
// Post:  The squares encoded in SQUARES have been decoded into BUFFER
void base3_decode_squares(uint8_t squares, uint8_t* buffer)
{
	for (size_t i = 0, B = squares; i < 5; ++i, B /= 3)
		buffer[i] = B % 3;
}

int tictac_encode(const tictactoe_t* board, tictactoe_payload_t* p)
{
	/*
	 * Loop invariant:
	 *
	 * 0 <= i < TICTACTOE_N2
	 * 0 <= j < TICTACTOE_N3
	 *
	 * The base-2 encoded squares in board[..i) have been re-encoded
	 * into base-3 in p->bdata[..j)
	 */
	size_t i = 0, j = 0;
	while (i < TICTACTOE_N2)
		{
			// ghost old_i = i
			// ghost old_j = j
			uint8_t buffer[TICTAC_B2_SQUARES_PER_BYTE * TICTAC_B3_SQUARES_PER_BYTE];
			static_assert(sizeof(buffer) == 20);

			/*
			 * Loop invariant:
			 *
			 * 0 <= z < sizeof(buffer)
			 *
			 * The squares encoded in board[old_i..i) have been decoded into
			 * buffer[..z)
			 */
			for (size_t z = 0; z < sizeof(buffer); z += TICTAC_B2_SQUARES_PER_BYTE, ++i)
				base2_decode_squares(board->data[i], buffer + z);

			/*
			 * Loop invariant:
			 *
			 * 0 <= z < sizeof(buffer)
			 *
			 * The squares stored in buffer[..z) have been encoded into
			 * p->bdata[..j)
			 */
			for (size_t z = 0; z < sizeof(buffer); z += TICTAC_B3_SQUARES_PER_BYTE, ++j)
				p->bdata[j] = base3_encode_squares(buffer + z);
		}

	return (i == TICTACTOE_N2 && j == TICTACTOE_N3) ? 0 : -1;
}

/*
 * Decode the dense packet representation back to the
 * internal representation in memory that is fast to work with
 */
int tictac_decode(const tictactoe_payload_t* p, tictactoe_t* board)
{
	/*
	 * Loop invariant:
	 *
	 * 0 <= i < TICTACTOE_N3
	 * 0 <= j < TICTACTOE_N2
	 *
	 * The base-3 encoded squares in p->bdata[..i) have been re-encoded
	 * into base-2 in board[..j)
	 */
	size_t i = 0, j = 0;
	while (i < TICTACTOE_N3)
		{
			// ghost old_i = i
			// ghost old_j = j
			uint8_t buffer[TICTAC_B2_SQUARES_PER_BYTE * TICTAC_B3_SQUARES_PER_BYTE];
			static_assert(sizeof(buffer) == 20);

			/*
			 * Loop invariant:
			 *
			 * 0 <= z < sizeof(buffer)
			 *
			 * The squares encoded in p->bdata[old_i..i) have been decoded
			 * into buffer[..z)
			 */
			for (size_t z = 0; z < sizeof(buffer); z += TICTAC_B3_SQUARES_PER_BYTE, ++i)
				base3_decode_squares(p->bdata[i], buffer + z);

			/*
			 * Loop invariant:
			 *
			 * 0 <= z < sizeof(buffer)
			 *
			 * The squares stored in buffer[..z) have been encoded into
			 * board[old_j..j)
			 */
			for (size_t z = 0; z < sizeof(buffer); z += TICTAC_B2_SQUARES_PER_BYTE, ++j)
				board->data[j] = base2_encode_squares(buffer + z);
		}

	return (i == TICTACTOE_N3 && j == TICTACTOE_N2) ? 0 : -1;
}


/*
 * Subroutines with no range checks!
 *
 * tictac_get() returns the piece at (i,j), if any (0,1,2)
 * set_board() sets piece s to (i,j), return zero on success.
 *
 */
square_value_t tictac_get(const tictactoe_t* board, uint8_t i, uint8_t j)
{
	uint8_t idx = (i * TICTACTOE_BOARD + j) / 4;
	uint8_t sqr = (i * TICTACTOE_BOARD + j) % 4;
	return get_square(board->data[idx], sqr);
}

int tictac_set(tictactoe_t* board, uint8_t i, uint8_t j, square_value_t s)
{
	uint8_t idx = (i * TICTACTOE_BOARD + j) / 4;
	uint8_t sqr = (i * TICTACTOE_BOARD + j) % 4;
	board->data[idx] = set_square(board->data[idx], sqr, s);
	return 0;
}

void empty_board(tictactoe_t* board)
{
	memset(board, 0, sizeof(tictactoe_t));
}

/*
 * Returns the winner, or zero if no winner
 */
int tictac_game_over(const tictactoe_t* board)
{
	int evaluate(int i, int j, int dx, int dy)
	{
		int i1 = i + 4 * dx;
		int j1 = j + 4 * dx;
		if (i < 0 || i >= TICTACTOE_BOARD ||
				j < 0 || j >= TICTACTOE_BOARD)
			{
				printf("Oops");
				while(1)
					;
			}

		if (i1 < 0 || i1 >= TICTACTOE_BOARD ||
				j1 < 0 || j1 >= TICTACTOE_BOARD)
			return 0;
		square_value_t s = tictac_get(board, i, j);
		if (!s)
			return 0;
		for(int k = 0; k < 4; ++k) // four more and we are done!
			{
				i += dx;
				j += dy;
				if (tictac_get(board, i, j) != s)
					return 0;
			}
		return s; // winner found! 1 or 2
	}

	for(int i = 0; i < TICTACTOE_BOARD; ++i)
		{
			for(int j = 0; j < TICTACTOE_BOARD; ++j)
				{
					int r =
						evaluate(i, j, 1, 0) |
						evaluate(i, j, 0, 1) |
						evaluate(i, j, 1, 1) |
						evaluate(i, j, 1,-1);
					if (r)
						return r;
				}
		}
	return 0;
}

/* some elementary 16-bit randomness for actions! */
int my_random(void)
{
	static uint32_t st = 1;
	return (st = (64733*st + 1) & 0xffff);
}

/*
 * Simple heuristic for our end.
 * Returns zero on a successful choice
 */
int tictac_auto(const tictactoe_t* board, uint8_t* x, uint8_t* y, uint8_t s)
{
	int best_v = -1;
	int best_i = 0;
	int best_j = 0;
	int m = TICTACTOE_BOARD - 1;
	int markers = 0; // count the number of symbols
	int equal = 0;

	for(int i = 0; i < TICTACTOE_BOARD; ++i)
		{
			for(int j = 0; j < TICTACTOE_BOARD; ++j)
				{
					if (tictac_get(board, i, j))
						{
							markers++;
							continue;
						}
					int v = 0; // value of this action
					if (i > 0 && tictac_get(board, i - 1, j)) v++;
					if (j > 0 && tictac_get(board, i, j - 1)) v++;
					if (i > 0 && j>0 && tictac_get(board, i - 1, j - 1)) v++;
					if (i < m && tictac_get(board, i + 1, j)) v++;
					if (j < m && tictac_get(board, i, j + 1)) v++;
					if (i < m && j < m && tictac_get(board, i + 1, j + 1)) v++;

					if (v < best_v)
						continue;

					if (v == best_v)
						{
							equal++;
							if (my_random() > 0xffff/equal)
								continue;
						}
					else
						equal = 1;
					/* accept this as the best */
					best_v = v;
					best_i = i;
					best_j = j;
				}
		}
	//printf("auto loops done, best %d, equal=%d\n", best_v, equal);
	if (best_v < 0)
		return -1;
	if (markers == 0)
		{
			best_i = TICTACTOE_BOARD / 2;
			best_j = TICTACTOE_BOARD / 2;
		}
	//printf("best: %d,%d\n", best_i, best_j);
	*x = best_i;
	*y = best_j;
	return 0;
}


/***********************************************************/

#ifdef STANDALONE
void print_board(const tictactoe_t* board)
{
	for(int i = 0; i < TICTACTOE_BOARD; ++i)
		{
			printf("%02d: ", i + 1);
			for(int j = 0; j < TICTACTOE_BOARD; ++j)
				{
					square_value_t s = tictac_get(board, i, j);
					printf("%c", (s == EMPTY ? '_' : (s == PLAYER1 ? 'x' : 'o')));
				}
			putc('\n', stdout);
		}
}

int main(int argc, char** argv)
{
	tictactoe_payload_t payload;
	tictactoe_t board;
	uint8_t res, x, y;
	int round = 0;

	empty_board(&board);

	/*
	 * Test the game by playing up to 10 rounds
	 */
	while (!(res = tictac_game_over(&board)))
		{
			/* check that encoding/decoding to packet format works */
			if (tictac_encode(&board, &payload))
				{
					printf("Board encoding failed!\n");
					return -1;
				}
			empty_board(&board); // no cheating!
			if (tictac_decode(&payload, &board))
				{
					printf("Board decoding failed!\n");
					return -1;
				}

			if (!(round & 1))
				{
					print_board(&board);
					tictac_move(&board, &x, &y, 1, 3);
					tictac_set(&board, x, y, 1);
				}
			else if (!tictac_auto(&board, &x, &y, 2) &&
							 !tictac_set(&board, x, y, 2))
				{
					// ok move!
				}
			else
				{
					printf("Heuristic did not find a any move!\n");
					return -1;
				}
			round++;
		}
	print_board(&board);
	printf("Result: %d\n", res);
	return 0;
}

#endif
/***********************************************************/
