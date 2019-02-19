#pragma once
#include "IInterProcess.h"

#include <string>
#include <vector>

namespace logger
{

	class LoggingStreamBuffer : public std::basic_streambuf<char>
	{

	private:
		IInterProcess &m_output;
		std::vector<char> m_buffer;

	protected:
		int_type overflow(int_type character = traits_type::eof()) override;
		int_type sync() override;

		virtual void publish();

	public:
		explicit LoggingStreamBuffer(IInterProcess &output, BufferSize bufferSize);
	};

} // namespace logger

