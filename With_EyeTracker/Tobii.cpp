#include "pch.h"
#include "Tobii.h"


// a static function that returns auto can not be used before it is defined
Tobii::Tobii()
	:m_IsExit{ false }
{

	// Create log mutex used for thread synchronization in log function
	std::mutex logMutex;
	tobii_custom_log_t custom_log{ &logMutex, Log };


	auto error = tobii_api_create(&m_pApi, nullptr, &custom_log);
	if (error != TOBII_ERROR_NO_ERROR)
	{
		std::cerr << "Failed to initialize the Tobii Stream Engine API." << std::endl;
		return;
	}

	std::vector<std::string>  devices = ListDevices(m_pApi);
	if (devices.size() == 0)
	{
		std::cerr << "No stream engine compatible device(s) found." << std::endl;
		tobii_api_destroy(m_pApi);
		return;
	}
	std::string  selected_device = devices.size() == 1 ? devices[0] : SelectDevice(devices);
	std::cout << "Connecting to " << selected_device << "." << std::endl;


	error = tobii_device_create(m_pApi, selected_device.c_str(), &m_pDevice);
	if (error != TOBII_ERROR_NO_ERROR)
	{
		std::cerr << "Failed to initialize the device with url " << selected_device << "." << std::endl;
		tobii_api_destroy(m_pApi);
		return;
	}
	// Start the background processing thread before subscribing to data
	// move semantics
	Thread = std::thread(
		[this]()
	{
		while (!(this->m_IsExit))
		{
			// Do a timed blocking wait for new gaze data, will time out after some hundred milliseconds
			auto error = tobii_wait_for_callbacks(NULL, 1, &(this->m_pDevice));

			if (error == TOBII_ERROR_TIMED_OUT) continue; // If timed out, redo the wait for callbacks call

			if (error == TOBII_ERROR_CONNECTION_FAILED)
			{
				// Block here while attempting reconnect, if it fails, exit the thread
				error = Reconnect(this->m_pDevice);
				if (error != TOBII_ERROR_NO_ERROR)
				{
					std::cerr << "Connection was lost and reconnection failed." << std::endl;
					return;
				}
				continue;
			}
			else if (error != TOBII_ERROR_NO_ERROR)
			{
				std::cerr << "tobii_wait_for_callbacks failed: " << tobii_error_message(error) << "." << std::endl;
				return;
			}
			// Calling this function will execute the subscription callback functions
			error = tobii_device_process_callbacks((this->m_pDevice));

			if (error == TOBII_ERROR_CONNECTION_FAILED)
			{
				// Block here while attempting reconnect, if it fails, exit the thread
				error = Reconnect(this->m_pDevice);
				if (error != TOBII_ERROR_NO_ERROR)
				{
					std::cerr << "Connection was lost and reconnection failed." << std::endl;
					return;
				}
				continue;
			}
			else if (error != TOBII_ERROR_NO_ERROR)
			{
				std::cerr << "tobii_device_process_callbacks failed: " << tobii_error_message(error) << "." << std::endl;
				return;
			}
		}
	});


	// Start subscribing to gaze and supply lambda callback function to handle the gaze point data
	error = tobii_gaze_point_subscribe(m_pDevice,
		[](tobii_gaze_point_t const* gaze_point, void* user_data)
	{
		Point2f *pPoint = (Point2f*)user_data; // Unused parameter
		if (gaze_point->validity == TOBII_VALIDITY_VALID)
		{
			//std::cout << "Gaze point: " << gaze_point->timestamp_us << " " << gaze_point->position_xy[0]
			//	<< ", " << gaze_point->position_xy[1] << std::endl;
			pPoint->x = gaze_point->position_xy[0];
			pPoint->y = gaze_point->position_xy[1];
			//std::cout << pPoint->x << '\t' << pPoint->y << '\n';
		}
		else
		{
			// screen is not being watched
			pPoint->x = -1;
			pPoint->y = -1;
			//std::cout << "Gaze point: " << gaze_point->timestamp_us << " INVALID" << std::endl;
			//std::cout << ".";
		}
	}, &m_Point);
	// last param is userdata is pointerto Point2f that is passed as parameter to the callback lambda

	if (error != TOBII_ERROR_NO_ERROR)
	{
		std::cerr << "Failed to subscribe to gaze stream." << std::endl;
		m_IsExit = true;
		Thread.join();
		tobii_device_destroy(m_pDevice);
		tobii_api_destroy(m_pApi);
		return;
	}
}

Tobii::~Tobii()
{
	std::mutex log_mutex;
	tobii_custom_log_t custom_log{ &log_mutex, Log };
	auto error = tobii_api_create(&m_pApi, nullptr, &custom_log);
	// Cleanup subscriptions and resources
	error = tobii_gaze_point_unsubscribe(m_pDevice);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to unsubscribe from gaze stream." << std::endl;

	m_IsExit = true;
	Thread.join();

	error = tobii_device_destroy(m_pDevice);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to destroy device." << std::endl;

	error = tobii_api_destroy(m_pApi);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to destroy API." << std::endl;

}

std::vector<std::string> Tobii::ListDevices(tobii_api_t* api)
{
	std::vector<std::string> result;
	auto error = tobii_enumerate_local_device_urls(api,
		[](char const* url, void* user_data) // Use a lambda for url receiver function
	{
		// Add url string to the supplied result vector
		auto list = (std::vector<std::string>*) user_data;
		list->push_back(url);
	}, &result);
	if (error != TOBII_ERROR_NO_ERROR) std::cerr << "Failed to enumerate devices." << std::endl;

	return result;
}



std::string Tobii::SelectDevice(std::vector<std::string> const& devices)
{
	auto selection = 0u;
	// Present the available devices and loop until user has selected a valid device
	while (selection < 1 || selection > devices.size())
	{
		std::cout << std::endl << "Select a device" << std::endl << std::endl;
		auto index = 0;
		for (auto t : devices) std::cout << ++index << ". " << t << std::endl;

		if (!(std::cin >> selection))
		{
			// Clean stdin before reading new value
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	return devices[selection - 1];
}


void Tobii::Log(void* log_context, tobii_log_level_t level, char const* text)
{
	// Synchronize access to the log printout
	std::lock_guard<std::mutex> lock(*(std::mutex*)log_context);

	if (level == TOBII_LOG_LEVEL_ERROR)
		std::cerr << "Logged error: " << text << std::endl;
}


tobii_error_t Tobii::Reconnect(tobii_device_t* device)
{
	// Try reconnecting for 10 seconds before giving up
	for (int i = 0; i < 40; ++i)
	{
		auto error = tobii_device_reconnect(device);
		if (error != TOBII_ERROR_CONNECTION_FAILED) return error;
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	return TOBII_ERROR_CONNECTION_FAILED;
}

Point2f Tobii::GetPoint()
{
	return m_Point;
}
