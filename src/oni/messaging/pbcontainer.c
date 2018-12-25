#include <oni/messaging/pbcontainer.h>
#include <protobuf-c/mirabuiltin.pb-c.h>

#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

PbContainer* pbcontainer_create(PbMessage* message)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!message)
		return NULL;

	PbContainer* container = k_malloc(sizeof(PbContainer));
	if (!container)
		return NULL;

	// Zero out the new container
	memset(container, 0, sizeof(*container));

	container->message = message;
	
	// Set the current reference count
	container->count = 1;

	return container;
}

PbContainer* pbcontainer_create2(MessageCategory category, int32_t type, uint8_t* data, uint64_t dataSize)
{
	WriteLog(LL_Warn, "here");

	if (category <= 0 || category >= MESSAGE_CATEGORY__MAX)
	{
		WriteLog(LL_Error, "category is out of bounds");
		return NULL;
	}

	WriteLog(LL_Warn, "here");

	if (!data || dataSize == 0)
	{
		WriteLog(LL_Error, "data or size is invalid (%p) (%llx)", data, dataSize);
		return NULL;
	}

	static PbMessage init_value = PB_MESSAGE__INIT;
	WriteLog(LL_Warn, "here");

	PbMessage* message = (PbMessage*)k_malloc(sizeof(PbMessage));
	if (!message)
	{
		WriteLog(LL_Error, "could not allocate PbMessage");
		return NULL;
	}

	WriteLog(LL_Warn, "here");

	*message = init_value;

	WriteLog(LL_Warn, "here");

	message->category = category;
	message->type = type;
	message->data.data = data;
	message->data.len = dataSize;

	WriteLog(LL_Warn, "here");

	PbContainer* container = pbcontainer_create(message);
	if (!container)
	{
		WriteLog(LL_Error, "pbcontainer_create returned null.");
		k_free(message);
		return NULL;
	}

	WriteLog(LL_Warn, "here");

	return container;
}

void pbcontainer_acquire(PbContainer* container)
{
	if (!container)
		return;

	__sync_fetch_and_add(&container->count, 1);

}

void pbcontainer_release(PbContainer* container)
{
	if (!container)
		return;

	if (!container)
		return;

	WriteLog(LL_Debug, "container %p count %d", container, container->count);

	// If the current reference count is <= 0 free everything
	if (__sync_sub_and_fetch(&container->count, 1) <= 0)
	{
		PbMessage* msg = container->message;
		if (!msg)
			return;

		// Free the protobuf message
		pb_message__free_unpacked(msg, NULL);
		container->message = NULL;

		k_free(container);
	}
}