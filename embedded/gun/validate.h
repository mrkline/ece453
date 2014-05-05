/*
 * validate.h
 *
 *  Created on: May 2, 2014
 *      Author: James G
 */

#ifndef VALIDATE_H_
#define VALIDATE_H_

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

/// Extracts at 16-bit unsigned integer from the memory at `buf`
uint16_t extractUInt16(const uint8_t* buf)
{
	return (uint16_t)(
		((uint16_t)buf[0] << 8) +
		((uint16_t)buf[1] << 0));
}

/// Extracts at 32-bit unsigned integer from the memory at `buf`
uint32_t extractUInt32(const uint8_t* buf)
{
	return ((uint32_t)buf[0] << 24)
	     + ((uint32_t)buf[1] << 16)
	     + ((uint32_t)buf[2] << 8)
	     + ((uint32_t)buf[3] << 0);
}

/// Extracts at 16-bit signed integer from the memory at `buf`
inline int16_t extractInt16(const uint8_t* buf) { return (int16_t)extractUInt16(buf); }

/// Extracts at 32-bit signed integer from the memory at `buf`
inline int32_t extractInt32(const uint8_t* buf) { return (int32_t)extractUInt32(buf); }

// Assume we have some getCRC function elsewhere
uint16_t getCRC(void* buf, size_t len);

typedef enum MessageType_t {
	MT_EMPTY, ///< A message containing nothing but an ID
	MT_RESPONSE, ///< A generic response
	MT_QUERY, ///< Query a board to see if it's in the system
	MT_SETUP, ///< Set up a game
	MT_START, ///< Start a game
	MT_STOP, ///< Stop a running game
	MT_STATUS, ///< Get the game status
	MT_STATUS_RESPONSE, ///< Response to a status query
	MT_RESULTS, ///< Get the game results after a match has finished
	MT_RESULTS_RESPONSE, ///< Response to a results query
	MT_SHOT, ///< Represents a shot fired
	MT_MOVEMENT, ///< Carries the motion of a previously-reported shot
	MT_TARGET_CONTROL, ///< A message to set target lights on or off
	MT_EXIT, ///< The entity receiving this message should exit/finish
	MT_TEST, ///< A test payload that holds a string
	MT_UNKNOWN ///< An unknown/invalid payload type
} MessageType;

// States for our receiver state machine
typedef enum ReceiveState_t {
	RS_MAGIC1,
	RS_MAGIC2,
	RS_TYPE,
	RS_ID1,
	RS_ID2,
	RS_LEN1,
	RS_LEN2,
	RS_PAYLOAD,
	RS_CRC1,
	RS_CRC2,
	RS_END1,
	RS_END2,
	RS_VALID
} ReceiveState;

uint8_t buffer[256]; ///< Our 256-byte buffer

ReceiveState currentState = RS_MAGIC1;

/// Our cursor for the buffer. Points to the next free slot in the buffer
uint8_t* cursor = buffer;

/// The type of the message. Only valid once we have passed the state RS_TYPE
int8_t type = -1;

/// The ID of the message. Only valid once we have passed the state RS_ID2
uint16_t currentID = 0;

/// The length of the current peyload. Only valid once we have passed the state RS_LEN2
uint16_t payloadLength = 0;

/// The payload amount we have read so far
uint16_t payloadRead = 0;

/// Call for each byte we receive from the uart
/// Returns true when we get a valid packet
bool onChar(uint8_t c)
{
	// Stores the char in the buffer and advances the cursor
	*cursor = c;
	++cursor;

	switch (currentState) {
		case RS_MAGIC1:
			// Advance if we get the right magic byte
			currentState = c == 'f' ? RS_MAGIC2 : RS_MAGIC1;
			break;

		case RS_MAGIC2:
			// Advance if we get the right magic byte
			currentState = c == 'u' ? RS_TYPE : ((c == 'f') ? RS_MAGIC2 : RS_MAGIC1);
			break;

		case RS_TYPE:
			// Advance if the type is in our known set of types
			type = (int8_t)c;
			currentState = type >= 0 && type <= MT_UNKNOWN ? RS_ID1 : RS_MAGIC1;
			// TODO: We can also reset the state machine if we don't care about a given type
			break;

		case RS_ID1:
			// Keep walking
			currentState = RS_ID2;
			break;

		case RS_ID2:
			// Assemble our current ID
			currentID = extractUInt16(cursor - 2);
			currentState = RS_LEN1;
			break;

		case RS_LEN1:
			// Keep walking
			currentState = RS_LEN2;
			break;

		case RS_LEN2:
			// Assemble our current length
			payloadLength = extractUInt16(cursor - 2);
			if(payloadLength == 0){
				currentState = RS_END1;
			}
			else{
				currentState = RS_PAYLOAD;
			}
			break;

		case RS_PAYLOAD:
			// Read the payload until we hit our CRC
			if (++payloadRead >= payloadLength)
				currentState = RS_END1;
			break;

		case RS_END1:
						// Advance if we get the right magic byte
						currentState = c == 'y' ? RS_END2 : RS_MAGIC1;
						break;

		case RS_END2:
						currentState = c == 'y' ? RS_VALID : RS_MAGIC1;
						if(c == 'y')
							return true;
						else
							return false;
		case RS_VALID:
			// TODO: Add some error indicator here.
			// We should not be providing a new character
			break;
	}

	// Reset all our stuff if we're resetting the state
	if (currentState == RS_VALID) {//reset everything, treat onChar iteration as RS_MAGIC1
		cursor = buffer;
		type = 0;
		currentID = 0;
		payloadLength = 0;
		payloadRead = 0;
		currentState = c == 'f' ? RS_MAGIC2 : RS_MAGIC1;
	}
		return false;//
	// Return true if we're done
	//return currentState == RS_VALID;
}


#endif /* VALIDATE_H_ */
