NAME	?= gomoku

CC	:=	clang

CFLAGS	:= -Wall -Wextra -Werror -g

SRCS_DIR	:= src

SRCS	?=  main.c \

SRCS	:= $(addprefix $(SRCS_DIR),/$(SRCS))

BUILD	:= .objs

OBJS	:= $(addprefix $(BUILD)/, $(SRCS:%.c=%.o))

MAKEFLAGS	+= --no-print-directory

RM			:= rm -rf

DIR_UP		= mkdir -p $(@D)

################################################################################

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $@
	@printf " $(GREEN)$(BOLD)$(ITALIC)■$(RESET)  building	$(GREEN)$(BOLD)$(ITALIC)$(NAME)$(RESET)\n"

$(BUILD)/%.o: %.c
	@$(DIR_UP)
	@printf " $(CYAN)$(BOLD)$(ITALIC)■$(RESET)  compiling	$(GRAY)$(BOLD)$(ITALIC)$^$(RESET)\n"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@$(RM) $(BUILD)
	@printf " $(RED)$(BOLD)$(ITALIC)■$(RESET)  removing	$(RED)$(BOLD)$(ITALIC)$(BUILD)$(RESET)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf " $(RED)$(BOLD)$(ITALIC)■$(RESET)  removing	$(RED)$(BOLD)$(ITALIC)$(NAME)$(RESET)\n"

re: fclean all

################################################################################


BLACK		=	\033[30m
RED			=	\033[31m
GREEN		=	\033[32m
YELLOW		=	\033[33m
BLUE		=	\033[34m
MAGENTA		=	\033[35m
CYAN		=	\033[36m
WHITE		=	\033[37m
GRAY		=	\033[90m

BOLD		=	\033[1m
ITALIC		=	\033[3m

RESET		=	\033[0m
LINE_CLR	=	\33[2K\r
