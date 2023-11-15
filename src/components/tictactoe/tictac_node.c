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

square_value_t tictac_base2_get(const tictactoe_board_t* board, uint8_t i, uint8_t j)
{
	uint8_t idx = (i + TICTACTOE_BOARD * j) / 4;
	uint8_t sqr = (i + TICTACTOE_BOARD * j) % 4;
	return get_square(board->data[idx], sqr);
}

int tictac_base2_set(tictactoe_board_t* board, uint8_t i, uint8_t j, square_value_t s)
{
	if (tictac_base2_get(board, i, j))
		return -1;
	uint8_t idx = (i + TICTACTOE_BOARD * j) / 4;
	uint8_t sqr = (i + TICTACTOE_BOARD * j) % 4;
	board->data[idx] = set_square(board->data[idx], sqr, s);
	return 0;
}

uint32_t tictac_checksum(const tictactoe_board_t* board)
{
	/* Then compute the CRC code */
	return crc24((const uint8_t*) &board->data, sizeof(tictactoe_board_t));
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
int tictac_move(const tictactoe_board_t* board, int* xp, int* yp, uint8_t s, uint32_t time_ms)
{
	/*
	 * TODO: figure out a better move!
	 */

	return tictac_auto(board, xp, yp, s);
}
