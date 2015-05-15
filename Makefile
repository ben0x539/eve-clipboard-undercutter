CFLAGS = -std=c89 -Werror-implicit-function-declaration -Weverything -Wno-unused-parameter
GTKCRUD = `pkg-config --cflags --libs gtk+-3.0 | sed -e 's/-I/-isystem /g'`

all: clipboard_undercutter

clipboard_undercutter: clipboard_undercutter.c
	clang $(CFLAGS) $(GTKCRUD) -o clipboard_undercutter clipboard_undercutter.c
