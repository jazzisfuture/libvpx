#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdio.h>
#include <QList>
#include <vp9bat.pb.h>

class Bitstream {

private:
  FILE *file;
  int currentFrameIdx;
  int currentSBRasterIdx;
  int fileSize;

  QList<PictureInfo*> frameInfo;
  VP9Syntax currentSBSyntax;

  void decodeFrame(int idx, bool fast, int storeSBSyntaxIdx = -1);

public:
  Bitstream(const char *name);
  ~Bitstream();

  bool isValid();

  PictureInfo *getFrameInfo(int idx) { return frameInfo[idx]; }



   void setFrame(int idx);
   void setSB(int x, int y);
   BlockInfo *getBlockInfo(int x, int y);

};

#endif // BITSTREAM_H
