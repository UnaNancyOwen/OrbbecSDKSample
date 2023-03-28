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

    // Infrared
    std::shared_ptr<ob::VideoStreamProfile> infrared_stream_profile = nullptr;
    std::shared_ptr<ob::IRFrame> infrared_frame = nullptr;
    cv::Mat infrared;

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

    // Update Infrared
    void update_infrared();

    // Draw Infrared
    void draw_infrared();

    // Show Infrared
    void show_infrared();
};

#endif // __ORBBEC__
