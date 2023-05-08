secure_server.exe		: 	secure_server.o
	g++ secure_server.cpp -lws2_32 -o secure_server.exe -L"C:/boost_1_82_0/libs" -static-libgcc -static-libstdc++
			
secure_server.o		:	secure_server.cpp 
	g++ -c -Wall -O2 -fconserve-space secure_server.cpp -I"C:/boost_1_82_0"

clean:
	del *.o
	del *.exe
