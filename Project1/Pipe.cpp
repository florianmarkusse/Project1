#include "Pipe.h"
#include <thread>

/*
	Class for managing the creation and usage of pipes for logging purposes.

	Florian Markusse
*/

namespace logger
{
	// Mandatory prefix for all pipes.
	constexpr static auto PIPE_NAME_PREFIX = R"(\\.\pipe\\)";

	/*
		<Constructor>
		Creates a Pipe instance 

		@param handle; The handle to the pipe.
		@param pipeName; The name after the prefix for the pipe.
		@param pipeState; The state of the pipe.

		@return The created Pipe instance
	*/
	Pipe::Pipe(HANDLE handle, std::string pipeName, PipeState pipeState)
		: m_handle { handle }, 
		m_pipeName { std::move(pipeName) },
		m_pipeState { pipeState } { }

	/*
		Returns the correct variable for opening the pipe for this mode.

		@param pipeMode; Which pipe mode the pipe must be opened/created.

		@return The DWORD for opening the pipe for this mode.
	*/
	DWORD Pipe::getPipeMode(PipeMode pipeMode)
	{
		DWORD openMode;

		switch (pipeMode) {
			case PipeMode::read: {
				openMode = PIPE_ACCESS_INBOUND;		// Read-only access.
			} break;
			case PipeMode::write: {
				openMode = PIPE_ACCESS_OUTBOUND;	// Writ-only access.
			} break;
			case PipeMode::readAndWrite: {
				openMode = PIPE_ACCESS_DUPLEX;		// Read-write access.
			} break;
		}

		return openMode;
	}

	/*
		Connects the Pipe instance to the actual pipe.
	*/
	void Pipe::connect()
	{
		if (m_pipeState != PipeState::serverDisconnected) {
			return;
		}

		if (!ConnectNamedPipe(m_handle, nullptr)) {
			CloseHandle(m_handle);
			printf("ConnectNamedPipe failed (%d).\n", GetLastError());
		}

		m_pipeState = PipeState::serverConnected;
	}

	/*
		Disconnects the Pipe instance from the actual pipe.
	*/
	void Pipe::disconnect()
	{
		if (m_pipeState == PipeState::serverConnected) {
			DisconnectNamedPipe(m_handle);
			m_pipeState = PipeState::serverDisconnected;
		}
	}

	/*
		<Move constructor>
		Moves the member variables of the "other" instance to this instance and
		sets the "other" instance to an invalid process.

		@param other; The other Pipe instance.
	*/
	Pipe::Pipe(Pipe && other) noexcept
		: m_handle { other.m_handle }, 
		m_pipeName { other.m_pipeName }, 
		m_pipeState { std::move(other.m_pipeState) }
	{
		// "other" is no longer valid.
		other.m_handle = INVALID_HANDLE_VALUE;
	}

	/*
		<Move assignment operator>
		Closes the pipe, moves the member variables of the "other" instance to 
		this instance and sets the "other" instance to an invalid process.
		Afterwards, returns the current instance.

		@param other; The other Pipe instance.

		@return The current Pipe instance.
	*/
	Pipe & Pipe::operator=(Pipe && other) noexcept
	{
		// Have to close this pipe first before using "other"'s pipe.
		close();

		m_handle = other.m_handle;
		m_pipeName = other.m_pipeName;
		m_pipeState = std::move(other.m_pipeState);

		// "other" is no longer valid.
		other.m_handle = INVALID_HANDLE_VALUE;

		return *this;
	}

	/*
		<Deconstructor>, <virtual>
		Closes the pipe.
	*/
	Pipe::~Pipe()
	{
		close();
	}

	/*
		Creates the pipe and returns a pipe instance.

		@param pipeName; The name after the prefix for the pipe.
		@param pipeMode; Which mode the pipe is to be created in.
		@param readBufferSize; The size of the read buffer in bytes.
		@param writeBufferSize; The size of the write buffer in bytes.

		@return the created Pipe instance.
	*/
	Pipe Pipe::create(
		const std::string & pipeName,
		PipeMode pipeMode,
		ReadBufferSize readBufferSize,
		WriteBufferSize writeBufferSize)
	{
		// Create full pipe name for Windows API.
		const std::string fullPipeName = PIPE_NAME_PREFIX + pipeName;

		const HANDLE pipeHandle = CreateNamedPipeA(
			fullPipeName.c_str(),
			getPipeMode(pipeMode),
			PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
			1,
			writeBufferSize.value,
			readBufferSize.value,
			1,
			nullptr
		);

		if (pipeHandle == INVALID_HANDLE_VALUE) {
			printf("CreateNamedPipeA failed (%d).\n", GetLastError());
		}

		return Pipe { pipeHandle, pipeName, PipeState::serverDisconnected };
	}

	/*
		Opens the pipe in the supplied mode. Returns the correct instance or an
		invalid instance if opening was unsuccessful.

		@param pipeName; The name after the prefix for the pipe.
		@param pipeMode; Which mode the pipe is to be created in.

		@return the created Pipe instance.
	*/
	Pipe Pipe::open(const std::string & pipeName, PipeMode pipeMode)
	{
		// Create full pipe name for Windows API.
		const std::string fullPipeName = PIPE_NAME_PREFIX + pipeName;

		// Give 3 attempts to connect to the pipe.
		for (unsigned short attempts = 0; attempts < 3; ++attempts) {
			const HANDLE pipeHandle = CreateFileA(
				fullPipeName.c_str(),
				getPipeMode(pipeMode),
				0,
				nullptr,
				OPEN_EXISTING,
				0,
				nullptr);

			// Successfully connected to the pipe.
			if (pipeHandle != INVALID_HANDLE_VALUE) {
				return Pipe { pipeHandle, pipeName, PipeState::client };
			}

			if (GetLastError() != ERROR_PIPE_BUSY) {
				printf("Unable to open pipe\n");
			}

			if (!WaitNamedPipeA(pipeName.c_str(), 10000)) {
				printf("Unable to wait on pipe\n");
			}

			// Give the pipe time to free up.
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		printf("Ran out of attempts, returning invalid handle\n");
		return Pipe { INVALID_HANDLE_VALUE, pipeName, PipeState::client };
	}

	/*
		Closes the pipe and sets this instance to invalid.
	*/
	void Pipe::close()
	{
		if (m_handle != INVALID_HANDLE_VALUE) {
			disconnect();
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
	}

	/*
		Returns the name of the pipe without the mandatory prefix.

		@return m_pipeName.
	*/
	const std::string & Pipe::getPipeName() const
	{
		return m_pipeName;
	}

	/*
		Reads bytes from the pipe into the buffer.

		@param buffer; The buffer to which the content of the pipe have to be
		written into.

		@return The amount of bytes read.
	*/
	size_t Pipe::read(Buffer buffer)
	{
		// Can only read data if we are connected to the pipe.
		connect();

		DWORD bytesRead;
		if (!ReadFile(
			m_handle,
			buffer.data,
			buffer.size.value,
			&bytesRead,
			nullptr)) {
				printf("ReadFile failed, could not read pipe\n");
		}

		return bytesRead;
	}

	/*
		Writes bytes from the buffer into the pipe.

		@param buffer; The buffer from which the content will be written to the
		pipe.
	*/
	void Pipe::write(const Buffer buffer)
	{
		// Can only write data if we are connected to the pipe.
		connect();

		DWORD bytesWritten;
		if (!WriteFile(
			m_handle,
			buffer.data,
			buffer.size.value,
			&bytesWritten,
			nullptr) || bytesWritten != buffer.size.value) {
				printf("WriteFile failed, could not write to pipe\n");
		}
	}
}

