There are 2 implementaions in this directory
	1) Non-pipelined
	2) Pipelined

There is also a helper gen.py file that generates CICERO machine code when given instructions in english

Type english code at the beginning of input.txt (There are 3 examples in the file)

run : cat input.txt | python3 gen.py
The CICERO code is displayed in the terminal
paste this code in the instruction memory of pipelined.cpp or non-pipelined.cpp

And then simply compile and run the binary file
	g++ pipelined.cpp
	./a.out

It will ask for a string as input. And it will output wether the string is accepted or not.

