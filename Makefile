AS := clang
CC := clang
LD := ld

C_SRCS := $(wildcard src/*.c)

OBJS += $(patsubst %.c,%.o,$(C_SRCS))

INCLUDE := -Iinclude

BASEFLAGS := -g
WARNFLAGS := -Weverything -Werror -Wno-float-equal -Wno-unused-parameter -Wno-missing-prototypes -Wno-unused-macros -Wno-padded -Wno-switch-enum -Wno-deprecated-declarations
CFLAGS := -std=c99 $(DEFINES) $(BASEFLAGS) $(WARNFLAGS) $(INCLUDE)
LDFLAGS := -demangle -dynamic -arch x86_64 -macosx_version_min 10.10.0  

LIBS := -lsdl2 -framework OpenGL -lSystem /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/../lib/clang/6.1.0/lib/darwin/libclang_rt.osx.a

ogl: $(OBJS)
	$(LD) $(LDFLAGS) $(LIBS) $(OBJS) -o $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -f $(OBJS) ogl

