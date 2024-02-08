#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main()
{
    // Выводится, если нет веб-камеры
    cv::Mat imageNoCam(200, 270, CV_8UC3, cv::Scalar(255, 0, 0));
    cv::putText(imageNoCam, "No webcam", cv::Point(85, 100), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

    // Выводится, если нет изображения с камеры
    cv::Mat imageNoImage(200, 270, CV_8UC3, cv::Scalar(255, 0, 0));
    cv::putText(imageNoImage, "No image", cv::Point(85, 100), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

    cv::VideoCapture cam(0);

    if (!cam.isOpened())
    {
        cv::imshow("No webcam", imageNoCam);
        cv::waitKey(0);
        return 0;
    }

    cv::Mat image;
    cv::Mat imageBW;

    while (true)
    {
        cam.read(image);

        if (!image.empty())
        {
            cv::cvtColor(image, imageBW, cv::COLOR_BGR2GRAY);
            cv::imshow("Webcam", imageBW);
        }
        else
        {
            cv::imshow("Webcam", imageNoImage);
        }

        // Если нажата ESC, завершаем выполнение
        if (cv::waitKey(20) == 0x1B)
        {
            break;
        }
    }

    return 0;
}