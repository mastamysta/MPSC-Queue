


TARGETS=creator consumer

all: $(TARGETS)

creator:
	g++ creator.cpp -o creator

consumer:
	g++ consumer.cpp -o consumer

clean:
	rm -rf *.o $(TARGETS)