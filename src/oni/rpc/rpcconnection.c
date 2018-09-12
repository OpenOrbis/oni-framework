#include <oni/rpc/rpcconnection.h>
#include <oni/config.h>

#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>

#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/escape.h>
#include <oni/utils/ref.h>

#include <oni/framework.h>

void rpcconnection_init(struct rpcconnection_t* connection)
/*
Initializes a new rpcconnection_t object
*/
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// Verify that our connection object is fine
	if (!connection)
		return;

	// Set the socket to invalid socket value
	connection->socket = -1;

	// Zero out thread information
	connection->thread = 0;
	connection->isRunning = 0;

	// Zero out the buffer and the buffer size
	memset(connection->buffer, 0, sizeof(connection->buffer));

	// Zero out the address buffer
	connection->address.sin_family = 0;
	connection->address.sin_port = 0;
	connection->address.sin_family = 0;
	connection->address.sin_len = sizeof(connection->address);
}

void rpcconnection_clientThread(void* data)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	WriteLog(LL_Debug, "rpcconnection_serverThread entered");

	// Verify that our data pointer is valid
	if (!data)
	{
		kthread_exit();
		return;
	}
	struct rpcconnection_t* connection = (struct rpcconnection_t*)data;

	// Jailbreak this new thread
	oni_threadEscape(curthread, NULL);

	// Initialize all of the buffers
	WriteLog(LL_Debug, "rpcconnection_serverThread init buffers");
	memset(connection->buffer, 0, sizeof(connection->buffer));

	// Set the running status
	connection->isRunning = true;

	const uint32_t messageHeaderSize = sizeof(struct message_header_t);
	uint32_t dataReceived = 0;

	// While we have the server running
	while (connection->isRunning)
	{
		//WriteLog(LL_Debug, "rpcconnection_serverThread zeroing buffer");
		// Zero out the buffer
		memset(connection->buffer, 0, sizeof(connection->buffer));

		// If the connection socket has an error state
		if (connection->socket < 0)
		{
			WriteLog(LL_Error, "socket error: %d", connection->socket);
			goto cleanup;
		}

		//WriteLog(LL_Debug, "rpcconnection_serverThread begin recv");
		dataReceived = 0;

		// Try to get the size of a full message
		int32_t recvSize = krecv(connection->socket, (char*)connection->buffer, messageHeaderSize, 0);
		if (recvSize <= 0)
		{
			WriteLog(LL_Error, "%d recv returned %d.", connection->socket, recvSize);
			goto cleanup;
		}

		// Increment the dataReceived
		dataReceived += recvSize;

		// Otherwise, try to get the entire full message
		while (dataReceived < messageHeaderSize)
		{
			//WriteLog(LL_Debug, "rpcconnection_serverThread recv more %d", dataReceived);
			uint32_t wantRecvSize = messageHeaderSize - dataReceived;

			recvSize = krecv(connection->socket, &connection->buffer[dataReceived], wantRecvSize, 0);
			if (recvSize <= 0)
			{
				WriteLog(LL_Error, "recv err returned %d.", recvSize);
				goto cleanup;
			}

			dataReceived += recvSize;
		}

		// Check the message header magic
		struct message_header_t* header = (struct message_header_t*)connection->buffer;
		if (header->magic != RPCMESSAGE_HEADER_MAGIC)
		{
			WriteLog(LL_Error, "invalid header got 0x%02x expected 0x%02x", header->magic, RPCMESSAGE_HEADER_MAGIC);
			goto cleanup;
		}
			
		// Check to see how much data we actually got
		uint64_t totalDataSize = recvSize + header->payloadSize;

		//WriteLog(LL_Debug, "checking payload length\n");
		// If the payload length is bigger than the maximum buffer size, then fail
		if (totalDataSize > ARRAYSIZE(connection->buffer))
		{
			WriteLog(LL_Error, "payload length greater than buffer size.");
			goto cleanup;
		}

		// Recv the payload
		while (dataReceived < totalDataSize)
		{
			uint64_t dataSizeRemaining = totalDataSize - dataReceived;
			recvSize = krecv(connection->socket, (char*)(connection->buffer) + dataReceived, dataSizeRemaining & 0xFFFFFFFF, 0);
			if (recvSize <= 0)
				goto cleanup;

			dataReceived += recvSize;
		}

		// Create a new "local" message
		struct ref_t* allocation = ref_alloc(totalDataSize);
		if (!allocation)
		{
			WriteLog(LL_Error, "could not allocate memory for message");
			continue;
		}

		struct message_t* internalMessage = ref_getData(allocation);
		if (!internalMessage)
			goto do_dec;

		internalMessage->payload = NULL;

		// Allow us to send header-only messages
		if (header->payloadSize > 0)
		{
			// TODO: Implement maximum size check
			// But also if there is an additional fd, then just write to the fd or error
			//WriteLog(LL_Debug, "allocating payload length %d", header->payloadSize);
			internalMessage->payload = kmalloc(header->payloadSize);
			if (!internalMessage->payload)
			{
				WriteLog(LL_Error, "error allocating payload");
				goto do_dec;
			}

			// Zero out and copy our buffer
			memset(internalMessage->payload, 0, header->payloadSize);

			const void* messageAddress = internalMessage++;
			memcpy(internalMessage->payload, messageAddress, header->payloadSize);

		}
		
		WriteLog(LL_Debug, "dispatching message %p %p", gFramework->messageManager, allocation);

		// Now that we have the full message chunk, parse the category and the type and get it the fuck outa here
		messagemanager_sendRequest(allocation);

	do_dec:
		ref_release(allocation);
	}

	
cleanup:
	connection->isRunning = false;
	if (connection->socket >= 0)
	{
		kshutdown(connection->socket, 2);
		kclose(connection->socket);
		connection->socket = -1;
	}

	if (connection->disconnect != NULL &&
		connection->server != NULL)
		connection->disconnect(connection->server, connection);
	
	kthread_exit();
}