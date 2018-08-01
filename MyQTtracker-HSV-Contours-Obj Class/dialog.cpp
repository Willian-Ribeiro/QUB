#include "dialog.h"
#include "ui_dialog.h"

#include <QtCore>
#include "my_qlabel.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    capWebCam.open(0);
    if(capWebCam.isOpened() == false)
    {
        ui->txtXYRadius->appendPlainText("error: capwebcam not accessed seccessfully");
        return;
    }

    tmrTimer = new QTimer(this);
    tmrTimer1 = new QElapsedTimer();
    connect( tmrTimer, SIGNAL(timeout()), this, SLOT(processFrameAndUpdateGUI()) ); // rate of frame refresh

    connect(ui->pushButton_sendCoord, SIGNAL(clicked()), this, SLOT(sendCoordinates())); // manual button to start and stop calibration
    connect(ui->erase_path, SIGNAL(clicked()), this, SLOT(eraseRecordedPath()));// save to text file buttons
    connect(ui->save_path, SIGNAL(clicked()), this, SLOT(saveRecordedPath()));
    connect(ui->record_path, SIGNAL(clicked()), this, SLOT(recordPathButton()));
    connect(ui->lblOriginal, SIGNAL(mousePressed()), this, SLOT(mousePressed())); // when clicked inside the image will record coordinates

    connect(ui->horizontalSlider_1, SIGNAL(valueChanged(int)), this, SLOT(hMax(int))); // horizontal sliders to set min and max VHS filter values
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(sMax(int)));
    connect(ui->horizontalSlider_3, SIGNAL(valueChanged(int)), this, SLOT(vMax(int)));
    connect(ui->horizontalSlider_4, SIGNAL(valueChanged(int)), this, SLOT(hMin(int)));
    connect(ui->horizontalSlider_5, SIGNAL(valueChanged(int)), this, SLOT(sMin(int)));
    connect(ui->horizontalSlider_6, SIGNAL(valueChanged(int)), this, SLOT(vMin(int)));

    connect(ui->previuos, SIGNAL(clicked()), this, SLOT(previousTrackingObj())); // regulates the HSV values for each type of Object
    connect(ui->next, SIGNAL(clicked()), this, SLOT(nextTrackingObj()));
    connect(ui->saveThreshold, SIGNAL(clicked()), this, SLOT(setVHSThreshold()));
    connect(ui->calibrateImg, SIGNAL(clicked()), this, SLOT(calibrateImg()));
    connect(ui->newTypeObj, SIGNAL(clicked()), this, SLOT(newTypeObj()));

    // Setting serial Port ----------------------------------------------------------
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.setPortName("COM1");
    serial.open(QIODevice::ReadWrite);
    // --------------------------------------------------------------------------------

    hMaxValue = sMaxValue = vMaxValue = hMinValue = sMinValue = vMinValue = 0;

    recordPath = false;
    calibratingMode = false;
    sendCoord = false;
    outputFilePath = "D:/Documents/QTFiles/MyQTtracker-HSV-Contours/outputFile.txt";
    contTrackedObjects = 0;
    whichObject = 0;
    //areaLastObj = 0;


    // will create only two elements for this class
    for(int i = 0; i < 1; i++)
    {
        TrackingObj *temp;
        if(i == 0) temp = new TrackingObj("RED");
        if(i == 1) temp = new TrackingObj("GREEN");
        objectTypes.push_back(temp); // receives the address of the object
    }
    toSetHSVObj = objectTypes.at(whichObject);

    tmrRefresh = 20;
    //    tmrTimer->start(tmrRefresh); // uncomment to start by itself
}

Dialog::~Dialog()
{
    delete ui;
    serial.close();
}

void Dialog::processFrameAndUpdateGUI()
{
    capWebCam.read(matOriginal);

    contTrackedObjects++;
    contTrackedObjects = contTrackedObjects % sizeObjectsTracked;

    if(matOriginal.empty() == true) return;

    //cv::flip(matOriginal, matOriginal, 1); // (source, destination, type of inversion)

    cv::cvtColor(matOriginal, matProcessedVHS, cv::COLOR_BGR2HSV); // transform the rgb image into VHS

    //iterates for as many types of object there is
    for(std::vector<TrackingObj*>::iterator iteratorTypeObj = objectTypes.begin(); iteratorTypeObj != objectTypes.end(); iteratorTypeObj++)
    {
        //procces the image to have only black and white pixels according to the min and max HSV values for the object
        processImage(matProcessedVHS, **iteratorTypeObj, calibratingMode);
        //when calibrating no circle is draw, stored, or sent as message, for some reason, I can't remove this comand, or will crash
        // apparently resolved
        //if(calibratingMode == true) break;

        //reseting the recorded path after 1 cycle (20 coordinates of an object)
        std::vector<cv::Point2i> *temp = (*iteratorTypeObj)->getStoredObjPath(contTrackedObjects);
        temp->clear();

        //if there isn't a processed image for this object type, must skip for the next object type
        if((*iteratorTypeObj)->getMatProcessedForItsThreshold().empty() == true) continue;

        // calculates the x and y position of the biggest area found in matProcessed
        objectFound = trackFilteredObjectManyObjs((*iteratorTypeObj)->getObjs(), (*iteratorTypeObj)->getMatProcessedForItsThreshold());

        //iterates for each object of this type
        std::vector<Obj>::iterator objIterator = ((*iteratorTypeObj)->getObjs()).begin();
        for(objIterator; objIterator != ((*iteratorTypeObj)->getObjs()).end(); objIterator++)
        {
            //record the value of last 20 objects
            (*iteratorTypeObj)->storeObjPath(objectFound, contTrackedObjects, (*objIterator).getPos()); // list of last 20 object's position

            //write on QString the pos of each detected Object, to eventually be written on a text file
            if(recordPath == true) outputFile += QString::number((*objIterator).getPos().x) + QString(",") + QString::number((*objIterator).getPos().y) + QString(" ");

            // send to microcontroller the coordinates of the object
            //if(sendCoord == true) sendCoordinates((*objIterator).getPos().x, (*objIterator).getPos().y, false);// only send if is the right laser

//            // draw small green circle at center of detected object
//            cv::circle(matOriginal,													// draw on original image
//                       cv::Point((*objIterator).getPos().x, (*objIterator).getPos().y),		// center point of circle
//                       3,															// radius of circle in pixels
//                       cv::Scalar((*iteratorTypeObj)->getInnerCircleColour().x,(*iteratorTypeObj)->getInnerCircleColour().y,(*iteratorTypeObj)->getInnerCircleColour().z),// draw pure green (remember, its BGR, not RGB)
//                       CV_FILLED);													// thickness, fill in the circle

//            // draw red circle around the detected object
//            cv::circle(matOriginal,													// draw on original image
//                       cv::Point((*objIterator).getPos().x, (*objIterator).getPos().y),		// center point of circle
//                       20,                     						// radius of circle in pixels
//                       cv::Scalar((*iteratorTypeObj)->getOuterCircleColour().x,(*iteratorTypeObj)->getOuterCircleColour().y,(*iteratorTypeObj)->getOuterCircleColour().z),// draw pure red (remember, its BGR, not RGB)
//                       3);															// thickness of circle in pixels

//            //writes the positionof the tracked object on screen
//            s1 = std::to_string((*objIterator).getPos().x);// preparing coordinates to be written
//            s2 = std::to_string((*objIterator).getPos().y);

//            cv::putText(matOriginal, s1+ ", " + s2, cv::Point( 10 + (*objIterator).getPos().x, 10 + (*objIterator).getPos().y),
//                        cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0,0,255), 1, 8, false); // write on screen

        }
    }

    separaLasers();
    bool cropEmpty = crop.empty();
    if(!cropEmpty)
    {
        //cv::imshow("cropped image", crop);
        crop.release();
    }

    clearObjTypes(); // releases all the position data from tracking obj and prints the tail

    cv::cvtColor(matOriginal, matOriginal, CV_BGR2RGB); // changes the format from cv (Blue Green Red) to QT's format (Red Green Blue)

    QImage qimgOriginal((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, (int)matOriginal.step, QImage::Format_RGB888);
    QImage qimgProcessed((uchar*)matProcessed.data, matProcessed.cols, matProcessed.rows, (int)matProcessed.step, QImage::Format_Indexed8);

    ui->lblOriginal->setPixmap(QPixmap::fromImage(qimgOriginal));
    ui->lblProcessed->setPixmap(QPixmap::fromImage(qimgProcessed));

    // gets data from serial
    readSerial();

    // show how much time is required to process each frame
    ui->lblFPS->setText(QString::number(1000/(tmrTimer1->restart())));
}

void Dialog::on_btnPauseorResume_clicked()
{
    if(tmrTimer->isActive() == true)
    {
        tmrTimer->stop();
        ui->btnPauseorResume->setText("Resume");
    }
    else
    {
        tmrTimer->start(tmrRefresh);
        ui->btnPauseorResume->setText("Pause");
    }
}

void Dialog::mousePressed()
{
    if(sendCoord == false) return;
    int xProv = ui->lblOriginal->x;
    int yProv = ui->lblOriginal->y;
    ui->txtMouseClickHistory->appendPlainText(QString("    x = %1      ---      y = %2").arg(xProv) .arg(yProv));

    sendCoordinates(xProv, yProv, true);// transmit reference values
}

void Dialog::hMax(int i)
{
    hMaxValue = i*255/99;
    ui->textMBed->appendPlainText(QString("max h value: ") + QString::number(hMaxValue));
}

void Dialog::sMax(int i)
{
    sMaxValue = i*255/99;
    ui->textMBed->appendPlainText(QString("max s value: ") + QString::number(sMaxValue));
}

void Dialog::vMax(int i)
{
    vMaxValue = i*255/99;
    ui->textMBed->appendPlainText(QString("max v value: ") + QString::number(vMaxValue));
}

void Dialog::hMin(int i)
{
    hMinValue = i*255/99;
    ui->textMBed->appendPlainText(QString("min h value: ") + QString::number(hMinValue));
}

void Dialog::sMin(int i)
{
    sMinValue = i*255/99;
    ui->textMBed->appendPlainText(QString("min s value: ") + QString::number(sMinValue));
}

void Dialog::vMin(int i)
{
    vMinValue = i*255/99;
    ui->textMBed->appendPlainText(QString("min v value: ") + QString::number(vMinValue));
}

// only set a new calibration value if in calibrating mode
void Dialog::setVHSThreshold()
{
    if(calibratingMode == false) return;
    toSetHSVObj->setHSVMax(hMaxValue, sMaxValue, vMaxValue);
    toSetHSVObj->setHSVMin(hMinValue, sMinValue, vMinValue);
    toSetHSVObj->setName(ui->nameTypeObj->toPlainText().toStdString());
}

void Dialog::nextTrackingObj()
{
    whichObject = (whichObject+1) % objectTypes.size();
    toSetHSVObj = objectTypes.at(whichObject);
    ui->nameTypeObj->clear();
    ui->nameTypeObj->appendPlainText(QString::fromStdString(toSetHSVObj->getName()) );
}

void Dialog::previousTrackingObj()
{
    whichObject--;
    if(whichObject < 0) whichObject = objectTypes.size() - 1;
    toSetHSVObj = objectTypes.at(whichObject);

    ui->nameTypeObj->clear();
    ui->nameTypeObj->appendPlainText(QString::fromStdString(toSetHSVObj->getName()) );
}

void Dialog::calibrateImg()
{
    if(calibratingMode == true)
    {
        calibratingMode = false;
        ui->calibrateImg->setText(QString("Calibrate Img"));
    }
    else
    {
        calibratingMode = true;
        ui->calibrateImg->setText(QString("Back"));
    }
}

void Dialog::newTypeObj()
{
    TrackingObj *temp;
    std::string nome = "Obj " + std::to_string(objectTypes.size());
    temp = new TrackingObj();
    temp->setName(nome);
    objectTypes.push_back(temp); // receives the address of the object
}

void Dialog::readSerial()
{
    if( (serial.bytesAvailable() > 0) || (serial.waitForReadyRead(10)) )
    {
        QByteArray frase;
        frase = serial.readAll();
        ui->textMBed->appendPlainText(frase);
    }
}

void Dialog::sendCoordinates()
{
    if(sendCoord == false)
    {
        sendCoord = true;
        ui->pushButton_sendCoord->setText("Stop Sending Coordinates");
    }
    else
    {
        sendCoord = false;
        ui->pushButton_sendCoord->setText("Keep Sending Coordinates");
    }
}

void Dialog::saveTXTFile(QString filePath, QString fileData)
{
    QFile file(filePath);

    if(!file.open(QFile::WriteOnly | QFile::Text) )
    {
        qDebug() << "Could not openfile for writting";
        return;
    }

    QTextStream out(&file);
    out << fileData ;

    file.flush(); // after putting data into a stream, it makes sure everything got written down to disc
    file.close(); // close the file after using it
}

void Dialog::loadTXTFile(QString filePath)
{
    QFile file(filePath);

    if(!file.open(QFile::ReadOnly | QFile::Text) )
    {
        qDebug() << "Could not openfile for reading";
        return;
    }

    QTextStream in(&file);
    QString myText = in.readAll();

    qDebug() << myText;

    file.close(); // close the file after using it
}

void Dialog::recordPathButton()
{
    if(recordPath == false)
    {
        recordPath = true;
        ui->record_path->setText("Stop Recording Path");
    }
    else
    {
        recordPath = false;
        ui->record_path->setText("Keep Recording Path");
    }
}

void Dialog::eraseRecordedPath()
{
    outputFile.clear();
    recordPath = false;
    ui->record_path->setText("Record Path");
}

void Dialog::saveRecordedPath()
{
    saveTXTFile(outputFilePath, outputFile);
}

// return the biggest area found, hopefully will be the object
bool Dialog::trackFilteredObject(std::vector<Obj> &vecObjs, cv::Mat threshold)
{
    cv::Mat temp;
    int x,y;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    cv::findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0)
    {
        int numObjects = (int)hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS)
        {
            for (int index = 0; index >= 0; index = hierarchy[index][0])
            {
                cv::Moments moment = cv::moments((cv::Mat)contours[index]);
                double area = moment.m00;

                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.

                //if I always compare with the refArea I'll always have 1 or none objects
                if((area>MIN_OBJECT_AREA) && (area<MAX_OBJECT_AREA) && (area>refArea))
                {
                    x = moment.m10/area;
                    y = moment.m01/area;
                    objectFound = true;
                    refArea = area;
                }else objectFound = (objectFound || false);
            }
        }
    }
    //areaLastObj = refArea;
    Obj *tempObj = new Obj(cv::Point2i(x,y), refArea);
    vecObjs.push_back(*tempObj);
    return objectFound;
}

// return the biggest area found, hopefully will be the object
bool Dialog::trackFilteredObjectManyObjs(std::vector<Obj> &vecObjs, cv::Mat threshold)
{
    cv::Mat temp;
    int x,y;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    cv::findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    bool objectFound = false;
    if (hierarchy.size() > 0)
    {
        int numObjects = (int)hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS)
        {
            for (int index = 0; index >= 0; index = hierarchy[index][0])
            {
                cv::Moments moment = cv::moments((cv::Mat)contours[index]);
                double area = moment.m00;

                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //iteration and compare it to the area in the next iteration.

                //if I always compare with the refArea I'll always have 1 or none objects
                if((area>MIN_OBJECT_AREA) && (area<MAX_OBJECT_AREA))
                {
                    x = moment.m10/area;
                    y = moment.m01/area;
                    objectFound = true;
                    Obj *tempObj = new Obj(cv::Point2i(x,y), area);
                    vecObjs.push_back(*tempObj);
                }else objectFound = (objectFound || false);
            }
        }
    }
    return objectFound;
}

void Dialog::sendCoordinates(int x, int y, bool referenceValue)
{
    char const * pxu;
    char const * pxl;
    char const * pyu;
    char const * pyl;

    ui->txtXYRadius->appendPlainText(QString("X Value: ")+QString::number(x)+
                                     QString("  -------------------  Y Value: ")+QString::number(y));

    //qWarning() << "valor de x: " << x << "bits rotacionados a direita: " << (sizeof(x)*8*3/4);

    //int here have 4 bytes
    int xTemp = x >> (sizeof(x)*8*1/4);
    x -= xTemp << (sizeof(x)*8*1/4);
    if(xTemp == 0) xTemp = 'z';
    if(x == 0) x = 1;

    int yTemp = y >> (sizeof(y)*8*1/4);
    y -= yTemp << (sizeof(y)*8*1/4);
    if(yTemp == 0) yTemp = 'z';
    if(y == 0) y = 1;

    //qWarning() << "valor de xUpper: " << xTemp << "valor de xLower: " << x;

    pxu = reinterpret_cast<char const *>(&xTemp); // even if the value of x cant be stored on p1 it will store
    pxl = reinterpret_cast<char const *>(&x);
    pyu = reinterpret_cast<char const *>(&yTemp);
    pyl = reinterpret_cast<char const *>(&y);

    //qWarning() << "valor de xUpper: " << pxu << "valor de xLower: " << pxl;
    //qWarning() << "valor de yUpper: " << pyu << "valor de yLower: " << pyl;

    if(referenceValue == true) serial.write("r"); // signal to transmit reference values
    else serial.write("s"); // signal to start transmitting position
    serial.write(pxu);
    serial.write(pxl);
    serial.write(pyu);
    serial.write(pyl);
}

//searches for the object threshold
void Dialog::processImage(cv::Mat hsvImage, TrackingObj &objs, bool calibratingMode)
{
    if(calibratingMode == true)
    {
        cv::Mat temp;
        cv::inRange(hsvImage,				// funcion input
                    cv::Scalar(hMinValue , sMinValue, vMinValue),	// min filtering value (if greater than or equal to this) (in BGR format)
                    cv::Scalar(hMaxValue, sMaxValue, vMaxValue), 	// max filtering value (and if less than this) (in BGR format)
                    temp);				// function output

        erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
        dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8,8));

        //cv::erode(temp,temp, erodeElement);
        cv::erode(temp,temp, erodeElement);

        cv::dilate(temp,temp, dilateElement);
        cv::dilate(temp,temp, dilateElement);

        matProcessed = temp.clone();
        return;

    }
    else
    {
        if(objs.getMatProcessedForItsThreshold().empty() != true) return;

        cv::Mat temp;

        cv::inRange(hsvImage,				// funcion input
                    cv::Scalar((objs.getHSVMin().x) , (objs.getHSVMin()).y, (objs.getHSVMin()).z),	// min filtering value (if greater than or equal to this) (in BGR format)
                    cv::Scalar((objs.getHSVMax()).x, (objs.getHSVMax()).y, (objs.getHSVMax()).z), 	// max filtering value (and if less than this) (in BGR format)
                    temp);				// function output

        erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
        dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8,8));

        //cv::erode(temp,temp, erodeElement);
        cv::erode(temp,temp, erodeElement);

        cv::dilate(temp,temp, dilateElement);
        cv::dilate(temp,temp, dilateElement);
        objs.setMatProcessedForItsThreshold(temp);// set the processed image for a type of obj

        if(objs.getName() == toSetHSVObj->getName()) temp.copyTo(matProcessed); // make see the processed image of this object
    }
}

void Dialog::separaLasers()
{
    for(std::vector<TrackingObj*>::iterator iteratorTypeObj = objectTypes.begin(); iteratorTypeObj != objectTypes.end(); iteratorTypeObj++)
    {

        std::vector<Obj>::iterator objIterator = ((*iteratorTypeObj)->getObjs()).begin();
        for(objIterator; objIterator != ((*iteratorTypeObj)->getObjs()).end(); objIterator++)
        {
            if( ((*objIterator).getPos().x < 0) || ((*objIterator).getPos().y < 0) ) continue;

            // getting the value of x and y of the area of the last laser point aquired
            int side = (sqrt((*objIterator).getArea()))/2; // half of the square root of area, supposing it's a square

            int xLesserLeftCorner = (*objIterator).getPos().x - side;
            if(xLesserLeftCorner+2*side > FRAME_WIDTH) xLesserLeftCorner = FRAME_WIDTH - 2*side;
            if(xLesserLeftCorner < 0) xLesserLeftCorner = 0;
            int yLesserLeftCorner = (*objIterator).getPos().y - side;
            if(yLesserLeftCorner+2*side > FRAME_HEIGHT) yLesserLeftCorner = FRAME_HEIGHT - 2*side;
            if(yLesserLeftCorner < 0) yLesserLeftCorner = 0;

            // this is just a header to the same matrix being used by matOriginal
            crop = matOriginal(cv::Rect( xLesserLeftCorner, yLesserLeftCorner, 2*side, 2*side ));
            crop = crop.clone();// must create a copy, otherwise the pixel counting wont work
            (*objIterator).setCrop(crop);

            unsigned int numberRed = 0;
            unsigned int numberGreen = 0;
            unsigned int numberBlue = 0;

            uchar* pixelPtr = (uchar*)crop.data;
            int cn = crop.channels();

            for(int line = 0; line < crop.rows; line++)
            {
                for(int column = 0; column < crop.cols; column++)
                {
                    uchar numberBlue1 = pixelPtr[line*crop.cols*cn + column*cn + 0]; // B
                    uchar numberGreen1 = pixelPtr[line*crop.cols*cn + column*cn + 1]; // G
                    uchar numberRed1 = pixelPtr[line*crop.cols*cn + column*cn + 2]; // R

                    // skipping white pixels
                    if( (numberRed1 > 245) && (numberBlue1 > 245) && (numberGreen1 > 245) ) continue;

                    if( (numberRed1 > numberBlue1) && (numberRed1 > numberGreen1) ) numberRed++;
                    else
                    {
                        if(numberGreen1 > numberBlue1) numberGreen++;
                        else numberBlue++;
                    }
                }
            }

            //qDebug() << numberRed << ", " << numberGreen << ", " << numberBlue;
            if( (numberRed > numberBlue) && (numberRed > numberGreen) )
            {
                cv::putText(matOriginal, "RED", cv::Point( (*objIterator).getPos().x,  (*objIterator).getPos().y),
                            cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0,0,255), 1, 8, false); // write on screen
                (*objIterator).setColor(QString("RED"));

                // red laser pos is the reference pos for the green laser
                if(sendCoord == true) sendCoordinates((*objIterator).getPos().x, (*objIterator).getPos().y, true);
            }
            else
            {
                if(numberGreen > numberBlue)
                {
                    cv::putText(matOriginal, "Green", cv::Point( (*objIterator).getPos().x, (*objIterator).getPos().y),
                                cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(0,255,0), 1, 8, false); // write on screen
                    (*objIterator).setColor(QString("GREEN"));

                    //green is the color of the controlled laser
                    if(sendCoord == true) sendCoordinates((*objIterator).getPos().x, (*objIterator).getPos().y, false);

                }
                else
                {
                    cv::putText(matOriginal, "Blue", cv::Point( (*objIterator).getPos().x, (*objIterator).getPos().y),
                                cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(255,0,0), 1, 8, false); // write on screen
                    (*objIterator).setColor(QString("BLUE"));
                }
            }
        }
    }
}

// clear vectors from trackingobj class and print the tail on matOriginal
void Dialog::clearObjTypes()
{
    // print the tail made by the last positions of the objects
    for(std::vector<TrackingObj*>::iterator itTrackObjs = objectTypes.begin(); itTrackObjs < objectTypes.end(); itTrackObjs++)
    {
//        //iterating on size of tail(time)
//        for(int i = 0; i < (*itTrackObjs)->sizeObjectsTracked; i++)
//        {
//            int v = i - contTrackedObjects;
//            if( v < 0 ) v = sizeObjectsTracked + v;

//            std::vector<cv::Point2i> *temp = (*itTrackObjs)->getStoredObjPath(i);
//            //iterating for each object found in that time
//            for(int j = 0; j < temp->size(); j++)
//            {
//                cv::circle(matOriginal,													// draw on original image
//                           temp->at(j),		// center point of circle, back gives the stored value (in this case cv::Point2i)
//                           (15*v/(sizeObjectsTracked-1)),                                   // radius of circle in pixels, they diminish size as time pass
//                           cv::Scalar((*itTrackObjs)->getTailColour().x,(*itTrackObjs)->getTailColour().y,(*itTrackObjs)->getTailColour().z),// draw pure green (remember, its BGR, not RGB)
//                           CV_FILLED);
//            }
//        }

        (*itTrackObjs)->clearObjType();
    }
}
