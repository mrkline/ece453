# Proposed JSON schema for the user interface

This is terribly incomplete. Additions will be made as the design is fleshed out.

## Message protocol

All messages in this protocol will be encapsulated in a JSON object with the following fields:

- "type" - The type of message being sent. Used to determine what parser to use for the payload

- "version" - An integer representing the version of this protocol. This version is 1.

- "id" - An ID/sequence number for the message. Each ID must be a sequentially increasing value compared to other
  messages sent by a given sender (i.e. the UI or the system).

- "payload" - The information being sent, in the form of one of the objects below

## Response

A general response from the controller to the UI, unless otherwise specified below, will be of type "response"
and contain a payload object with the following fields:

- "responding to" - The ID of the message that this one is a response to

- "code" - One of the following strings:

    - "ok" - Everything went as expected

    - "internal error" - Something went wrong inside the system

    - "unknown request" - The system couldn't make sense of the request

    - "invalid request" - The request was valid but was given at an inappropriate time.
                          An example may be issuing a stop command when no game is running.

    - "unsupported request" - The request was valid but the system could not fulfill it.
                      An example may be requesting a game setup with more players than the system supports.

- "message" - A field containing any text message to accompany the error code. Empty if there is no message.

## Setup

To set up a game, a message with type "setup" and a payload object with the following fields will be sent to the system:

- "game type" - One of the following:

  - "pop-up" - A target flashes and players shoot it. Score is based on how quickly they shoot.

  - "follow-up" - Two targets pop up in succession and players shoot them in the order they popped up.
                  Score is based on how quickly players hit both targets.

  - "dump" - A target flashes and players must shoot the target a given number of times.
             The game data field will contain a "shot count" field, which will be an integer indicating
             how many shots the player must fire.

- "player count" - The number of players for the game

- "win conditions" - An array containing one or both of the following
  (in which case the game ends when the first of the two is reached):

  - "time" - End the game after a given amount of time

  - "points" - End the game after a user has scored a given number of points

- "end time" - If time is a win condition, this field gives the maximum game length, in seconds.
               If time is not a win condition, this field contains -1.

- "winning score" - If points is a win condition, this field gives the score a player must get to win.
                    If points is not a win condition, this field contains -1.

- "game data" - Some game types may require additional setup information, which will be in this object.
                This field is not expected if the game type does not require it.
                If it is present, it contains a map of integers.

## Start

To start the game, a message with type "start" is sent to the system with an empty payload object.


## Stop

To stop a running game, a message with type "stop" is sent to the system with an empty payload object.

## Status

To get the status of the system, a message of type "get status" is sent to the system with an empty payload object.
Future versions of the protocol may have this request contain a payload indicating which fields to fetch
instead of mandating a monolithic update.

## Status response

Instead of the usual response (see above), status requests will be met with a message of type "status response"
with the following payload:

- As a subclass of response, this payload will contain all the members a normal response payload contains.

- "running" - A boolean indicating whether the game is currently running. If this value is false,
              no other fields will be contained in the payload.

- "time remaining" - An integer indicating the number of remaining seconds, or -1 if the current game has no time limit.

- "winning score" - An integer indicating the score required to win a game, or -1 if the current game has no score limit.

- "player stats" - An array of objects containing the following fields:

  - "score" - An integer representing the player's current score

  - "hits" - The number of hits the player has gotten so far

## Results

To get the detailed results of a match, a message of type "get results" is sent to the system with an empty payload.
This must be done _after_ a match has been run.
Future versions of the protocol may have this request contain a payload indicating which fields to fetch
instead of mandating a monolithic update.

## Results response

Instead of the usual response (see above), results requests will be met with a message of type "results response"
with the following payload:

- As a subclass of a response, this payload will contain all the members a normal response payload contains.

- "player stats" - An array of objects containing the following fields:

  - "score" - An integer representing the player's final score

  - "hits" - The number of hits the player got in the round

  - "shots" - An array of objects, each one representing a shot the player took, containing the following information:

        - "time" - An integer representing the time at which the shot was taken, in milliseconds since the game started.

        - "hit" - A boolean indicating whether or not the shot was a hit

        - "movement" - An array of decimal arrays of the form `[x, y, z]` which indicates motion leading up to the shot.
                       These values will be at a known, to be determined interval.
