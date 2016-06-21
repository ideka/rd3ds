TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source ../common/source
INCLUDES	:=	source ../common/source

PREFIX		:=	x86_64-w64-mingw32-
#PREFIX		:=	i686-w64-mingw32-

CC	:=	$(PREFIX)gcc
LD	:=	$(CC)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			-I$(CURDIR)/$(BUILD)

CFLAGS	:=	-g -Wall -Wextra -O3 -std=c99
CFLAGS	+=	$(INCLUDE)
LDFLAGS	:=	-static-libgcc -lws2_32 -ld3d9 -ld3dx9 \
                -static -lz

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

export OFILES	:=	$(addprefix $(BUILD)/,$(CFILES:.c=.o))

.PHONY: clean

$(OUTPUT).exe: $(OFILES)
	@echo linking $(notdir $@)
	@$(LD) $(OFILES) -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.c
	@echo $(notdir $<)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo clean...
	@rm -fr $(BUILD) $(TARGET).exe
