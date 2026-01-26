NAME = ircserv

SRC = srcs/main.cpp\
	  srcs/Channel.cpp\
	  srcs/Client.cpp\
	  srcs/Command.cpp\
	  srcs/Server.cpp\
	  srcs/commands/commands.cpp

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

HEAD = includes/Channel.hpp\
		includes/Client.hpp\
		includes/Command.hpp\
		includes/Server.hpp\

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ): $(HEAD)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
