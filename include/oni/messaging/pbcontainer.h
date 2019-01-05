#pragma once
#include <oni/utils/types.h>
#include <protobuf-c/mirabuiltin.pb-c.h>

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

/// <summary>
/// Message container that holds references to the raw input data,
/// the decoded message that both need to be freed when all references are gone
/// Will need to implement some kind of reference counting in this object
/// </summary>
typedef struct pbcontainer_t
{
	PbMessage* message;

	// DO NOT FUCKING TOUCH THIS VARIABLE DIRECTLY, USE THE FUCKING
	// ACQUIRE/RELEASE METHODS U FUCKING SCRUB
	volatile int32_t count;

	uint8_t messageOwned;
} PbContainer;

/// <summary>
/// pbcontainer_create
/// messageOwned is set to true when WE allocate a PbMessage with k_malloc,
/// if it is from pb_message__unpack we set to false
/// </summary>
PbContainer* pbcontainer_create(PbMessage* message, uint8_t messageOwned);
PbContainer* pbcontainer_createNew(MessageCategory category, int32_t type, uint8_t* data, uint64_t dataSize);

void pbcontainer_acquire(PbContainer* container);

void pbcontainer_release(PbContainer* container);

