#ifndef __ORBBEC__
#define __ORBBEC__

#include <libobsensor/ObSensor.hpp>
#include <open3d/Open3D.h>

class orbbec
{
private:
    // Orbbec
    uint32_t device_index = 0;
    std::string address = "192.168.1.10";
    uint16_t port = 8090;
    ob::Context context;
    std::shared_ptr<ob::Pipeline> pipeline = nullptr;
    std::shared_ptr<ob::Device > device = nullptr;
    std::shared_ptr<ob::Config> config = nullptr;
    std::shared_ptr<ob::FrameSet> frameset = nullptr;

    // Color
    std::shared_ptr<ob::VideoStreamProfile> color_stream_profile = nullptr;

    // Depth
    std::shared_ptr<ob::VideoStreamProfile> depth_stream_profile = nullptr;

    // Point Cloud
    OBFormat format = OBFormat::OB_FORMAT_RGB_POINT;
    std::shared_ptr<ob::PointCloudFilter> pointcloud_filter;
    std::shared_ptr<ob::Frame> pointcloud_frame = nullptr;
    std::shared_ptr<open3d::geometry::PointCloud> pointcloud = nullptr;
    open3d::visualization::VisualizerWithKeyCallback visualizer;
    bool is_run = true;

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

    // Initialize Point Cloud
    void initialize_pointcloud();

    // Finalize
    void finalize();

    // Update Frame
    void update_frame();

    // Update Point Cloud
    void update_pointclod();

    // Draw Point Cloud
    void draw_pointcloud();

    // Show Point Cloud
    void show_pointcloud();
};

#endif // __ORBBEC__
