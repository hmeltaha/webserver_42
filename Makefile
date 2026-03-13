NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

SRCS = main.cpp \
       src/parser/ConfigParser.cpp \
       src/parser/ConfigParserUtils.cpp \
       src/parser/ConfigParserValidation.cpp \
       src/parser/ServerConfig.cpp \
       src/parser/LocationConfig.cpp \
       src/handlers/FileHandler.cpp \
       src/handlers/DirectoryLister.cpp \
       src/handlers/UploadHandler.cpp \
       src/handlers/DeleteHandler.cpp \
       src/router/Router.cpp \
       src/utils/MethodValidator.cpp \
       src/server/server.cpp \
       src/server/Client.cpp \
       src/server/MainLoop.cpp \
       src/server/SignalHandler.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
