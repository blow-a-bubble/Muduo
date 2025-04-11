all:client6

client1:client1.cpp
	g++ -o $@ $^ -lpthread -std=c++11
client2:client2.cpp
	g++ -o $@ $^ -lpthread -std=c++11
client3:client3.cpp
	g++ -o $@ $^ -lpthread -std=c++11
client4:client4.cpp
	g++ -o $@ $^ -lpthread -std=c++11
client5:client5.cpp
	g++ -o $@ $^ -lpthread -std=c++11
client6:client6.cpp
	g++ -o $@ $^ -lpthread -std=c++11
server:server.cc
	g++ -g -o $@ $^ -lpthread -std=c++11
client:tcp_client.cc
	g++ -o $@ $^ -lpthread -std=c++11



.PHONY:clean
clean:
	rm -rf server client