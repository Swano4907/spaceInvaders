spaceInvaders: src/main.o src/init.o src/render.o src/entity.o
	cc src/main.o src/render.o src/init.o src/entity.o -o spaceInvaders -lncurses -lpthread

main.o: src/bullet.h src/entity.h src/entDefs.h src/init.h src/render.h
	cc -c src/main.c -o src/main.o

init.o: src/init.c src/init.h src/entDefs.h
	cc -c src/init.c -o src/init.o

entity.o: src/entity.c src/entity.h src/entDefs.h src/init.h
	cc -c src/entity.c -o src/entity.h

render.o: src/render.c src/render.h src/entDefs.h
	cc -c src/render.c -o src/render.o

clean:
	rm src/*.o
	rm spaceInvaders
	rm log*
	clear
