#ifndef __ORBBEC__
#define __ORBBEC__

#include <libobsensor/ObSensor.h>
#include <opencv2/opencv.hpp>

class orbbec
{
private:
    // Orbbec
    uint32_t device_index = 0;
    std::string address = "192.168.1.10";
    uint16_t port = 8090;
    ob_error* error = NULL;
    ob_context* context = nullptr;
    ob_pipeline* pipeline = nullptr;
    ob_device* device = nullptr;
    ob_config* config = nullptr;
    ob_frame* frameset = nullptr;

    // Color
    ob_stream_profile* color_stream_profile = nullptr;
    ob_frame* color_frame = nullptr;
    cv::Mat color;

public:
    // Constructor
    orbbec();

    // Destructor
    ~orbbec();

    // Run
    void run();

    // Update
    void update();

    // Draw
    void draw();

    // Show
    void show();

private:
    // Initialize
    void initialize();

    // Initialize Sensor
    void initialize_sensor();

    // Finalize
    void finalize();

    // Update Frame
    void update_frame();

    // Update Color
    void update_color();

    // Draw Color
    void draw_color();

    // Show Color
    void show_color();
};

#endif // __ORBBEC__
