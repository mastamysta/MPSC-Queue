


TARGETS=creator consumer

all: $(TARGETS)

creator:
	g++ creator.cpp -o creator -g

consumer:
	g++ consumer.cpp -o consumer -g

clean:
	rm -rf *.o $(TARGETS)