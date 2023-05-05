#pragma once
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#pragma comment(lib, "tobii_stream_engine.lib")


// STL
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

//alignment for atomic storage
//struct/* __declspec(align(64))*/ Point2f { float x, y; };

class Tobii final
{
public:
	Tobii();
	~Tobii();
	Tobii(const Tobii&) = delete;
	Tobii& operator=(const Tobii&) = delete;

	std::vector<std::string> ListDevices(tobii_api_t * api);
	std::string SelectDevice(std::vector<std::string> const & devices);
	static void Log(void * log_context, tobii_log_level_t level, char const * text);
	tobii_error_t Reconnect(tobii_device_t * device);
	Point2f GetPoint();
private:
	tobii_api_t * m_pApi;
	tobii_device_t* m_pDevice;
	// Create atomic used for inter thread communication
	std::atomic<bool> m_IsExit;
	std::thread Thread;
	//std::atomic<Point2f> m_Point;
	Point2f m_Point;
};

