#pragma once
#include "IInterProcess.h"

#include <windows.h>
#include <string>

/*
	Class for managing the creation and usage of pipes for logging purposes.

	Florian Markusse
*/
namespace logger
{

	enum class PipeState
	{
		client,
		serverConnected,
		serverDisconnected
	};

	enum class PipeMode
	{
		read,
		write,
		readAndWrite
	};

	enum class LogLevel : char
	{
		// White color font.
		info = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		// Yellow/amber color font.
		warn = FOREGROUND_GREEN | FOREGROUND_RED,
		// Red color font.
		error = FOREGROUND_RED,
		// Green color font.
		success = FOREGROUND_GREEN
	};

	class Pipe : public IInterProcess
	{

	private:
		HANDLE m_handle;
		std::string m_pipeName;
		PipeState m_pipeState;

		Pipe(HANDLE handle, std::string pipeName, PipeState pipeState);

		static DWORD getPipeMode(PipeMode pipeMode);

		void connect();
		void disconnect();

	public:
		Pipe(const Pipe &other) = delete;
		Pipe(Pipe &&other) noexcept;

		Pipe &operator=(const Pipe &other) = delete;
		Pipe &operator=(Pipe && other) noexcept;

		virtual ~Pipe();

		static Pipe create(
			const std::string &pipeName,
			PipeMode pipeMode,
			ReadBufferSize readBufferSize,
			WriteBufferSize writeBufferSize);

		static Pipe open(const std::string &pipeName, PipeMode pipeMode);

		void close();
		const std::string &getPipeName() const;
		size_t read(Buffer buffer) override;
		void write(Buffer buffer) override;
	};

} // namespace logger


