#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QtSerialPort/QtSerialPort>
#include <qelapsedtimer.h>

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>

#include "trackingobj.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

public slots:
    void processFrameAndUpdateGUI();

    void sendCoordinates();

    void recordPathButton(); // save the coordinates acquired in a qstring

    void eraseRecordedPath(); // erase the qstring

    void saveRecordedPath(); // save the qstring in a text file

private slots:
    void on_btnPauseorResume_clicked();

    void mousePressed();

    void setVHSThreshold();

    void hMax(int i);
    void sMax(int i);
    void vMax(int i);
    void hMin(int i);
    void sMin(int i);
    void vMin(int i);

    void nextTrackingObj();// also displays which processed image will be shown
    void previousTrackingObj();
    void calibrateImg(); // make see the calibrating black and white image

    void newTypeObj();

private:
    Ui::Dialog *ui;

    cv::VideoCapture capWebCam;
    cv::Mat matOriginal;
    cv::Mat matProcessed, matProcessedVHS;

    const int FRAME_HEIGHT = 480;
    const int FRAME_WIDTH = 640;

    cv::Mat erodeElement;
    cv::Mat dilateElement;

    QImage qImgOriginal;
    QImage qImgProcessed;

    static const int sizeObjectsTracked = 20;
    cv::Point vecTrackedObjects[sizeObjectsTracked];
    int contTrackedObjects;

    QTimer *tmrTimer;
    int tmrRefresh;

    QElapsedTimer *tmrTimer1;

    QSerialPort serial;

    bool sendCoord; // indicates when to send coordinates through port

    std::string s1, s2;

    QString outputFilePath; // file to be read or written
    QString outputFile; // text inside the file

    void saveTXTFile(QString filePath, QString fileData);
    void loadTXTFile(QString filePath);

    bool recordPath; // indicates when it is to add to qstring the coordinates acquired

    //function to trckobject using contour and area
    bool trackFilteredObject(std::vector<Obj> &vecObjs, cv::Mat threshold);
    //same function,but aquires many objects
    bool Dialog::trackFilteredObjectManyObjs(std::vector<Obj> &vecObjs, cv::Mat threshold);

    //max number of objects to be detected in frame
    const int MAX_NUM_OBJECTS=50;
    //minimum and maximum object area
    const int MIN_OBJECT_AREA = 5*10;
    const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

    bool objectFound; // informs if in the actual frame a object was found

    void sendCoordinates(int x, int y, bool referenceValue);

    void processImage(cv::Mat hsvImage, TrackingObj &objs, bool calibratingMode);

    std::vector<TrackingObj*> objectTypes;

    // used to select which object to set HSV
    int whichObject;
    TrackingObj* toSetHSVObj;

    int hMaxValue, sMaxValue, vMaxValue, hMinValue, sMinValue, vMinValue;

    bool calibratingMode;

    cv::Mat crop;

    void separaLasers();
    void clearObjTypes();

    void readSerial();
};

#endif // DIALOG_H
