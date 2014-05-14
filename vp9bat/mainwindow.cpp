#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent) {

  mode = MODE_PRED;
  bs = NULL;

  buildUI();

  loadSettings();


}

void MainWindow::loadSettings() {
  QSettings settings;

  restoreState(settings.value("windowstate").toByteArray());
  if(settings.contains("windowgeometry")) {
    restoreGeometry(settings.value("windowgeometry").toByteArray());
  } else {
    QDesktopWidget *d = QApplication::desktop();
    QSize defSize(d->screenGeometry(0).width() * 80 / 100, d->screenGeometry(0).height() * 80 / 100);
    QPoint defPoint(d->screenGeometry(0).width() * 10 / 100, d->screenGeometry(0).height() * 10 / 100);
    resize(defSize);
    move(defPoint);
  }
  topSplit->restoreState(settings.value("split1").toByteArray());
  mainSplit->restoreState(settings.value("split2").toByteArray());
  samplesSelectionSplit->restoreState(settings.value("split3").toByteArray());
  videoBytesSplit->restoreState(settings.value("split4").toByteArray());
  setMode((VideoInfoMode)settings.value("mode", MODE_PRED).toInt(),true);
  updateRecentFiles();
}


void MainWindow::saveSettings() {
  QSettings settings;

  settings.setValue("windowstate", saveState());
  settings.setValue("windowgeometry", saveGeometry());
  settings.setValue("split1", topSplit->saveState());
  settings.setValue("split2", mainSplit->saveState());
  settings.setValue("split3", samplesSelectionSplit->saveState());
  settings.setValue("split4", videoBytesSplit->saveState());
  settings.setValue("mode", mode);
}


void MainWindow::closeEvent(QCloseEvent *event) {
  if(fullscreenAction->isChecked())
    setFullscreen(false);
  saveSettings();
  if(bs)
    delete bs;

  event->accept();
}

void MainWindow::buildUI() {

  setWindowTitle(tr("VP9 Bitstream Analyzer by Pieter Kapsenberg (sexy name pending)."));
  setUnifiedTitleAndToolBarOnMac(true);


  topSplit = new QSplitter(Qt::Vertical, this);
  mainSplit = new QSplitter(Qt::Horizontal, topSplit);
  samplesSelectionSplit = new QSplitter(Qt::Vertical, mainSplit);
  videoBytesSplit = new QSplitter(Qt::Vertical, mainSplit);

  filmstripWidget = new FilmstripWidget(topSplit);
  scrollingFilmstrip = new QScrollArea(this);
  scrollingFilmstrip->setWidget(filmstripWidget);
  scrollingFilmstrip->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollingFilmstrip->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  videoWidget = new VideoInfoWidget(videoBytesSplit);

  rawBytesText = new QTextEdit(videoBytesSplit);

  QWidget *syntaxWidget = new QWidget(mainSplit);
  QStackedWidget *syntaxStackWidget = new QStackedWidget(syntaxWidget);

  headerSyntaxTableView = new QTableView(syntaxStackWidget);

  partitionSyntaxTableView = new QTableView(syntaxStackWidget);

  QSplitter *txSplitter = new QSplitter(syntaxStackWidget);
  txSplitter->setOrientation(Qt::Vertical);
  txWidget = new TxWidget(txSplitter);
  txSplitter->addWidget(txWidget);
  txSyntaxTableView = new QTableView(txSplitter);
  txSplitter->addWidget(txSyntaxTableView);

  probablityTreeView = new QTreeView(syntaxStackWidget);

  counterTreeView = new QTreeView(syntaxStackWidget);

  QWidget *stateWidget = new QWidget(syntaxStackWidget);
  referencePoolTableView = new QTableView(stateWidget);
  segmentDataTableView = new QTableView(stateWidget);
  loopFilterDeltasTableView = new QTableView(stateWidget);
  contextsTableView = new QTableView(stateWidget);
  QGridLayout *stateGridLayout = new QGridLayout(stateWidget);
  stateGridLayout->addWidget(new QLabel(tr("Reference pool:"), stateWidget), 0, 0, 1, 2);
  stateGridLayout->addWidget(referencePoolTableView,                        1, 0, 1, 2);
  stateGridLayout->addWidget(new QLabel(tr("Segment data:"), stateWidget),   2, 0, 1, 2);
  stateGridLayout->addWidget(segmentDataTableView,                          3, 0, 1, 2);
  stateGridLayout->addWidget(new QLabel(tr("LF Deltas:"), stateWidget),      4, 0, 1, 1);
  stateGridLayout->addWidget(loopFilterDeltasTableView,                     5, 0, 1, 1);
  stateGridLayout->addWidget(new QLabel(tr("Contexts:"), stateWidget),       4, 1, 1, 1);
  stateGridLayout->addWidget(contextsTableView,                             5, 1, 1, 1);
  stateGridLayout->setSpacing(0);
  stateGridLayout->setMargin(0);
  stateWidget->setLayout(stateGridLayout);

  QWidget *statsWidget = new QWidget(syntaxStackWidget);
  statsText = new QTextEdit(statsWidget);
  statsComboBox = new QComboBox(statsWidget);
  statsTypeComboBox = new QComboBox(statsWidget);
  statsPieWidget = new StatsPieWidget(statsWidget);
  QGridLayout *statsGridLayout = new QGridLayout(statsWidget);
  statsGridLayout->addWidget(statsText,         0, 0, 1, 2);
  statsGridLayout->addWidget(statsComboBox,     1, 0, 1, 1);
  statsGridLayout->addWidget(statsTypeComboBox, 1, 1, 1, 1);
  statsGridLayout->addWidget(statsPieWidget,    2, 0, 1, 2);
  statsGridLayout->setSpacing(0);
  statsGridLayout->setMargin(0);
  statsGridLayout->setRowStretch(0, 1);
  statsGridLayout->setRowStretch(1, 0);
  statsGridLayout->setRowStretch(2, 2);
  statsWidget->setLayout(statsGridLayout);

  syntaxStackWidget->addWidget(headerSyntaxTableView);
  syntaxStackWidget->addWidget(partitionSyntaxTableView);
  syntaxStackWidget->addWidget(txSplitter);
  syntaxStackWidget->addWidget(probablityTreeView);
  syntaxStackWidget->addWidget(counterTreeView);
  syntaxStackWidget->addWidget(stateWidget);
  syntaxStackWidget->addWidget(statsWidget);

  syntaxComboBox = new QComboBox(syntaxWidget);
  syntaxComboBox->addItem(tr("Header syntax"));
  syntaxComboBox->addItem(tr("Partition syntax"));
  syntaxComboBox->addItem(tr("Coefficient syntax"));
  syntaxComboBox->addItem(tr("Probabilities"));
  syntaxComboBox->addItem(tr("Context counters"));
  syntaxComboBox->addItem(tr("Decoder state"));
  syntaxComboBox->addItem(tr("Statistics"));

  QVBoxLayout *syntaxLayout = new QVBoxLayout(syntaxWidget);
  syntaxLayout->addWidget(syntaxComboBox);
  syntaxLayout->addWidget(syntaxStackWidget);
  syntaxLayout->setMargin(0);
  syntaxLayout->setSpacing(0);
  syntaxWidget->setLayout(syntaxLayout);
  connect(syntaxComboBox, SIGNAL(activated(int)), syntaxStackWidget, SLOT(setCurrentIndex(int)));

  selectionTableView = new QTableView(samplesSelectionSplit);

  QWidget *samplesWidget = new QWidget(samplesSelectionSplit);
  samplesComboBox = new QComboBox(samplesWidget);
  sampleDetailWidget = new SampleDetailWidget(samplesWidget);
  QVBoxLayout *samplesLayout = new QVBoxLayout(samplesWidget);
  samplesLayout->addWidget(samplesComboBox);
  samplesLayout->addWidget(sampleDetailWidget);
  samplesLayout->setMargin(0);
  samplesLayout->setSpacing(0);
  samplesWidget->setLayout(samplesLayout);
  connect(samplesComboBox, SIGNAL(activated(int)), sampleDetailWidget, SLOT(setSampleType(int)));




  videoBytesSplit->addWidget(videoWidget);
  videoBytesSplit->addWidget(rawBytesText);
  videoBytesSplit->setChildrenCollapsible(true);
  videoBytesSplit->setStretchFactor(0, 20);
  videoBytesSplit->setStretchFactor(1, 1);

  samplesSelectionSplit->addWidget(selectionTableView);
  samplesSelectionSplit->addWidget(samplesWidget);
  samplesSelectionSplit->setStretchFactor(0, 1);
  samplesSelectionSplit->setStretchFactor(1, 4);

  mainSplit->addWidget(videoBytesSplit);
  mainSplit->addWidget(samplesSelectionSplit);
  mainSplit->addWidget(syntaxWidget);
  mainSplit->setChildrenCollapsible(true);
  mainSplit->setStretchFactor(0, 5);
  mainSplit->setStretchFactor(1, 2);
  mainSplit->setStretchFactor(2, 1);

  topSplit->addWidget(scrollingFilmstrip);
  topSplit->addWidget(mainSplit);
  topSplit->setChildrenCollapsible(true);
  topSplit->setStretchFactor(0, 1);
  topSplit->setStretchFactor(1, 10);

  setCentralWidget(topSplit);




  // ----------- Tool bar ----------------
  //fileDialog = new QFileDialog(this);
  dummyRecentAction = new QAction(tr("Recent files appear here"), this);
  dummyRecentAction->setEnabled(false);
  mainToolbar = new QToolBar(this);
  mainToolbar->setObjectName("toolbar");
  openFileMenu = new QMenu(tr("Open"), mainToolbar);
  clearRecentAction = new QAction(tr("Clear recent files"), openFileMenu);
  clearRecentAction->setEnabled(false);
  connect(clearRecentAction, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
  openFileMenu->addAction(clearRecentAction);
  openFileMenu->addSeparator();
  openFileMenu->addAction(dummyRecentAction);
  connect(openFileMenu->menuAction(), SIGNAL(triggered()), this, SLOT(loadFile()));
  for(int i = 0; i < MAX_RECENT; i++) {
    recentFilesActions[i] = new QAction(this);
    recentFilesActions[i]->setVisible(false);
    connect(recentFilesActions[i], SIGNAL(triggered()), this, SLOT(loadFile()));
    openFileMenu->addAction(recentFilesActions[i]);
  }


  thumbsAction = new QAction("Thumbs", mainToolbar);
  thumbsAction->setToolTip(tr("Thumbnails"));
  thumbsAction->setCheckable(true);
  thumbsAction->setChecked(true);
  barsAction = new QAction("Bars", mainToolbar);
  barsAction->setCheckable(true);
  barsAction->setToolTip(tr("Bar graph style"));
  QActionGroup *filmstripStyleActionGroup = new QActionGroup(mainToolbar);
  filmstripStyleActionGroup->addAction(thumbsAction);
  filmstripStyleActionGroup->addAction(barsAction);
  frameSpinBox = new QSpinBox(mainToolbar);
  extractAction = new QAction(tr("Extract"), mainToolbar);
  extractAction->setToolTip("Write out the smallest possible IVF bitstream containing this frame.");

  modeActions[MODE_PRED] = new QAction("Pred", mainToolbar);
  modeActions[MODE_PRED]->setCheckable(true);
  modeActions[MODE_PRED]->setToolTip(tr("Prediction modes"));
  modeActions[MODE_PRED]->setChecked(true); // default value
  modeActions[MODE_PRED]->setData(MODE_PRED);
  modeActions[MODE_RES] = new QAction("Res", mainToolbar);
  modeActions[MODE_RES]->setCheckable(true);
  modeActions[MODE_RES]->setToolTip(tr("Residual signal structure"));
  modeActions[MODE_RES]->setData(MODE_RES);
  modeActions[MODE_RECON] = new QAction("Recon", mainToolbar);
  modeActions[MODE_RECON]->setCheckable(true);
  modeActions[MODE_RECON]->setToolTip(tr("Reconstructed samples"));
  modeActions[MODE_RECON]->setData(MODE_RECON);
  modeActions[MODE_LF] = new QAction("LF", mainToolbar);
  modeActions[MODE_LF]->setCheckable(true);
  modeActions[MODE_LF]->setToolTip(tr("Loop filter process"));
  modeActions[MODE_LF]->setData(MODE_LF);
  modeActions[MODE_YUV] = new QAction("YUV", mainToolbar);
  modeActions[MODE_YUV]->setCheckable(true);
  modeActions[MODE_YUV]->setToolTip(tr("Final decoded image / YUVDiff"));
  modeActions[MODE_YUV]->setData(MODE_YUV);
  modeActions[MODE_HEAT] = new QAction("Heat", mainToolbar);
  modeActions[MODE_HEAT]->setCheckable(true);
  modeActions[MODE_HEAT]->setToolTip(tr("Heat map, bits by area"));
  modeActions[MODE_HEAT]->setData(MODE_HEAT);
  modeActions[MODE_EFF] = new QAction("Eff", mainToolbar);
  modeActions[MODE_EFF]->setCheckable(true);
  modeActions[MODE_EFF]->setToolTip(tr("Efficiency map, bools per bit by area"));
  modeActions[MODE_EFF]->setData(MODE_EFF);
  modeActions[MODE_PSNR] = new QAction("PSNR", mainToolbar);
  modeActions[MODE_PSNR]->setCheckable(true);
  modeActions[MODE_PSNR]->setToolTip(tr("PSNR map, quality by area"));
  modeActions[MODE_PSNR]->setEnabled(false);
  modeActions[MODE_PSNR]->setData(MODE_PSNR);
  QActionGroup *modeActionGroup = new QActionGroup(mainToolbar);
  for(int i=0;i<NUM_MODES;i++)
    modeActionGroup->addAction(modeActions[i]);
  connect(modeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(setMode(QAction*)));


  yuvDiffMenu = new QMenu(tr("YUVDiff"), mainToolbar);
  yuvDiffMenu->setToolTip("Load an external YUV file to compare with");
  closeYuvAction = new QAction(tr("Close YUV file"), yuvDiffMenu);
  closeYuvAction->setEnabled(false);
  clearRecentYuvAction = new QAction(tr("Clear recent files"), yuvDiffMenu);
  clearRecentYuvAction->setEnabled(false);
  yuvPlanarAction = new QAction(tr("4:2:0 Planar"), yuvDiffMenu);
  yuvPlanarAction->setCheckable(true);
  yuvPlanarAction->setChecked(true);
  yuvNV12Action = new QAction(tr("4:2:0 NV12 (chroma interleaved)"), yuvDiffMenu);
  yuvNV12Action->setCheckable(true);
  QActionGroup *yuvFormatGroup = new QActionGroup(yuvDiffMenu);
  yuvFormatGroup->addAction(yuvPlanarAction);
  yuvFormatGroup->addAction(yuvNV12Action);
  yuvHasHiddenAction = new QAction(tr("YUV file contains hidden frames"), yuvDiffMenu);
  yuvHasHiddenAction->setCheckable(true);
  yuvCropAction = new QAction(tr("YUV file is cropped to display size"), yuvDiffMenu);
  yuvCropAction->setCheckable(true);
  yuvCropAction->setChecked(true);
  yuvCheckForChangesAction = new QAction(tr("Check for changes"), yuvDiffMenu);
  yuvCheckForChangesAction->setCheckable(true);
  yuvCheckForChangesAction->setChecked(true);
  yuvSetOffsetAction = new QAction(tr("Set frame offset..."), yuvDiffMenu);


  yuvDiffMenu->addAction(closeYuvAction);
  yuvDiffMenu->addAction(clearRecentYuvAction);
  yuvDiffMenu->addSeparator();
  yuvDiffMenu->addAction(dummyRecentAction);
  yuvDiffMenu->addSeparator();
  yuvDiffMenu->addAction(yuvPlanarAction);
  yuvDiffMenu->addAction(yuvNV12Action);
  yuvDiffMenu->addSeparator();
  yuvDiffMenu->addAction(yuvHasHiddenAction);
  yuvDiffMenu->addAction(yuvCropAction);
  yuvDiffMenu->addAction(yuvCheckForChangesAction);
  yuvDiffMenu->addAction(yuvSetOffsetAction);

  decodedYuvAction = new QAction(tr("Dec"), mainToolbar);
  decodedYuvAction->setCheckable(true);
  decodedYuvAction->setChecked(true);
  decodedYuvAction->setToolTip(tr("Show final decoded YUV"));
  debugYuvAction = new QAction(tr("Debug"), mainToolbar);
  debugYuvAction->setCheckable(true);
  debugYuvAction->setToolTip(tr("Show loaded YUV file to debug"));
  diffYuvAction = new QAction(tr("Diff"), mainToolbar);
  diffYuvAction->setCheckable(true);
  diffYuvAction->setToolTip(tr("Show subtraction image between decoded and debug"));
  QActionGroup *yuvModeActionGroup = new QActionGroup(mainToolbar);
  yuvModeActionGroup->addAction(decodedYuvAction);
  yuvModeActionGroup->addAction(debugYuvAction);
  yuvModeActionGroup->addAction(diffYuvAction);
  decodedYuvAction->setEnabled(false);
  debugYuvAction->setEnabled(false);
  diffYuvAction->setEnabled(false);

  fullscreenAction = new QAction("FS", mainToolbar);
  fullscreenAction->setToolTip(tr("Hide all panels except main"));
  fullscreenAction->setCheckable(true);
  connect(fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(setFullscreen(bool)));
  resetZoomAction = new QAction("R", mainToolbar);
  resetZoomAction->setToolTip(tr("Reset zoom (zoom to fit)"));
  zoomInAction = new QAction("+", mainToolbar);
  zoomInAction->setToolTip(tr("Zoom in"));
  zoomOutAction = new QAction("-", mainToolbar);
  zoomOutAction->setToolTip(tr("Zoom out"));
  overlayAction = new QAction("Info", mainToolbar);
  overlayAction->setToolTip(tr("Toggle annotations"));
  overlayAction->setCheckable(true);
  overlayAction->setChecked(true);
  viewNoneAction = new QAction("No img", mainToolbar);
  viewNoneAction->setCheckable(true);
  viewYUVAction = new QAction("YUV", mainToolbar);
  viewYUVAction->setCheckable(true);
  viewYUVAction->setChecked(true);
  viewYAction = new QAction("Y", mainToolbar);
  viewYAction->setCheckable(true);
  viewUAction = new QAction("U", mainToolbar);
  viewUAction->setCheckable(true);
  viewVAction = new QAction("V", mainToolbar);
  viewVAction->setCheckable(true);
  QActionGroup *viewModeActionGroup = new QActionGroup(mainToolbar);
  viewModeActionGroup->addAction(viewNoneAction);
  viewModeActionGroup->addAction(viewYUVAction);
  viewModeActionGroup->addAction(viewYAction);
  viewModeActionGroup->addAction(viewUAction);
  viewModeActionGroup->addAction(viewVAction);

  hexAction = new QAction("H", mainToolbar);
  hexAction->setToolTip(tr("Hex values"));
  hexAction->setCheckable(true);
  hexAction->setChecked(true);
  decAction = new QAction("D", mainToolbar);
  decAction->setCheckable(true);
  decAction->setToolTip(tr("Decimal values"));
  QActionGroup *radixActionGroup = new QActionGroup(mainToolbar);
  radixActionGroup->addAction(hexAction);
  radixActionGroup->addAction(decAction);

  mainToolbar->addAction(openFileMenu->menuAction());
  mainToolbar->addSeparator();
  mainToolbar->addAction(thumbsAction);
  mainToolbar->addAction(barsAction);
  mainToolbar->addWidget(new QLabel("Frame:", mainToolbar));
  mainToolbar->addWidget(frameSpinBox);
  mainToolbar->addAction(extractAction);
  mainToolbar->addSeparator();
  for(int i=0;i<NUM_MODES;i++)
    mainToolbar->addAction(modeActions[i]);
  mainToolbar->addSeparator();
  mainToolbar->addAction(yuvDiffMenu->menuAction());
  mainToolbar->addAction(decodedYuvAction);
  mainToolbar->addAction(debugYuvAction);
  mainToolbar->addAction(diffYuvAction);
  mainToolbar->addSeparator();
  mainToolbar->addAction(fullscreenAction);
  mainToolbar->addAction(resetZoomAction);
  mainToolbar->addAction(zoomInAction);
  mainToolbar->addAction(zoomOutAction);
  mainToolbar->addSeparator();
  mainToolbar->addAction(overlayAction);
  mainToolbar->addAction(viewNoneAction);
  mainToolbar->addAction(viewYUVAction);
  mainToolbar->addAction(viewYAction);
  mainToolbar->addAction(viewUAction);
  mainToolbar->addAction(viewVAction);
  mainToolbar->addSeparator();
  mainToolbar->addAction(hexAction);
  mainToolbar->addAction(decAction);



  addToolBar(Qt::TopToolBarArea, mainToolbar);

}

void MainWindow::setFullscreen(bool fs) {
  // Don't duplicate work
  if(fs != fullscreenAction->isChecked())
    return;
  if(fs) {
    savedSplitSizesTop << topSplit->sizes();
    savedSplitSizesMain << mainSplit->sizes();
    savedSplitSizesVideoBytes << videoBytesSplit->sizes();

    int w = geometry().width();
    int h = geometry().height();
    topSplit->setSizes(QList<int>() << 0 << h);
    mainSplit->setSizes(QList<int>() << w << 0 << 0);
    videoBytesSplit->setSizes(QList<int>() << h << 0);
  } else {
    topSplit->setSizes(savedSplitSizesTop);
    mainSplit->setSizes(savedSplitSizesMain);
    videoBytesSplit->setSizes(savedSplitSizesVideoBytes);
    savedSplitSizesTop.clear();
    savedSplitSizesMain.clear();
    savedSplitSizesVideoBytes.clear();
  }
}

void MainWindow::loadFile() {
  QString fName;
  QAction *action = qobject_cast<QAction *>(sender());
  if(action && action->data().toString().size() > 0) {
    fName = action->data().toString();
  } else {
    fName = QFileDialog::getOpenFileName(this,
                                         tr("Load bitstream"),
                                         pwd,
                                         tr(""));
    // User canceled:
    if(fName.size() == 0)
      return;
  }
  // Old filter: "VP9 Bitstream files (*.bit *.bin *.vp9 *.mkv *.webm);; All types"

  //qDebug() << QApplication::applicationDirPath();
  QString shortName = fName.section('/', -1, -1);
  pwd = fName.section('/', 0, -2);

  if(bs != NULL)
    delete bs;
  bs = new Bitstream(fName.toLocal8Bit().constData());
  if(!bs->isValid()) {
    QMessageBox::warning(this, tr("Problem"), QString(tr("Sorry, %1 couldn't be opened.")).arg(shortName), QMessageBox::Ok);
    delete bs;
    return;
  }

  // Update the recent files list
  QSettings settings;
  // Obtain the list from persistent storage
  QStringList recentList = settings.value("recentfiles").toStringList();
  int idx = -1;
  // Look for the new file in the existing list.
  for(int i = 0; i < recentList.size(); i++) {
    if(recentList[i].compare(fName) == 0) {
      idx = i;
      break;
    }
  }
  // Remove the new file from the list if it was already in it somewhere
  if(idx > 0) {
    recentList.removeAt(idx);
  }
  recentList.prepend(fName);
  // trim to the correct size
  while(recentList.size() > MAX_RECENT)
    recentList.removeLast();

  // Save the updated list.
  settings.setValue("recentfiles", recentList);

  updateRecentFiles();

  // TODO - load bitstream, populate UI. Oh that's all?


}

void MainWindow::updateRecentFiles() {
  QSettings settings;
  QStringList recentList = settings.value("recentfiles").toStringList();

  // Update all the actions
  for(int i = 0; i < recentList.size(); i++) {
    QString menuName = recentList[i].right(30);
    if(recentList[i].size() > 30)
      menuName.prepend("...");
    recentFilesActions[i]->setData(recentList[i]);
    recentFilesActions[i]->setText(menuName);
    recentFilesActions[i]->setVisible(true);
  }
  // Hide any that exist beyond the list.
  for(int i = recentList.size(); i < MAX_RECENT; i++)
    recentFilesActions[i]->setVisible(false);

  // Remove the placeholder message just in case.
  if(recentList.size()) {
    openFileMenu->removeAction(dummyRecentAction);
    clearRecentAction->setEnabled(true);
  }
}

void MainWindow::clearRecentFiles() {
  QSettings settings;
  settings.setValue("recentfiles", QVariant());
  for(int i = 0; i < MAX_RECENT; i++)
    recentFilesActions[i]->setVisible(false);
  openFileMenu->addAction(dummyRecentAction);
  clearRecentAction->setEnabled(false);
}

void MainWindow::setMode(VideoInfoMode mode, bool updateToolbar) {
  this->mode = mode;
  const char *componentNames[] = {"Luma", "Chroma Cb", "Chroma Cr"};
  const char *resTypeNames[] = {"coeffs", "dequant", "residual"};
  int idx = samplesComboBox->currentIndex();
  if(idx < 0)
    idx = 0;
  samplesComboBox->clear();
  for(int i = 0; i < (mode == MODE_RES ? 9 : 3); i++) {
    if(mode == MODE_RES)
      samplesComboBox->addItem(QString("%1 %2").arg(componentNames[i % 3]).arg(resTypeNames[i / 3]));
    else
      samplesComboBox->addItem(componentNames[i % 3]);
  }
  if(mode != MODE_RES)
    samplesComboBox->setCurrentIndex(idx % 3);
  else
    samplesComboBox->setCurrentIndex(idx);
  if(updateToolbar) {
    modeActions[mode]->setChecked(true);
  }

  // TODO, tell the rest of the UI about the new mode
}

void MainWindow::setMode(QAction *action) {
  setMode((VideoInfoMode)(action->data().toInt()));
}

