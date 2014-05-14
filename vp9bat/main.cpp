#include <QtWidgets>
#include "mainwindow.h"
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationName("Google");
  QCoreApplication::setOrganizationDomain("google.com");
  QCoreApplication::setApplicationName("vp9bat");
  //QApplication::setApplicationDisplayName("VP9 bitstream analysis tool");


  MainWindow w;
  QSettings settings;
  bool fs = settings.value("fullscreen",false).toBool();
  if(fs) {
    w.showMaximized();
  } else {
    w.showNormal();
  }
  return a.exec();
}
