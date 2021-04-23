#include <iostream>
#include <vector>
#include <map>
#include <deque>

const size_t memSize = 30000;
unsigned char memory[memSize];
int currPos = 0;

int main() {
	std::string in;
	std::string pre = ">>> ";

	int RE = 0;
	int x;
	std::cout << pre ;
	while (x = getchar()) {
		if (x == EOF) break;
		if (x != '\n') {
			in += x;
			continue;
		}
		RE = 0;
		int bal = 0;
		size_t sz = in.size();
		for (size_t i = 0; i < sz; ++i) {
			if (in[i] == '[') ++bal;
			if (in[i] == ']') --bal;
			if (bal < 0) break;
		}
		if (bal > 0) {
			std::cout << "Syntax error, too much '['\n";
		} else if (bal < 0) {
			std::cout << "Syntax error, too much ']'\n";
		}
		for (int i = 0; i < sz; ++i) {
			switch (in[i]) {
				case '>':
					++currPos;
					break;
				case '<':
					--currPos;
					break;
				case '+':
					if (currPos < 0 || currPos >= memSize) {
						RE = 1;
					} else {
						++memory[currPos];
					}
					break;
				case '-':
					if (currPos < 0 || currPos >= memSize) {
						RE = 1;
					} else {
						--memory[currPos];
					}
					break;
				case ',':
					if (currPos < 0 || currPos >= memSize) {
						RE = 1;
					} else {
						memory[currPos] = getchar();
						getchar();
					}
					break;
				case '.':
					if (currPos < 0 || currPos >= memSize) {
						RE = 1;
					} else {
						std::cout << memory[currPos];
					}
					break;
				case '[':
					if (memory[currPos] == 0) {
						bal = 1;
						while (bal > 0) {
							++i;
							if (in[i] == '[') ++bal;
							else if (in[i] == ']') --bal;
						}
					}
					break;
				case ']':
					bal = -1;
					--i;
					while (bal < 0) {
						if (in[i] == '[') ++bal;
						else if (in[i] == ']') --bal;
						--i;
					}
					break;
				case 'n':
					std::cout << '\n';
					break;
			}
			if (RE) {
				std::cout << "Runtime error, error code: " << RE << '\n';
				break;
			}
		}
		std::cout << pre;
		in.clear();
	}
}