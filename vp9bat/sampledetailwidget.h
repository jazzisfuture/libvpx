#ifndef SAMPLEDETAILWIDGET_H
#define SAMPLEDETAILWIDGET_H

#include <QWidget>

class SampleDetailWidget : public QWidget
{
  Q_OBJECT
public:
  explicit SampleDetailWidget(QWidget *parent = 0);

signals:

public slots:
  void setSampleType(int sel);


};

#endif // SAMPLEDETAILWIDGET_H
