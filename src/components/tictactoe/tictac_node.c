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
int tictac_move(const tictactoe_t* board, int* xp, int* yp, uint8_t s, uint32_t time_ms)
{
	/*
	 * TODO: figure out a better move!
	 */
// Usage: square_free(BOARD, X, Y)
// Pre:   0 <= X < TICTACTOE_BOARD
//        0 <= Y < TICTACTOE_BOARD
// Value: true if square X,Y is free on BOARD
bool square_free(const tictactoe_t* board, uint8_t x, uint8_t y)
{
	return tictac_get(board, x, y) == EMPTY;
}


	return tictac_auto(board, xp, yp, s);
}
