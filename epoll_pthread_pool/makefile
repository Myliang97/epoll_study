src=$(wildcard *.c)
hf=$(wildcard *.h)
flag=-Wall
lib=-lpthread
all:server client

res=$(filter-out client.c,$(src)) 
server:$(res)
	gcc  $(res) $(flag) $(lib) $(hf) -o $@
crc=client.c common.c
csrc=$(filter $(crc),$(src))
client:$(csrc)
	gcc $(csrc) $(filter common.h,$(hf)) $(flag)  -o $@

.PHONY:clean all
clean:
	rm -rf server client