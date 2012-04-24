NAME     = ../bin/unpack

SRCS     = $(shell find . -name '*.cpp' -print) $(shell find ../common -name '*.cpp' -print)
OBJS     = $(SRCS:.cpp=.o)

CXX      = g++
RM       = rm -f

CXXFLAGS = -Wall -Wextra -Werror -I../common
LFLAGS   = -lboost_filesystem -lboost_system

$(NAME): $(OBJS)
	$(CXX) -o $(@) $(OBJS) $(LFLAGS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all
