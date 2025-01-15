CCX = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -Werror -O3
LDFLAGS =
OBJDIR = build
SRC = src

SRCS = $(wildcard $(SRC)/*.cc)
OBJS = $(SRCS:$(SRC)/%.cc=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: server client

server: $(OBJDIR)/command.o $(OBJDIR)/epoll_manager.o $(OBJDIR)/server.o
	$(CCX) $(CXXFLAGS) -o server $^ $(LDFLAGS)

client: $(OBJDIR)/client.o
	$(CCX) $(CXXFLAGS) -o client $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRC)/%.cc | $(OBJDIR)
	$(CCX) $(CXXFLAGS) -c $< -o $@ -MMD -MP

$(OBJDIR):
	mkdir -p $(OBJDIR)

debug:
	$(CCX) $(CXXFLAGS) -D_DEBUG -g -o server src/command.cc src/epoll_manager.cc src/server.cc
	$(CCX) $(CXXFLAGS) -D_DEBUG -g -o client src/client.cc

clean:
	$(RM) -r $(OBJDIR) server client

-include $(DEPS)
