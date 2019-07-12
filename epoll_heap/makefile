src=$(wildcard *.c)
targets=$(patsubst %.c,%,$(src))
tip=-Wall
all:$(targets)
$(targets):%:%.c
	gcc $< -o $@ $(Wall)

.PHNOY:clean all
clean:
	rm  -rf $(targets)

