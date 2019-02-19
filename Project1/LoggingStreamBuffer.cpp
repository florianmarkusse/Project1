#include "LoggingStreamBuffer.h"

/*
	Class for managing the content written to a Console instance.

	Florian Markusse
*/
namespace logger
{

	/*
		<override>
		Handles overflow to the stream buffer to ensure that there is space at
		the put area.

		@param character; The character that is overflowing or EoF.

		@return The character that is handled correctly or EoF when not
		successful.
	*/
	std::basic_streambuf<char>::int_type
		LoggingStreamBuffer::overflow(int_type character)
	{
		// Put area is full, thus publish!
		publish();

		// Check if the put area is still valid.
		if (character != traits_type::eof()) {
			*pptr() = character;
			pbump(1);
			return character;
		}

		return traits_type::eof();
	}

	/*
		<override>
		Synchronizes the put area buffer with the character sequence by
		publishing.

		@return 0.
	*/
	std::basic_streambuf<char>::int_type LoggingStreamBuffer::sync()
	{
		publish();
		return 0;
	}

	/*
		Write the contents of the put area to the pipe and resets the put area
		pointers.
	*/
	void LoggingStreamBuffer::publish()
	{
		// Need to know how much to write to the pipe.
		const int size = pptr() - pbase();

		if (size > 0) {
			m_output.write(
				Buffer {
					m_buffer.data(),
					static_cast<size_t> ( size )
				});
			// Define the put area again after writing.
			setp(m_buffer.data(), m_buffer.data() + m_buffer.size());
		}
	}

	/*
		<Constructor>
		Write the contents of the put area to the pipe and resets the put area
		pointers.
	*/
	LoggingStreamBuffer::LoggingStreamBuffer(
		IInterProcess & output,
		BufferSize bufferSize)
		: m_output { output }, m_buffer(bufferSize.value)
	{
		char * const begin = m_buffer.data();
		char * const end = begin + m_buffer.size();
		// Define the put area.
		setp(begin, end);
	}

} // namespace logger
