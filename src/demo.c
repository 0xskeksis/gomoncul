#include "gomoku.h"
#include <stdio.h>
#include <string.h>

# define INPUT_SIZE 32
# define COL_LETTERS "ABCDEFGHIJKLMNOPQRS"

# define COLOR_RESET	"\033[0m"
# define COLOR_GRID		"\033[38;5;244m"
# define COLOR_LABEL	"\033[1;38;5;250m"
# define COLOR_BLACK	"\033[1;38;5;39m"
# define COLOR_WHITE	"\033[1;38;5;208m"
# define COLOR_INFO		"\033[38;5;250m"

# define GLYPH_EMPTY	COLOR_GRID "." COLOR_RESET
# define GLYPH_BLACK	COLOR_BLACK "X" COLOR_RESET
# define GLYPH_WHITE	COLOR_WHITE "O" COLOR_RESET

# define BOX_TOP_LEFT		COLOR_GRID "."
# define BOX_TOP_RIGHT		"." COLOR_RESET
# define BOX_HORIZONTAL		"-"
# define BOX_VERTICAL		COLOR_GRID ":" COLOR_RESET

static const char
*glyph_of(cell _cell)
{
	if (_cell == CELL_BLACK)
		return (GLYPH_BLACK);
	if (_cell == CELL_WHITE)
		return (GLYPH_WHITE);
	return (GLYPH_EMPTY);
}

static void
print_frame(void)
{
	int	x;

	printf("   " BOX_TOP_LEFT);
	x = 0;
	while (x < BOARD_SIZE * 2 + 1)
	{
		printf(BOX_HORIZONTAL);
		x++;
	}
	printf(BOX_TOP_RIGHT "\n");
}

static void
print_header(void)
{
	int	x;

	printf("     ");
	x = 0;
	while (x < BOARD_SIZE)
	{
		printf(COLOR_LABEL "%c " COLOR_RESET, COL_LETTERS[x]);
		x++;
	}
	printf("\n");
}

static void
print_rows(const Board *board)
{
	int	x;
	int	y;

	y = 0;
	while (y < BOARD_SIZE)
	{
		printf(COLOR_LABEL "%2d " COLOR_RESET BOX_VERTICAL " ", y + 1);
		x = 0;
		while (x < BOARD_SIZE)
		{
			printf("%s ", glyph_of(board_cell(board, board_to_index(x, y))));
			x++;
		}
		printf(BOX_VERTICAL COLOR_LABEL " %-2d" COLOR_RESET "\n", y + 1);
		y++;
	}
}

static void
print_board(const Board *board, cell turn)
{
	printf("\n");
	print_header();
	print_frame();
	print_rows(board);
	print_frame();
	print_header();
	printf("\n" COLOR_INFO "  turn " COLOR_RESET "%s"
		COLOR_INFO "    captured " COLOR_RESET "%s" COLOR_INFO " %u  "
		COLOR_RESET "%s" COLOR_INFO " %u    moves %d\n" COLOR_RESET,
		glyph_of(turn),
		GLYPH_BLACK, board->captured_stones[CELL_BLACK],
		GLYPH_WHITE, board->captured_stones[CELL_WHITE],
		board->move_count);
}

static bool
parse_coordinates(const char *line, int *x, int *y)
{
	const char	*found;
	int			row;

	while (*line == ' ')
		line++;
	found = strchr(COL_LETTERS, *line & ~('a' - 'A'));
	if (!found || !*found)
		return (false);
	if (sscanf(line + 1, "%d", &row) != 1)
		return (false);
	*x = (int)(found - COL_LETTERS);
	*y = row - 1;
	return (board_in_bounds(*x, *y));
}

static void
apply_input(Board *board, const char *line, cell *turn)
{
	int	x;
	int	y;

	if (line[0] == 'u' || line[0] == 'U')
	{
		if (board_undo(board))
		{
			printf(COLOR_INFO "  undo successful\n" COLOR_RESET);
			*turn = opponent_of(*turn);
		}
		else
			printf(COLOR_INFO "  nothing to undo\n" COLOR_RESET);
		return ;
	}
	if (!parse_coordinates(line, &x, &y))
	{
		printf(COLOR_INFO "  invalid input\n" COLOR_RESET);
		return ;
	}
	if (!board_play(board, board_to_index(x, y), *turn, NULL, 0))
	{
		printf(COLOR_INFO "  invalid move\n" COLOR_RESET);
		return ;
	}
	*turn = opponent_of(*turn);
}

void
demo_start(void)
{
	Board	board;
	cell	turn;
	char	input[INPUT_SIZE];

	board_init(&board);
	turn = CELL_BLACK;
	while (true)
	{
		print_board(&board, turn);
		printf(COLOR_INFO "  move (e.g. J10), 'u' to undo, 'q' to quit > "
			COLOR_RESET);
		if (!fgets(input, sizeof(input), stdin))
			break ;
		if (input[0] == 'q' || input[0] == 'Q')
			break ;
		if (input[0] != '\n')
			apply_input(&board, input, &turn);
	}
	printf("\n");
}