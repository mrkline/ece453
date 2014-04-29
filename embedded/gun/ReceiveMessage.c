class ReceiveMessage{

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



		bool ValidateMessage(char *msg)
		{
			message = msg;			//Put into our receive buffer

			//Check the length of msg
			//If less than 9, dont send acknowledment

			//Check the first two bytes of message for magic letters 'f' and 'u'

			//Check the type to see if it matches any in the enum

			//Check ID to match to the guns ID

			//Get the length of the payload

			computeCRC();		//Get the CRC of the received message

			//Validate created CRC with the sent CRC
		}

		void empty(void)
		{
			///< A message containing nothing but an ID
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
		}

		void stop(void)
		{
			///< Stop a running game
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

		void target_control(void)
		{
			///< A message to set target lights on or off
		}

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
