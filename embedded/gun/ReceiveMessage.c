class ReceiveMessage{

	uint8_t payloadLength;
	uint16_t crc;
	/// Different types of message payloads.
	typedef enum {Type} type;
		EMPTY, ///< A message containing nothing but an ID
		RESPONSE, ///< A generic response
		SETUP, ///< Set up a game
		START, ///< Start a game
		STOP, ///< Stop a running game
		STATUS, ///< Get the game status
		STATUS_RESPONSE, ///< Response to a status query
		RESULTS, ///< Get the game results after a match has finished
		RESULTS_RESPONSE, ///< Response to a results query
		SHOT, ///< Represents a shot fired
		MOVEMENT, ///< Carries the motion of a previously-reported shot
		TARGET_CONTROL, ///< A message to set target lights on or off
		EXIT, ///< The entity receiving this message should exit/finish
		TEST, ///< A test payload that holds a string
		UNKNOWN ///< An unknown/invalid payload type


		//Check if the message came in correctly, load what was on ReceiveBuffer to msg
		bool ValidateMessage(char *msg, uint8_t gunID)
		{
			//Check the length of msg
			msgLength = strlen(*msg) - 4; //Minus 4 accounts for the CRC bits
			if(msgLength < 9)
			{
				return false;//If less than 9, dont send acknowledment
			}

			//Check the first two bytes of message for magic letters 'f' and 'u'
			if(msg[msgLength] != 'f' && msg[msgLength - 1] != 'u')
			{
				return false;
			}

			if(msg[msgLength - 2] != EMPTY || msg[msgLength - 2] != RESPONSE || msg[msgLength - 2] != SETUP ||
					msg[msgLength - 2] != START || msg[msgLength - 2] != STOP || msg[msgLength - 2] != STATUS ||
					msg[msgLength - 2] != STATUS_RESPONSE || msg[msgLength - 2] != RESULTS || msg[msgLength - 2] != RESULTS_RESPONSE ||
					msg[msgLength - 2] != SHOT || msg[msgLength - 2] != MOVEMENT || msg[msgLength - 2] != TARGET_CONTROL ||
					msg[msgLength - 2] != EXIT || msg[msgLength - 2] != TEST || msg[msgLength - 2] != UNKNOWN)//Check the type to see if it matches any in the enum
			{
				return false;
			}

			//Check ID to match to the guns ID
			if(msg[msgLength - 3, msgLength - 4] != gunID)
			{
				return false;
			}

			payloadLength = msg[msgLength - 5, msgLength - 6];		//Get the length of the payload

			crc = computeCRC();		//Get the CRC of the received message

			if(crc != msg[4,0])//Validate created CRC with the sent CRC
			{
				return false;
			}

			return true;
		}

		void empty(void)
		{
			///< A message containing nothing but an ID
			//Get the ID from message and store into a variable
		}

		void response(void)
		{
			///< A generic response
		}

		void setup(void)
		{
			///< Set up a game
		}

		void start(void)
		{
			///< Start a game
			//Let gun know the game is starting
		}

		void stop(void)
		{
			///< Stop a running game
			//Tell guns the game is over...stop timers
		}

		void status(void)
		{
			///< Get the game status
		}

		void status_response(void)
		{
			///< Response to a status query
		}

		void results(void)
		{
			///< Get the game results after a match has finished
		}

		void results_response(void)
		{
			///< Response to a results query
		}

		void shot(void)
		{
			///< Represents a shot fired
		}

		void movement(void)
		{
			///< Carries the motion of a previously-reported shot
		}

	/*	void target_control(void)
		{
			///< A message to set target lights on or off
		}*/

		void exit(void)
		{
			///< The entity receiving this message should exit/finish
		}

		void test(void)
		{
			///< A test payload that holds a string
		}

		void unknown(void)
		{
			///< An unknown/invalid payload type
		}


};
