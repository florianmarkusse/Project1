#pragma once

/*
	Interface that every platform (Windows, Linux and Apple) has to inherit
	that provides the interprocess communication and structures to use for
	communication purposes.

	Florian Markusse
*/
namespace logger
{
	// Need to know the size of the number of bytes that we have to write to the
	// pipe.
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

	class IInterProcess
	{

	public:

		virtual size_t read(Buffer buffer) = 0;
		virtual void write(Buffer buffer) = 0;
	};

}
