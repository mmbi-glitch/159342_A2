secure_server.exe		: 	secure_server.o CryptoSystem.o
	g++ -o secure_server.exe secure_server.o CryptoSystem.o -lws2_32 -L"C:/boost_1_82_0/libs" -static-libgcc -static-libstdc++
			
secure_server.o		:	secure_server.cpp
	g++ -c -Wall -O2 secure_server.cpp -I"C:/boost_1_82_0"

CryptoSystem.o		:	../CryptoSystem.cpp
	g++ -c -O2 -Wall ../CryptoSystem.cpp -I"C:/boost_1_82_0"

clean:
	del *.o
	del *.exe
