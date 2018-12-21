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

	struct mtx lock;

	// DO NOT FUCKING TOUCH THIS VARIABLE DIRECTLY, USE THE FUCKING
	// ACQUIRE/RELEASE METHODS U FUCKING SCRUB
	volatile int32_t count;
} PbContainer;

PbContainer* pbcontainer_create(PbMessage* message);
PbContainer* pbcontainer_create2(MessageCategory category, int32_t type, uint8_t* data, uint64_t dataSize);

void pbcontainer_acquire(PbContainer* container);

void pbcontainer_release(PbContainer* container);

