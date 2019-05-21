#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include "bitmap.h"

#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "xil_exception.h"
#include "vga_periph_mem.h"




#define UP     0b01000000
#define DOWN   0b00000100
#define LEFT   0b00100000
#define RIGHT  0b00001000
#define CENTER 0b00010000

#define SW0 0b00000001
#define SW1 0b00000010

#define SIZE   8
#define WIDTH  8

#define WHITE  1
#define BLACK -1


// CECA U PARIZU

enum Piece { DEAD = 0, PAWN = 10, ROOK = 50, KNIGHT = 30, BISHOP = 40, QUEEN = 200, KING = 1000 };

/* Custom structures used in game */

typedef struct point_st {
    int x, y;
} POINT;

typedef struct chess_piece_st {
    POINT point;
    int piece;
    int color;
} PIECE;

typedef struct square_st {
    POINT point;
    int color;
    PIECE* piece;
} SQUARE;


/* Global variables used in game */

static int player_turn = WHITE;
static int X = 0;
static int Y = 0;

static PIECE black[WIDTH<<1] = {};
static PIECE white[WIDTH<<1] = {};
static SQUARE board[WIDTH][WIDTH] = {};


/* Functions used in game */

void setup_board(SQUARE board[][WIDTH], PIECE black[], PIECE white[]) {
	int x, y;
    for (y = 0; y < WIDTH; y++) {
        for (x = 0; x < WIDTH; x++) {
            board[y][x].point.x = x;
            board[y][x].point.y = y;
            board[y][x].piece = NULL;

            if (y == 0)
                board[y][x].piece = (PIECE*)(black + x);
            if (y == 1)
                board[y][x].piece = (PIECE*)(black + WIDTH + x);
            if (y == 6)
                board[y][x].piece = (PIECE*)(white + WIDTH + x);
            if (y == 7)
                board[y][x].piece = (PIECE*)(white + x);

            if (((x+y) & 1) == 0)
                board[y][x].color = WHITE;
            else
                board[y][x].color = BLACK;
        }
    }
}


void setup_players(PIECE black[], PIECE white[]) {

    // PAWNS Black & White
	int i;

    for (i = 0; i < WIDTH; i++) {
        black[WIDTH+i].point.x = i;
        black[WIDTH+i].point.y = 1;
        black[WIDTH+i].piece = PAWN;
        black[WIDTH+i].color = BLACK;

        white[WIDTH+i].point.x = i;
        white[WIDTH+i].point.y = 6;
        white[WIDTH+i].piece = PAWN;
        white[WIDTH+i].color = WHITE;
    }

    // ROOKS's Black & White

    black[0].point.x = 0;
    black[0].point.y = 0;
    black[0].piece = ROOK;
    black[0].color = BLACK;

    white[0].point.x = 0;
    white[0].point.y = 7;
    white[0].piece = ROOK;
    white[0].color = WHITE;

    black[7].point.x = 7;
    black[7].point.y = 0;
    black[7].piece = ROOK;
    black[7].color = BLACK;

    white[7].point.x = 7;
    white[7].point.y = 7;
    white[7].piece = ROOK;
    white[7].color = WHITE;

    // KNIGHT's Black & White

    black[1].point.x = 1;
    black[1].point.y = 0;
    black[1].piece = KNIGHT;
    black[1].color = BLACK;

    white[1].point.x = 1;
    white[1].point.y = 7;
    white[1].piece = KNIGHT;
    white[1].color = WHITE;

    black[6].point.x = 6;
    black[6].point.y = 0;
    black[6].piece = KNIGHT;
    black[6].color = BLACK;

    white[6].point.x = 6;
    white[6].point.y = 7;
    white[6].piece = KNIGHT;
    white[6].color = WHITE;

    // BISHOP's Black & White

    black[2].point.x = 2;
    black[2].point.y = 0;
    black[2].piece = BISHOP;
    black[2].color = BLACK;

    white[2].point.x = 2;
    white[2].point.y = 7;
    white[2].piece = BISHOP;
    white[2].color = WHITE;

    black[5].point.x = 5;
    black[5].point.y = 0;
    black[5].piece = BISHOP;
    black[5].color = BLACK;

    white[5].point.x = 5;
    white[5].point.y = 7;
    white[5].piece = BISHOP;
    white[5].color = WHITE;

    // QUEEN's Black & White

    black[4].point.x = 4;
    black[4].point.y = 0;
    black[4].piece = QUEEN;
    black[4].color = BLACK;

    white[4].point.x = 4;
    white[4].point.y = 7;
    white[4].piece = QUEEN;
    white[4].color = WHITE;

    // KING's Black & White

    black[3].point.x = 3;
    black[3].point.y = 0;
    black[3].piece = KING;
    black[3].color = BLACK;

    white[3].point.x = 3;
    white[3].point.y = 7;
    white[3].piece = KING;
    white[3].color = WHITE;
}

void draw_piece(POINT in, POINT out) {
    int RGB, R, G, B;
    unsigned short tmp = 0;
    size_t size = 30;
    int x, y, iy, ix, ii, ox, oy, oi;

	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			ox = ( 40 + (out.x * size) ) + x; // konverzija mozda sam zajebao
			oy = ( out.y * size ) + y;
			oi = oy * 320 + ox;

			ix = in.x + x;
			iy = in.y + y;
			ii = iy * bitmap.width + ix;

			tmp = ( (unsigned short)bitmap.pixel_data[ii * bitmap.bytes_per_pixel + 1] << 8 ) | (unsigned short)bitmap.pixel_data[ii * bitmap.bytes_per_pixel + 0];

			R =   ((tmp >> 0)  & 0x1f) >> 2;

			G =   ((tmp >> 5)  & 0x3f) >> 3;

			B =   ((tmp >> 11) & 0x1f) >> 2;

			B <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi * 4, RGB);
        }
    }
}


void draw_field(POINT out, int color) {
    int RGB; // beton verzija "crna" ili "bela"
    int x, y, ox, oy, oi;

    size_t size = 30;

    RGB = (color == BLACK ? 0x163 : 0x1F5);

	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			ox = ( 40 + ( out.x * size) ) + x;
			oy = ( out.y * size ) + y;
			oi = oy * 320 + ox;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi * 4, RGB);
        }
    }
}


/*
    drawing cursor and clearing old cursor value, BATMAN 0 [CLEAR]
                                                  BATMAN 1 [WRITE]
*/
void draw_cursor(int startX, int startY, int endX, int endY, int batman) {
    int RGB;
    int x, y, i;

    if (batman)
        RGB = 0x38;
    else
        if (( (startX-40)/30 + (startY-40)/30 ) & 1)
            RGB = 0x1F5;
        else
            RGB = 0x163;

	// gornja ivica
	for (x = startX; x < endX; x++) {
		for (y = startY; y < startY + 2; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, RGB);
		}
	}

	// donja ivica
	for (x = startX; x < endX; x++) {
		for (y = endY - 2; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, RGB);
		}
	}

	// leva ivica
	for (x = startX; x < startX + 2; x++) {
		for (y = startY; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, RGB);
		}
	}

	// desna ivica
	for (x = endX - 2; x < endX; x++) {
		for (y = startY; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, RGB);
		}
	}
}


void draw_board(SQUARE board[][WIDTH]) {

    POINT in, out;
    int x, y;

    out.x = out.y = 0;

    for (y = 0; y < WIDTH; y++, out.y++) {
    	out.x = 0;
        for (x = 0; x < WIDTH; x++, out.x++) {
            if (board[y][x].piece != NULL)  {

                switch (board[y][x].piece->piece) {

                    case PAWN:

                        in.x = 0;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;

                    case ROOK:

                        in.x = 30;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;

                    case KNIGHT:

                        in.x = 60;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;

                    case BISHOP:

                        in.x = 90;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;

                    case QUEEN:

                        in.x = 120;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;

                    case KING:

                        in.x = 150;

                        if ( board[y][x].color == WHITE && board[y][x].piece->color == WHITE ) {
                            in.y = 0;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == WHITE && board[y][x].piece->color == BLACK ) {
                            in.y = 30;
                            draw_piece(in, out);
                        }
                        else if ( board[y][x].color == BLACK && board[y][x].piece->color == WHITE ) {
                            in.y = 60;
                            draw_piece(in, out);
                        }
                        else {
                            in.y = 90;
                            draw_piece(in, out);
                        }
                        break;
                    default:
                        break;
                }
            } else {
                if (board[y][x].color == WHITE)
                    draw_field(out, WHITE);
                else
                    draw_field(out, BLACK);
            }
        }
    }
}


//function that controls switches and buttons
void game(PIECE gray[]) {
	int i = 0, cnt;
	int startX = 40, startY = 0, endX = 70, endY = 30;


	typedef enum { NOTHING_PRESSED, SOMETHING_PRESSED } btn_state_t;

	btn_state_t btn_state = NOTHING_PRESSED;

	startX=gray[i].point.x*30+40;
	startY=gray[i].point.y*30;
	endX=startX+30;
	endY=startY+30;
	draw_cursor(startX, startY, endX, endY, 1);

	while(1) {
		if (btn_state == NOTHING_PRESSED) {
			btn_state = SOMETHING_PRESSED;

			if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & DOWN) == 0) {
					draw_cursor(startX, startY, endX, endY, 0);

					Y++;

					if (i > 7 )
					{
						i -= 8;

						startX = gray[i].point.x * 30 + 40;
						startY = gray[i].point.y * 30;
						endX=startX+30;
						endY=startY+30;
					}

					draw_cursor(startX, startY, endX, endY, 1);
			}
			else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0) {
					draw_cursor(startX, startY, endX, endY, 0);

					Y--;

					if (i < 8) {
						i += 8;

						startX = gray[i].point.x * 30 + 40;
						startY = gray[i].point.y * 30;
						endX=startX+30;
						endY=startY+30;
					}

					draw_cursor(startX, startY, endX, endY, 1);
			}
			else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0) {
				if (endX < 280) {
					draw_cursor(startX, startY, endX, endY, 0);

					X++;
					if(i<15){
						i++;
						startX = gray[i].point.x * 30 + 40;
						startY = gray[i].point.y * 30;
						endX=startX+30;
						endY=startY+30;
					}
					draw_cursor(startX, startY, endX, endY, 1);
				}
			}
			else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0) {
				if (startX > 40) {
					draw_cursor(startX, startY, endX, endY, 0);

					X--;
					if(i > 0)
					{
						i--;
						startX = gray[i].point.x * 30 + 40;
						startY = gray[i].point.y * 30;
						endX=startX+30;
						endY=startY+30;
					}

					draw_cursor(startX, startY, endX, endY, 1);
				}
			}
		}

		for (cnt=0; cnt < 2000000; cnt++);
		btn_state = NOTHING_PRESSED;
	}
}


void playable() {
	int i, x, y;
	for (i = 0; i < WIDTH; i++) {
		x = white[WIDTH+i].point.x * 30 + 40;
		y = white[WIDTH+i].point.y * 30;

		draw_cursor(x, y, x+30, y+30, 1);
	}
}


/*-----------------------------------MAIN--------------------------------------*/

int main() {

	int x, y, i;

	init_platform();

	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); // direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); // display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x0); // show frame      2
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0xff); // font size       3
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); // foreground 4
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF); // background color 5
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); // frame color      6
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 1);

	// Mihajlo Solda's gay background
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x5F);
		}
	}

    setup_players(black, white);
    setup_board(board, black, white);


    board[4][3].piece = board[6][3].piece;
    board[6][3].piece = NULL;
    white[WIDTH+3].point.x = 3;
    white[WIDTH+3].point.y = 4;






    draw_board(board);

  //  playable();
	game(black);

	cleanup_platform();

	return 0;
}

/*-----------------------------------MAIN--------------------------------------*/
