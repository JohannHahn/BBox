#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace cv;
// Matrix for current frame, global so it can be redrawn in mouse callback
Mat frame;
// frame without bboxes
Mat frame_clean;
// previous frame
Mat frame_b;
// checks if the current box is the first on the current frame
bool first_box = true;
//String with all bounding boxes, saved to .txt after pressing s
std::string bboxes_string = "";
int box_count = 0;
int frame_counter = 0;

//adds bbox coordinates in yolo-format to output-string, values should be formatted already as input
void addBBoxToString(float* input, int object_class)
{
    float x = input[0];
    float y = input[1];
    float width = input[2];
    float height = input[3];
    bboxes_string +=  std::to_string(object_class) + " " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(width) + " " + std::to_string(height) + "\n";
}
/*calculates the bbox in yolo-format from any abcd rectangle (input values) on screen 
 * to the yolo format (output values)
 */
void bboxToYolo(int* input, float* output)
{
    /* yolo data format: text file with the same name as the image and contents of:
             * <object-class> <x> <y> <width> <height>
             * class is some integer 0 to classes-1
             * x, y are floats (0 to 1) he center of the bounding box relative to image size
             * height, width also floats from 0 to 1, size of BB relative to image size
             */

    /* convert from top-left, bottom-right points to center and divide by img-size
     * min(points[0], points[2] = x of top left corner, same for y
     */
    float width = min(abs(input[2] - input[0]) / (float)frame.cols, 1.f);
    float height = min(abs(input[3] - input[1]) / (float)frame.rows, 1.f);

    float x = min(((abs(input[2] - input[0]) / 2.f) + min(input[0], input[2])) / (float)frame.cols, 1.f);
    float y = min(((abs(input[3] - input[1]) / 2.f) + min(input[1], input[3])) / (float)frame.rows, 1.f);

    output[0] = x;
    output[1] = y;
    output[2] = width;
    output[3] = height;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    // Save first point on left mouse button click
    if(event == EVENT_LBUTTONDOWN)
    {
        ((int*)userdata)[0] = x;
        ((int*)userdata)[1] = y;
        if(first_box)
        {
            //copy image without bounding boxes to backup to
            frame.copyTo(frame_clean);
            frame.copyTo(frame_b);
            first_box = false;
        }
    }
    // save second point and draw BB on left mouse button drag or up
    if(event == EVENT_LBUTTONUP || (event == EVENT_MOUSEMOVE && flags & EVENT_FLAG_LBUTTON))
    {
        ((int*)userdata)[2] = x;
        ((int*)userdata)[3] = y;
        // Convert points to Point2f
        Point2f box[2];
        box[0] = Point2f(((int*)userdata)[0], ((int*)userdata)[1]);
        box[1] = Point2f(x, y);
        // redraw box while holding mousebutton
        if (event != EVENT_LBUTTONUP)
        {
            // frame_b holds on to previous drawn boxes
            frame_b.copyTo(frame);
            rectangle(frame, box[0], box[1], 0xff0000, 2);
            imshow("video", frame);
        }
        else
            {
                // save box to frame_b to display later
                frame.copyTo(frame_b);
                // Draw bounding box
                rectangle(frame, box[0], box[1], 0xff0000, 2);
                box_count++;
                putText(frame, "Box: " + std::to_string(box_count), Point(220,22), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 1);
                imshow("video", frame);

                // calculate bbox in yolo-format and save to ouput string
                float bbox_yolo[4] = {0.0, 0.0, 0.0, 0.0};
                bboxToYolo((int *) userdata, &bbox_yolo[0]);
                addBBoxToString(&bbox_yolo[0], 0);
            }
    }
}
int main(int argc, char** argv)
{
    // File input
    CommandLineParser parser(argc, argv, "{input | ../robot_stat3.mp4 | path to video}"
                                               "{step  | 1 | how many frames to skip}");
    // Read video file
    VideoCapture capture(parser.get<String>("input"));
    int step = parser.get<int>("step");
    frame_counter = -1;
    if (!capture.isOpened())
    {
        //error in opening the video input
        std::cerr << "Unable to open: " << parser.get<String>("input") << std::endl;
        return 0;
    }
    // Window to display video, need name for mouse callback
    namedWindow("video", WINDOW_AUTOSIZE);
    // 2 points (x1,y1, x2,y2) to save the x/y coordinates of mouse clicks
    int points[4] = {-1, -1, -1, -1};
    // register callback function, it will save the coordinates in points
    setMouseCallback("video", CallBackFunc, &points[0]);
    // Loop through the video
    while(true)
    {
        if (frame_counter >= 0) {
            // Next frame to image
            for (int i = 0; i < step; ++i) {
                frame_counter += 1;
                capture >> frame;
            }
        } else {
            capture >> frame;
            frame_counter = 0;
        }
        first_box = true;
        box_count = 0;

        if (frame.empty()) break;
        putText(frame, "Frame: " + std::to_string(frame_counter), Point(5,22), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 1);
        // Display frame
        imshow("video", frame);
        // quit with q or esc
        int keyboard = waitKey();
        if (keyboard == 'q' || keyboard == 27)
            break;

        // save bounding boxes to textfile
        else if (keyboard == 's')
        {
            // save image
            std::string s = "../output/" + std::to_string(frame_counter);
            imwrite(s+".jpg", frame_clean);

            // Create textfile
            std::ofstream outPutFile(s+".txt");
            // Object class 0 for now, since only 1 BB pro image possible now anyway
            outPutFile << bboxes_string;
            bboxes_string = "";
        }
        // next frame with n
        else if (keyboard == 'n')
            continue;
    }
}
