#ifndef MYSLIDER_H
#define MYSLIDER_H


#include <QSlider>
#include <QMouseEvent>
#include <QDebug>

class MySlider : public QSlider {
    Q_OBJECT

public:
    MySlider(QWidget *parent = nullptr) : QSlider(parent) {
        isMousePressed = false;
    }

public:
    void mousePressEvent(QMouseEvent *event) override {
        isMousePressed = true;
//        qDebug() << "Slider pressed";
        QSlider::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        isMousePressed = false;
//        qDebug() << "Slider released";
        QSlider::mouseReleaseEvent(event);
    }

    bool isSliderSelected() const {
        return isMousePressed;
    }

private:
    bool isMousePressed;
};


#endif // MYSLIDER_H
