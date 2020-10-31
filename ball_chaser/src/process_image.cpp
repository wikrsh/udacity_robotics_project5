#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv)) {
        ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int count_left = 0;
    int count_mid = 0;
    int count_right = 0;

    for (int h = 0; h < img.height; h++) {
        for (int w = 0; w < img.width; w++) {
            int i = 3 * (h * img.width + w);
            int r = img.data[i];
            int g = img.data[i+1];
            int b = img.data[i+2];
            if (r != white_pixel || g != white_pixel || b != white_pixel) {
                continue;
            }

            if (w < img.width / 3) {
                count_left++;
            } else if (w < (2 * img.width / 3)) {
                count_mid++;
            } else {
                count_right++;
            }
        }
    }

    if (count_left == 0 && count_mid == 0 && count_right == 0) {
        // stop
        drive_robot(0.0, 0.0);
    } else if (count_left > count_mid && count_left > count_right) {
        // turn left
        drive_robot(0.0, 0.3);
    } else if (count_right > count_mid) {
        // turn right
        drive_robot(0.0, -0.3);
    } else {
        // move foward
        drive_robot(0.3, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
