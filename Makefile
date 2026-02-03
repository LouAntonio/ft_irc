NAME = ircserv

SRC = srcs/main.cpp\
	  srcs/Channel.cpp\
	  srcs/Client.cpp\
	  srcs/Server.cpp\
	  srcs/commands/commands.cpp\
	  srcs/commands/nick.cpp\
	  srcs/commands/join.cpp\
	  srcs/commands/user.cpp\
	  srcs/commands/pass.cpp\
	  srcs/commands/invite.cpp\
	  srcs/commands/privmsg.cpp\
	  srcs/commands/kick.cpp\
	  srcs/commands/topic.cpp\
	  srcs/commands/mode.cpp\

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

HEAD = includes/Channel.hpp\
		includes/Client.hpp\
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

run:
	./$(NAME) 3000 1234

.PHONY: all clean fclean re run
