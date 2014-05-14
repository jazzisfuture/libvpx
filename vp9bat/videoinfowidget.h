#ifndef VIDEOINFOWIDGET_H
#define VIDEOINFOWIDGET_H

#include <QWidget>
#include "vp9bat.h"

class VideoInfoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit VideoInfoWidget(QWidget *parent = 0);





  void setMode(VideoInfoMode mode);
signals:

public slots:



};

#endif // VIDEOINFOWIDGET_H
