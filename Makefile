NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

MAIN_SRC := $(wildcard main.cpp)

SRCS = src/parser/ConfigParser.cpp \
       src/parser/ConfigParserUtils.cpp \
       src/parser/ConfigParserValidation.cpp \
       src/parser/ServerConfig.cpp \
       src/parser/LocationConfig.cpp

ifneq ($(MAIN_SRC),)
SRCS += $(MAIN_SRC)
BUILD_TARGET := $(NAME)
else
BUILD_TARGET := modules
endif

OBJS = $(SRCS:.cpp=.o)

all: $(BUILD_TARGET)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

modules: $(OBJS)
	@echo "Built objects only (no main.cpp found)."

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re modules
