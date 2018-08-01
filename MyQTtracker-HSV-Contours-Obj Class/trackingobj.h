#ifndef TRACKINGOBJ_H
#define TRACKINGOBJ_H
#include <string>
#include <opencv2/core/core.hpp>
#include "obj.h"

class TrackingObj
{
public:
    TrackingObj();
    TrackingObj(std::string name);
    ~TrackingObj();

    static const int sizeObjectsTracked = 20;

    void setHSVMax(int h, int s, int v);
    void setHSVMin(int h, int s, int v);
    cv::Point3i getHSVMax();
    cv::Point3i getHSVMin();

    void setName(std::string name);
    std::string getName();

    void setMatProcessedForItsThreshold(cv::Mat mat);
    cv::Mat getMatProcessedForItsThreshold();

    std::vector<Obj>& getObjs();

    // resets the obj type processed mat and clears the positions of the objs
    void clearObjType();

    void storeObjPath(bool objFound, int time, cv::Point2i pt);
    std::vector<cv::Point2i>* getStoredObjPath(int time);

    cv::Point3i getTailColour();
    cv::Point3i getOuterCircleColour();
    cv::Point3i getInnerCircleColour();

    void setTailColour(cv::Point3i cor);
    void setOuterCircleColour(cv::Point3i cor);
    void setInnerCircleColour(cv::Point3i cor);

private:
    std::string objName;

    cv::Point3i hSVMaxValue, hSVMinValue;

    std::vector<Obj> objects;

    cv::Mat matProcessedForItsThreshold;

    std::vector<cv::Point2i> trackedObjs[sizeObjectsTracked];

    cv::Point3i tailColour, outerCircleColour, innerCircleColour;
};

#endif // TRACKINGOBJ_H
