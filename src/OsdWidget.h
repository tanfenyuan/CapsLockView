#pragma once

/**
* @file    OsdWidget.h
* @author  tanfenyuan@outlook.com
* @date    2026-10-06
*/


#include <QWidget>

class QSvgWidget;

class OsdWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsdWidget(QWidget* parent = nullptr);

    void showLockState(bool enabled);

private:
    QSvgWidget* icon_widget_;
};
