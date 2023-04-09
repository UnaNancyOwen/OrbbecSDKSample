#ifndef __ORBBEC__
#define __ORBBEC__

#include <libobsensor/ObSensor.h>
#include <open3d/Open3D.h>

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

    // Depth
    ob_stream_profile* depth_stream_profile = nullptr;

    // Point Cloud
    ob_format format = ob_format::OB_FORMAT_RGB_POINT;
    ob_filter* pointcloud_filter = nullptr;
    ob_frame* pointcloud_frame = nullptr;
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
