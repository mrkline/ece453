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

A response from the controller to the UI, unless otherwise specified below, will be of type "response"
and contain a payload object with the following fields:

- "responding to" - The ID of the message that this one is a response to

- "code" - One of the following strings:

    - "ok" - Everything went as expected

    - "internal error" - Something went wrong inside the system

    - "unknown request" - The system couldn't make sense of the request

    - "invalid request" - The request was valid but was given at an inappropriate time.
                          An example may be issuing a stop command when no game is running.

    - "unsupported" - The request was valid but the system could not fulfill it.
                      An example may be requesting a game setup with more players than the system supports.

- "message" - (OPTIONAL) A field containing any text message to accompany the error code

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

- "game data" - Some game types may require additional setup information, which will be in this object.
                This field is not expected if the game type does not require it.

## Start

To start the game, a message with type "start" and the following payload is sent to the system:

- "win condition" - An array containing one or both of the following
  (in which case the game ends when the first of the two is reached):

  - "time" - End the game after a given amount of time

  - "points" - End the game after a user has scored a given number of points

- "end time" - If "time" is listed as a win condition,
               this field is present and gives the maximum game length, in seconds.
               If "time" is not a win condition, this field should not exist.

- "winning score" - If "points" is listed as a win condition,
                    this field is present and gives the score a player must get to win.
                    If "points" is not a win condition, this field should not exist.

## Stop

To stop a running game, a message with type "stop" is sent to the system with an empty payload object.
