/*
 * This file will contain the student's implementations for P4 (Fall 2023)
 *
 * - isolated so that the other team can work on the game protocols
 *
 */

#include <string.h>
#include <stdint.h>

#include "tictactoe.h"

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

static const uint8_t MASKS[] = {
	0b11000000,
	0b00110000,
	0b00001100,
	0b00000011
};

static const uint8_t SHIFTS[] = {6, 4, 2, 0};

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

// Usage: tictac_base2_encode(BOARD, BUFFER)
// Pre:   BOARD != NULL, BUFFER != NULL
//        BUFFER is of length TICTACTOE_N2
// Post:  The base-2 encoding of BOARD has been written to BUFFER
void tictac_base2_encode(const tictactoe_t* board, tictactoe_board_t buffer)
{
	for (size_t i = 0, j = 0; i < TICTACTOE_N; i += 4, ++j)
		{
			buffer[j] = (set_square(0, 0, board->board[i])
									 + set_square(0, 1, board->board[i+1])
									 + set_square(0, 2, board->board[i+2])
									 + set_square(0, 3, board->board[i+3]));
		}
}

square_value_t tictac_base2_get(const tictactoe_board_t board, uint8_t i, uint8_t j)
{
	uint8_t idx = (i + TICTACTOE_BOARD * j) / 4;
	uint8_t sqr = (i + TICTACTOE_BOARD * j) % 4;
	return get_square(board[idx], sqr);
}

int tictac_base2_set(tictactoe_board_t board, uint8_t i, uint8_t j, square_value_t s)
{
	if (tictac_base2_get(board, i, j))
		return -1;
	uint8_t idx = (i + TICTACTOE_BOARD * j) / 4;
	uint8_t sqr = (i + TICTACTOE_BOARD * j) % 4;
	board[idx] = set_square(board[idx], sqr, s);
	return 0;
}

void print_board_base2(const tictactoe_board_t b)
{
	for(int i = 0; i < TICTACTOE_BOARD; ++i)
		{
			printf("%02d: ", i + 1);
			for(int j = 0; j < TICTACTOE_BOARD; ++j)
				{
					square_value_t s = tictac_base2_get(b, i, j);
					printf("%c", (s == EMPTY ? '_' : (s == PLAYER1 ? 'x' : 'o')));
				}
			putc('\n', stdout);
		}
}

// Usage: tictac_base2_decode(BUFFER, BOARD)
// Pre:   BUFFER != NULL, BOARD != NULL
//        BUFFER is a base-2 representation of a game board
// Post:  BUFFER has been decoded into BOARD
void tictac_base2_decode(const tictactoe_board_t buffer, tictactoe_t* board)
{
	for (size_t i = 0, j = 0; i < TICTACTOE_N; i += 4, ++j)
		{
			uint8_t squares = buffer[j];
			uint8_t tmp[] = {
				get_square(squares, 0),
				get_square(squares, 1),
				get_square(squares, 2),
				get_square(squares, 3),
			};

			memcpy(board->board + i, tmp, 4);
		}
}

uint32_t tictac_checksum(const tictactoe_t* b)
{
	uint8_t b4[TICTACTOE_N2]; // work here

	/* Init the memory chunk */
	memset(b4, 0, TICTACTOE_N2); // clear all?

	tictac_base2_encode(b, b4);

	/* Then compute the CRC code */
	return crc24(b4, TICTACTOE_N2 );
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

int tictac_move(const tictactoe_t* b, int* xp, int* yp, uint8_t s, uint32_t time_ms)
{
	/*
	 * TODO: figure out a better move!
	 */
	return tictac_auto(b, xp, yp, s);
}
