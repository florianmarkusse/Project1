#include <windows.h>
#include <iostream>
#include <array>
#include <thread>

#include "Pipe.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "not enough arguments\n" << "call syntax: console_helper [title] [pipe name]" << std::endl;
		return -1;
	}

	SetConsoleTitleA(argv[1]);
	auto pipe = logger::Pipe::open(argv[2], logger::PipeMode::read);

	auto buffer = std::array<char, 1024>{};

	unsigned short failures = 0;

	while (true) {
		const auto bytesRead = pipe.read(logger::Buffer { buffer.data(), buffer.size() });
		std::cout.write(buffer.data(), bytesRead);

		if (bytesRead == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			++failures;
		}

		if (failures == 3) {
			break;
		}

	}

	std::cout << "Finished logging, press any key to continue...\n";
	std::cin.get();

	return 0;
}