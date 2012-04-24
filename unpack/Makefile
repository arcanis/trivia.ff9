NAME     = ../bin/unpack

SRCS     = $(shell find . -name '*.cpp' -print)
OBJS     = $(SRCS:.cpp=.o)

CXX      = g++
RM       = rm -f

CXXFLAGS = -Wall -Wextra -Werror -I../common
LFLAGS   = 

$(NAME): $(OBJS)
	$(CXX) -o $(@) $(<) $(LFLAGS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all
