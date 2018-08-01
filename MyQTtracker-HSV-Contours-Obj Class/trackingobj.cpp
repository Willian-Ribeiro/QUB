#include "trackingobj.h"

TrackingObj::TrackingObj()
{
    setHSVMax(185,255,255);
    setHSVMin(100,175,235);
    setName("Um Tipo de Objeto");
    this->matProcessedForItsThreshold = NULL;
    setTailColour(cv::Point3i(255,0,0));
    setOuterCircleColour(cv::Point3i(255,0,0));
    setInnerCircleColour(cv::Point3i(0,255,0));
    for(int i = 0; i < sizeObjectsTracked; i++)
    {
        std::vector<cv::Point2i> temp;
        this->trackedObjs[i] = temp;
    }
}

TrackingObj::TrackingObj(std::string name)
{
    if(name == "RED")
    {
        setHSVMax(188,167,255);
        setHSVMin(150,46,230);
        setName(name);
        this->matProcessedForItsThreshold = NULL;
        setTailColour(cv::Point3i(0,0,255));
        setOuterCircleColour(cv::Point3i(0,0,255));
        setInnerCircleColour(cv::Point3i(0,255,0));
        for(int i = 0; i < sizeObjectsTracked; i++)
        {
            std::vector<cv::Point2i> temp;
            this->trackedObjs[i] = temp;
        }
    }
    else
    {
        if(name == "GREEN")
        {
            setHSVMax(79,255,255);
            setHSVMin(50,150,230);
            setName(name);
            this->matProcessedForItsThreshold = NULL;
            setTailColour(cv::Point3i(135,255,160));
            setOuterCircleColour(cv::Point3i(0,255,0));
            setInnerCircleColour(cv::Point3i(255,0,0));
            for(int i = 0; i < sizeObjectsTracked; i++)
            {
                std::vector<cv::Point2i> temp;
                this->trackedObjs[i] = temp;
            }
        }
        else
        {
            setHSVMax(185,255,255);
            setHSVMin(100,175,235);
            setName("Objeto nao nomeado");
            this->matProcessedForItsThreshold = NULL;
            setTailColour(cv::Point3i(255,0,0));
            setOuterCircleColour(cv::Point3i(255,0,0));
            setInnerCircleColour(cv::Point3i(0,255,0));
            for(int i = 0; i < sizeObjectsTracked; i++)
            {
                std::vector<cv::Point2i> temp;
                this->trackedObjs[i] = temp;
            }
        }
    }
}

TrackingObj::~TrackingObj()
{

}

void TrackingObj::setHSVMax(int h, int s, int v)
{
    this->hSVMaxValue = cv::Point3i(h,s,v);
}

void TrackingObj::setHSVMin(int h, int s, int v)
{
    this->hSVMinValue = cv::Point3i(h,s,v);
}

cv::Point3i TrackingObj::getHSVMax()
{
    return this->hSVMaxValue;
}

cv::Point3i TrackingObj::getHSVMin()
{
    return this->hSVMinValue;
}

void TrackingObj::setName(std::string name)
{
    this->objName = name;
}

void TrackingObj::setMatProcessedForItsThreshold(cv::Mat mat)
{
    (this->matProcessedForItsThreshold) = mat;
}

cv::Mat TrackingObj::getMatProcessedForItsThreshold()
{
    return this->matProcessedForItsThreshold;
}

std::vector<Obj> &TrackingObj::getObjs()
{
    return this->objects;
}

void TrackingObj::clearObjType()
{
    this->matProcessedForItsThreshold.release();
    this->objects.clear();
}

void TrackingObj::storeObjPath(bool objFound, int time, cv::Point2i pt)
{
    if(objFound == false) return;

    this->trackedObjs[time].push_back(pt);
}

std::vector<cv::Point2i> *TrackingObj::getStoredObjPath(int time)
{
    return &(this->trackedObjs[time]);
}

std::string TrackingObj::getName()
{
    return this->objName;
}

cv::Point3i TrackingObj::getTailColour()
{
    return this->tailColour;
}

cv::Point3i TrackingObj::getOuterCircleColour()
{
    return this->outerCircleColour;
}

cv::Point3i TrackingObj::getInnerCircleColour()
{
    return this->innerCircleColour;
}

void TrackingObj::setTailColour(cv::Point3i cor)
{
    this->tailColour = cor;
}

void TrackingObj::setOuterCircleColour(cv::Point3i cor)
{
    this->outerCircleColour = cor;
}

void TrackingObj::setInnerCircleColour(cv::Point3i cor)
{
    this->innerCircleColour = cor;
}

