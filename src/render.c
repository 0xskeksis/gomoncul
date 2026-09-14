#include "gomoku.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 900

#define BOARD_MARGIN  70.0f
#define BOARD_LENGTH  760.0f

#define GRID_STEP \
	(BOARD_LENGTH / (float)(BOARD_SIZE - 1))

#define STONE_RADIUS \
	(GRID_STEP * 0.40f)

typedef struct
{
	Board	board;
	cell	turn;
	bool	game_over;
} Game;

static Game g_game;

static float
board_screen_x(int x)
{
	return (BOARD_MARGIN + (float)x * GRID_STEP);
}

static float
board_screen_y(int y)
{
	return (BOARD_MARGIN + (float)y * GRID_STEP);
}

static void
draw_circle(float cx, float cy, float radius)
{
	int		i;
	float	angle;
	float	x;
	float	y;

	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(cx, cy);

	i = 0;
	while (i <= 32)
	{
		angle = 2.0f * (float)M_PI * (float)i / 32.0f;
		x = cx + cosf(angle) * radius;
		y = cy + sinf(angle) * radius;
		glVertex2f(x, y);
		i++;
	}

	glEnd();
}

static void
draw_board_background(void)
{
	glClearColor(
		0.72f,
		0.55f,
		0.32f,
		1.0f
	);

	glClear(GL_COLOR_BUFFER_BIT);
}

static void
draw_grid(void)
{
	int		i;
	float	x;
	float	y;

	glColor3f(
		0.12f,
		0.08f,
		0.04f
	);

	glLineWidth(1.5f);

	glBegin(GL_LINES);

	i = 0;
	while (i < BOARD_SIZE)
	{
		x = board_screen_x(i);
		glVertex2f(x, BOARD_MARGIN);
		glVertex2f(x, BOARD_MARGIN + BOARD_LENGTH);

		y = board_screen_y(i);
		glVertex2f(BOARD_MARGIN, y);
		glVertex2f(BOARD_MARGIN + BOARD_LENGTH, y);

		i++;
	}

	glEnd();
}

static void
draw_stones(const Board *board)
{
	int		x;
	int		y;
	square	pos;
	cell	c;

	y = 0;
	while (y < BOARD_SIZE)
	{
		x = 0;
		while (x < BOARD_SIZE)
		{
			pos = board_to_index(x, y);
			c = board_cell(board, pos);

			if (c == CELL_BLACK)
			{
				glColor3f(
					0.03f,
					0.03f,
					0.03f
				);

				draw_circle(
					board_screen_x(x),
					board_screen_y(y),
					STONE_RADIUS
				);
			}
			else if (c == CELL_WHITE)
			{
				glColor3f(
					0.95f,
					0.95f,
					0.95f
				);

				draw_circle(
					board_screen_x(x),
					board_screen_y(y),
					STONE_RADIUS
				);

				glColor3f(
					0.15f,
					0.15f,
					0.15f
				);

				glLineWidth(1.0f);

				glBegin(GL_LINE_LOOP);

				{
					int		i;
					float	angle;

					i = 0;
					while (i < 32)
					{
						angle = 2.0f * (float)M_PI
							* (float)i / 32.0f;

						glVertex2f(
							board_screen_x(x)
							+ cosf(angle) * STONE_RADIUS,
							board_screen_y(y)
							+ sinf(angle) * STONE_RADIUS
						);

						i++;
					}
				}

				glEnd();
			}

			x++;
		}

		y++;
	}
}

static void
draw_last_move(const Board *board)
{
	const Move	*move;
	int			x;
	int			y;
	float		size;

	move = board_last_move(board);

	if (!move)
		return ;

	x = board_x(move->square);
	y = board_y(move->square);

	size = STONE_RADIUS * 0.25f;

	glColor3f(
		0.85f,
		0.15f,
		0.15f
	);

	glBegin(GL_QUADS);

	glVertex2f(
		board_screen_x(x) - size,
		board_screen_y(y) - size
	);

	glVertex2f(
		board_screen_x(x) + size,
		board_screen_y(y) - size
	);

	glVertex2f(
		board_screen_x(x) + size,
		board_screen_y(y) + size
	);

	glVertex2f(
		board_screen_x(x) - size,
		board_screen_y(y) + size
	);

	glEnd();
}

static void
render_board(const Board *board)
{
	draw_board_background();
	draw_grid();
	draw_stones(board);
	draw_last_move(board);
}

static bool
mouse_to_board(GLFWwindow *window,
			   double mouse_x,
			   double mouse_y,
			   int *x,
			   int *y)
{
	int		width;
	int		height;
	float	screen_x;
	float	screen_y;
	float	board_x;
	float	board_y;
	int		grid_x;
	int		grid_y;

	glfwGetFramebufferSize(
		window,
		&width,
		&height
	);

	if (width <= 0 || height <= 0)
		return false;

	screen_x = (float)mouse_x * (float)width
		/ (float)WINDOW_WIDTH;

	screen_y = (float)mouse_y * (float)height
		/ (float)WINDOW_HEIGHT;

	board_x = (screen_x - BOARD_MARGIN) / GRID_STEP;
	board_y = (screen_y - BOARD_MARGIN) / GRID_STEP;

	grid_x = (int)roundf(board_x);
	grid_y = (int)roundf(board_y);

	if (!board_in_bounds(grid_x, grid_y))
		return false;

	if (fabsf(
			board_x - (float)grid_x) > 0.35f)
		return false;

	if (fabsf(
			board_y - (float)grid_y) > 0.35f)
		return false;

	*x = grid_x;
	*y = grid_y;

	return true;
}

static void
mouse_button_callback(GLFWwindow *window,
					  int button,
					  int action,
					  int mods)
{
	double	mouse_x;
	double	mouse_y;
	int		x;
	int		y;
	square	pos;
	cell	winner;

	(void)mods;

	if (button != GLFW_MOUSE_BUTTON_LEFT)
		return ;

	if (action != GLFW_PRESS)
		return ;

	if (g_game.game_over)
		return ;

	glfwGetCursorPos(
		window,
		&mouse_x,
		&mouse_y
	);

	if (!mouse_to_board(
			window,
			mouse_x,
			mouse_y,
			&x,
			&y))
		return ;

	pos = board_to_index(x, y);

	if (!board_play(
			&g_game.board,
			pos,
			g_game.turn,
			NULL,
			0))
	{
		printf("Invalid move: %c%d\n",
			'A' + x,
			y + 1);
		return ;
	}

	winner = board_winner(&g_game.board);

	if (winner == CELL_BLACK ||
		winner == CELL_WHITE)
	{
		g_game.game_over = true;

		printf(
			"Winner: %s\n",
			winner == CELL_BLACK
				? "BLACK"
				: "WHITE"
		);

		return ;
	}

	g_game.turn = opponent_of(g_game.turn);
}

static void
key_callback(GLFWwindow *window,
			 int key,
			 int scancode,
			 int action,
			 int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return ;

	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(
			window,
			GLFW_TRUE
		);
		return ;
	}

	if (key == GLFW_KEY_R)
	{
		board_init(&g_game.board);
		g_game.turn = CELL_BLACK;
		g_game.game_over = false;

		printf("Game restarted.\n");
	}
}

static void
setup_projection(GLFWwindow *window)
{
	int	width;
	int	height;

	glfwGetFramebufferSize(
		window,
		&width,
		&height
	);

	glViewport(
		0,
		0,
		width,
		height
	);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(
		0.0,
		(float)WINDOW_WIDTH,
		(float)WINDOW_HEIGHT,
		0.0,
		-1.0,
		1.0
	);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int
main(void)
{
	GLFWwindow	*window;

	if (!glfwInit())
	{
		fprintf(
			stderr,
			"Failed to initialize GLFW\n"
		);
		return EXIT_FAILURE;
	}

	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		"Gomoku",
		NULL,
		NULL
	);

	if (!window)
	{
		fprintf(
			stderr,
			"Failed to create GLFW window\n"
		);
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetMouseButtonCallback(
		window,
		mouse_button_callback
	);

	glfwSetKeyCallback(
		window,
		key_callback
	);

	board_init(&g_game.board);
	g_game.turn = CELL_BLACK;
	g_game.game_over = false;

	while (!glfwWindowShouldClose(window))
	{
		setup_projection(window);

		render_board(&g_game.board);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
