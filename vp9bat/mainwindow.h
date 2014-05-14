#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "videoinfowidget.h"
#include "txwidget.h"
#include "statspiewidget.h"
#include "pixelvalueswidget.h"
#include "filmstripwidget.h"
#include "sampledetailwidget.h"
#include "vp9bat.h"
#include "bitstream.h"

#define MAX_RECENT 9

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = 0);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void saveSettings();
  void loadSettings();
  void buildUI();
  void updateRecentFiles();
  void setMode(VideoInfoMode mode, bool updateToolbar = false);

  Bitstream *bs;


  VideoInfoMode mode;
  QList<int> savedSplitSizesTop;
  QList<int> savedSplitSizesMain;
  QList<int> savedSplitSizesVideoBytes;
  QString pwd;

  // ---------------- UI elements --------------------

  //QFileDialog *fileDialog;

  // Set of tools
  QAction *dummyRecentAction;
  QToolBar *mainToolbar;
  QMenu *openFileMenu;
  QAction *clearRecentAction;
  QAction *recentFilesActions[MAX_RECENT];

  QAction *modeActions[NUM_MODES];

  QMenu *yuvDiffMenu;
  QAction *clearRecentYuvAction;
  QAction *closeYuvAction;
  QAction *yuvPlanarAction;
  QAction *yuvNV12Action;
  QAction *yuvCropAction;
  QAction *yuvHasHiddenAction;
  QAction *yuvCheckForChangesAction;
  QAction *yuvSetOffsetAction;

  QAction *decodedYuvAction;
  QAction *debugYuvAction;
  QAction *diffYuvAction;


  QAction *resetZoomAction;
  QAction *zoomInAction;
  QAction *zoomOutAction;
  QAction *fullscreenAction;
  QAction *viewNoneAction;
  QAction *viewYUVAction;
  QAction *viewYAction;
  QAction *viewUAction;
  QAction *viewVAction;
  QAction *hexAction;
  QAction *decAction;
  QAction *overlayAction;

  QAction *thumbsAction;
  QAction *barsAction;
  QSpinBox *frameSpinBox;
  QAction *extractAction;



  // Main UI organizing containers

  QSplitter *topSplit;
  QSplitter *mainSplit;
  QSplitter *samplesSelectionSplit;
  QSplitter *videoBytesSplit;


  // Filmstrip
  FilmstripWidget *filmstripWidget;
  QScrollArea *scrollingFilmstrip;

  QComboBox *syntaxComboBox;
  // Panel showing header syntax. Compressed and uncompressed.
  QTableView *headerSyntaxTableView;
  // Panel showing all syntax of the selected leaf
  QTableView *partitionSyntaxTableView;
  // Panel containing these two: A way to pick a tx block, and show its syntax
  TxWidget *txWidget;
  QTableView *txSyntaxTableView;

  // Tree of all probabilities. Could include a way to pick one of the 4 models.
  QTreeView *probablityTreeView;
  // Tree of all context counters
  QTreeView *counterTreeView;

  // Details of the current selected block. Size, mode, bits, etc.
  QTableView *selectionTableView;

  QComboBox *samplesComboBox;
  SampleDetailWidget *sampleDetailWidget;

  // Show raw byte values in hex of the selection.
  QTextEdit *rawBytesText;

  // Stats panel
  QTextEdit *statsText;
  QComboBox *statsComboBox;
  QComboBox *statsTypeComboBox;
  StatsPieWidget *statsPieWidget;

  // Decoder state
  QTableView *referencePoolTableView; // indicate which is LAST, GOLDEN, ALTREF
  QTableView *loopFilterDeltasTableView;
  QTableView *contextsTableView;
  QTableView *segmentDataTableView;

  // Decode progress
  QProgressBar *progressBar;





  VideoInfoWidget *videoWidget;
  PixelValuesWidget *pixelValuesWidget;

signals:

private slots:
  void setFullscreen(bool fs);
  void loadFile();
  void clearRecentFiles();
  void setMode(QAction *action);

};

#endif // MAINWINDOW_H
