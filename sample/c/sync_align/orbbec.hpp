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

    // Depth
    ob_stream_profile* depth_stream_profile = nullptr;
    ob_frame* depth_frame = nullptr;
    cv::Mat depth;
    std::tuple<double, double> depth_range = std::make_tuple<double, double>( 0.0, 0.0 );

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

    // Update Depth
    void update_depth();

    // Update Color
    void update_color();

    // Draw Color
    void draw_color();

    // Draw Depth
    void draw_depth();

    // Show Color
    void show_color();

    // Show Depth
    void show_depth();

    // Get Depth Range
    std::tuple<double, double> get_depth_range( ob_stream_profile* depth_stream_profile );
};

#endif // __ORBBEC__
