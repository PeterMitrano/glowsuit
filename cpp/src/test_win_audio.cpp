#include <Windows.h>
#include <Winnt.h>
#include <iostream>

int main() {
	auto const result = PlaySound(TEXT("C:/users/peter/Documents/glowsuit/test.wav"), NULL, SND_FILENAME);
	std::cout << "done: " << result << '\n';
	std::cin.get();
	return 0;
}