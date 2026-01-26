NAME = ircserv

SRC = srcs/main.cpp\
	  srcs/Server.cpp\
	  srcs/Client.cpp\
	  srcs/Channel.cpp\
	  srcs/commands/commands.cpp\
	  srcs/commands/nick.cpp

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

HEAD = includes/Client.hpp\
		includes/Server.hpp\
		includes/Channel.hpp\

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