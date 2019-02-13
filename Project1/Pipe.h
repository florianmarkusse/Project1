#pragma once
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

	struct BufferSize
	{
		const size_t value;

		explicit BufferSize(const size_t value)
			: value { value }
		{
		}
	};

	struct ReadBufferSize : public BufferSize
	{
		explicit ReadBufferSize(const size_t value)
			: BufferSize { value }
		{
		}
	};

	struct WriteBufferSize : BufferSize
	{
		explicit WriteBufferSize(const size_t value)
			: BufferSize { value }
		{
		}
	};

	struct Buffer
	{
		char *data;
		BufferSize size;

		Buffer(char *data, size_t size)
			: data { data }, size { size } {}
	};

	class Pipe
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
		size_t read(Buffer buffer);
		void write(const Buffer buffer);
	};

}


