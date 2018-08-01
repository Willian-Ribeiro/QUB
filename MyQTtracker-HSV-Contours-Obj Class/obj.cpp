#include "obj.h"

Obj::Obj(cv::Point2i pos)
{
    this->pos = pos;
}

Obj::Obj(cv::Point2i pos, int area)
{
    this->area = area;
    this->pos = pos;
}

void Obj::setArea(int area)
{
    this->area = area;
}

int Obj::getArea()
{
    return this->area;
}

void Obj::setPos(cv::Point2i pos)
{
    this->pos = pos;
}

cv::Point2i Obj::getPos()
{
    return this->pos;
}

void Obj::setCrop(cv::Mat crop)
{
    this->crop = crop;
}

cv::Mat Obj::getCrop()
{
    return this->crop;
}

void Obj::setColor(QString color)
{
    this->color = color;
}

QString Obj::getColor()
{
    return this->color;
}
