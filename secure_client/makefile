secure_client.exe		: 	secure_client.o CryptoSystem.o
	g++ -o secure_client.exe secure_client.o CryptoSystem.o -lws2_32 -L"C:/boost_1_82_0/libs" -static-libgcc -static-libstdc++
			
secure_client.o		:	secure_client.cpp
	g++ -c -Wall -O2 secure_client.cpp -I"C:/boost_1_82_0"

CryptoSystem.o		:	../CryptoSystem.cpp
	g++ -c -Wall -O2 ../CryptoSystem.cpp -I"C:/boost_1_82_0"

clean:
	del *.o
	del *.exe