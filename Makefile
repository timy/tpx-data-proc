DIR_SRC     =   ./src

all:
	cd $(DIR_SRC) && make

install:
	if [ ! -d "./bin" ] ; then \
		mkdir ./bin; \
	fi
	mv $(DIR_SRC)/main ./bin/main

clean:
	cd $(DIR_SRC) && make clean

cleanall: clean
	if [ -d "./bin" ] ; then \
		rm -r ./bin; \
	fi
