#include <oni/utils/ref.h>
#include <oni/messaging/message.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>

// Local messages will have socket of -1
void message_init(struct message_t* message, int32_t socket)
{
	if (!message)
		return;

	// Initialize this message header
	message->header.magic = RPCMESSAGE_HEADER_MAGIC;
	message->header.category = RPCCAT_NONE;
	message->header.error_type = 0;
	message->header.payloadSize = 0;

	message->socket = socket;

	// Initialize the payload pointer
	message->payload = NULL;
}

//struct ref_t* message_initParse(struct message_header_t* header, int32_t socket)
//{
//	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
//
//	if (!header)
//		return NULL;
//
//	// Allocate space for the new message
//	struct allocation_t* allocation = __malloc(sizeof(struct message_t));
//	if (!allocation)
//		return NULL;
//
//	struct message_t* message = allocation->data;
//	if (!message)
//	{
//		__free(allocation);
//		return NULL;
//	}
//
//	// Copy the header over
//	memcpy(&message->header, header, sizeof(message->header));
//
//	// Null out the payload pointer
//	message->payload = NULL;
//
//	// Set the socket pointer
//	message->socket = socket;
//
//	return allocation;
//}