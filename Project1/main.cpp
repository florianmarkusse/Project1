#include <windows.h>
#include <iostream>
#include <array>
#include <thread>

#include "Pipe.h"

#define READ_BUFFER_SIZE 4096
#define PRECEDING_INFORMATION_SIZE 1
#define LOG_LEVEL_LOCATION 0

/*
	Checks if the character is a preceding command or an actual part of the
	message.

	@param chracter; The character to be checked.

	@return If the character is a preceding command.
*/
bool checkIfPreceding(const char character)
{
	return ( character == static_cast<char> ( logger::LogLevel::info )
		|| character == static_cast<char> ( logger::LogLevel::warn )
		|| character == static_cast<char> ( logger::LogLevel::error )
		|| character == static_cast<char> ( logger::LogLevel::success ) );
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr 
			<< "not enough arguments\n" 
			<< "call syntax: console_helper [title] [pipe name]" 
			<< std::endl;
		return -1;
	}

	SetConsoleTitleA(argv[1]);

	// Need to open the pipe to receive the logging messages.
	logger::Pipe pipe = logger::Pipe::open(argv[2], logger::PipeMode::read);

	// Need to hold the bytes read from the pipe.
	std::array<char, READ_BUFFER_SIZE> buffer = 
		std::array<char, READ_BUFFER_SIZE>{};

	// Allow 3 failures before stopping with reading from the pipe.
	unsigned short failures = 0;

	// Handle to the window to change the color of the logging message(s).
	HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	while (true) {

		const auto bytesRead = pipe.read(logger::Buffer {
				buffer.data(),
				buffer.size() 
			});

		for (unsigned int i = 0; i < bytesRead; ++i) {
			if (!checkIfPreceding(buffer[i])) {
				std::cout << buffer[i];
			}
			else {
				SetConsoleTextAttribute(stdOut, buffer[i]);
			}
		}

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