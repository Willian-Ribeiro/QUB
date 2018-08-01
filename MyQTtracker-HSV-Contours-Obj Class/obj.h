#ifndef OBJ_H
#define OBJ_H

#include <QtCore>
#include <opencv2/core/core.hpp>

class Obj
{
public:
    Obj();
    Obj(cv::Point2i pos);
    Obj(cv::Point2i pos, int area);

    void setArea(int area);
    int getArea();

    void setPos(cv::Point2i pos);
    cv::Point2i getPos();

    void setCrop(cv::Mat crop);
    cv::Mat getCrop();

    void setColor(QString color);
    QString getColor();

private:

    cv::Point2i pos;

    int area;

    cv::Mat crop;// part of original image which shows the object

    QString color;

};

#endif // OBJ_H
