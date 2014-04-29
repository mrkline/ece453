class SendMessage{

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


		void create_empty(void)
		{
			///< A message containing nothing but an ID
		}

		void create_response(void)
		{
			///< A generic response
		}

		void create_setup(void)
		{
			///< Set up a game
		}

		void create_start(void)
		{
			///< Start a game
		}

		void create_stop(void)
		{
			///< Stop a running game
		}

		void create_status(void)
		{
			///< Get the game status
		}

		void create_status_response(void)
		{
			///< Response to a status query
		}

		void create_results(void)
		{
			///< Get the game results after a match has finished
		}

		void create_results_response(void)
		{
			///< Response to a results query
		}

		void create_shot(void)
		{
			///< Represents a shot fired
		}

		void create_movement(void)
		{
			///< Carries the motion of a previously-reported shot
		}

		void create_target_control(void)
		{
			///< A message to set target lights on or off
		}

		void create_exit(void)
		{
			///< The entity receiving this message should exit/finish
		}

		void create_test(void)
		{
			///< A test payload that holds a string
		}

		void create_unknown(void)
		{
			///< An unknown/invalid payload type
		}
};
