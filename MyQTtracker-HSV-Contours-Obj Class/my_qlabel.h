#ifndef MY_QLABEL_H
#define MY_QLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <qdebug.h>
#include <qevent.h>

class my_qlabel : public QLabel
{
    Q_OBJECT
public:
    explicit my_qlabel(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent *ev);

    float x, y;

signals:

    void mousePressed();

public slots:

};

#endif // MY_QLABEL_H
