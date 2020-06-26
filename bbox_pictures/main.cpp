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
int step = 1;

//adds bbox coordinates in yolo-format to output-string, values should be formatted already as input
void addBBoxToString(int* input)
{
    int x = input[0];
    int y = input[1];
    int width = input[2];
    int height = input[3];
    bboxes_string +=  std::to_string(frame_counter) + " " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(width) + " " + std::to_string(height) + "\n";
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
            // draw new box
            rectangle(frame, box[0], box[1], 0xff0000, 2);
            imshow("images", frame);
        }
        else
            {
                // save box to frame_b to display later
                frame.copyTo(frame_b);
                // Draw bounding box
                rectangle(frame, box[0], box[1], 0xff0000, 2);
                box_count++;
                putText(frame, "Box: " + std::to_string(box_count), Point(220,22), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 1);
                imshow("images", frame);
                addBBoxToString((int*)userdata);
            }
    }
}
int main(int argc, char** argv)
{
    // File input
    CommandLineParser parser(argc, argv, "{input | /home/johann/Schreibtisch/diag1_re3 | path to images}"
                                                "{max_frames | 78 | how many images}"
                                                "{step | 10 | how many frames between images}");

    int max_frames = parser.get<int>("max_frames");
    step = parser.get<int>("step");
    std::string input = parser.get<std::string>("input");
    frame_counter = -step;
    // Window to display video, need name for mouse callback
    namedWindow("images", WINDOW_AUTOSIZE);
    // 2 points (x1,y1, x2,y2) to save the x/y coordinates of mouse clicks
    int points[4] = {-1, -1, -1, -1};
    // register callback function, it will save the coordinates in points
    setMouseCallback("images", CallBackFunc, &points[0]);
    // Loop through the video
    for(int i = 0; i <= max_frames; ++i)
    {
        //read image
        std::string filename = input + "/" + std::to_string(i * step) + ".jpg";
        frame = imread(filename);
        frame_counter += step;
        first_box = true;
        box_count = 0;
        if (frame.empty()) break;
        putText(frame, "Frame: " + std::to_string(frame_counter), Point(5,22), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 1);
        // Display frame
        imshow("images", frame);
        // quit with q or esc
        int keyboard = waitKey();
        if (keyboard == 'q' || keyboard == 27)
            break;
        // save bounding boxes to textfile
        else if (keyboard == 's')
        {
            // save image if needed
           // std::string s = "../output/" + std::to_string(frame_counter);
           // imwrite(s+".jpg", frame_clean);
        }
        // next frame with n
        else if (keyboard == 'n')
            continue;
    }
    // Create textfile
    std::ofstream outPutFile("../output/labels.txt");
    // Object class 0 for now, since only 1 BB pro image possible now anyway
    outPutFile << bboxes_string;
}