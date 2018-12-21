#include <oni/messaging/pbcontainer.h>
#include <protobuf-c/mirabuiltin.pb-c.h>

#include <oni/utils/memory/allocator.h>

PbContainer* pbcontainer_create(PbMessage* message)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);

	if (!message)
		return NULL;

	PbContainer* container = k_malloc(sizeof(PbContainer));
	if (!container)
		return NULL;

	// Zero out the new container
	memset(container, 0, sizeof(*container));

	container->message = message;

	// Initialize the lock
	mtx_init(&container->lock, "pbcmtx", NULL, 0);
	
	// Set the current reference count
	container->count = 1;

	return container;
}

PbContainer* pbcontainer_create2(MessageCategory category, int32_t type, uint8_t* data, uint64_t dataSize)
{
	if (category <= 0 || category >= MESSAGE_CATEGORY__MAX)
		return NULL;

	if (!data || dataSize == 0)
		return NULL;
	static PbMessage init_value = PB_MESSAGE__INIT;
	PbMessage* message = (PbMessage*)k_malloc(sizeof(PbMessage));
	if (!message)
		return NULL;

	*message = init_value;

	message->category = category;
	message->type = type;
	message->data.data = data;
	message->data.len = dataSize;

	PbContainer* container = pbcontainer_create(message);
	if (!container)
	{
		k_free(message);
		return NULL;
	}

	return container;
}

void pbcontainer_acquire(PbContainer* container)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!container)
		return;

	_mtx_lock_flags(&container->lock, 0, __FILE__, __LINE__);
	
	container->count += 1;

	_mtx_unlock_flags(&container->lock, 0, __FILE__, __LINE__);

}

void pbcontainer_release(PbContainer* container)
{
	if (!container)
		return;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!container)
		return;

	_mtx_lock_flags(&container->lock, 0, __FILE__, __LINE__);

	container->count -= 1;

	// If the current reference count is <= 0 free everything
	if (container->count <= 0)
	{
		PbMessage* msg = container->message;
		if (!msg)
			return;

		// Free the internal data
		if (msg->data.data)
		{
			k_free(msg->data.data);
			msg->data.data = NULL;
			msg->data.len = 0;
		}

		// Free the protobuf message
		pb_message__free_unpacked(msg, NULL);
		container->count = 0;
	}

	_mtx_unlock_flags(&container->lock, 0, __FILE__, __LINE__);

	// Free the container itself
	if (container->count <= 0)
		k_free(container);
}