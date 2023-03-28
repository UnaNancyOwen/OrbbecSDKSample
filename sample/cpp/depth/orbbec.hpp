#ifndef __ORBBEC__
#define __ORBBEC__

#include <libobsensor/ObSensor.hpp>
#include <opencv2/opencv.hpp>

class orbbec
{
private:
    // Orbbec
    uint32_t device_index = 0;
    ob::Context context;
    std::shared_ptr<ob::Pipeline> pipeline = nullptr;
    std::shared_ptr<ob::Device > device = nullptr;
    std::shared_ptr<ob::Config> config = nullptr;
    std::shared_ptr<ob::FrameSet> frameset = nullptr;

    // Depth
    std::shared_ptr<ob::VideoStreamProfile> depth_stream_profile = nullptr;
    std::shared_ptr<ob::DepthFrame> depth_frame = nullptr;
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

    // Draw Depth
    void draw_depth();

    // Show Depth
    void show_depth();

    // Get Depth Range
    std::tuple<double, double> get_depth_range( std::shared_ptr<ob::VideoStreamProfile> depth_stream_profile );
};

#endif // __ORBBEC__
