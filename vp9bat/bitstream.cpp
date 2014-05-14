#include "bitstream.h"

Bitstream::Bitstream(const char *name) {
  file = fopen(name,"rb");
  if(file) {
    fseek(file,0,SEEK_END);
    fileSize = ftell(file);
    fseek(file,0,SEEK_SET);
  } else {
    fileSize = -1;
  }
  currentFrameIdx = 0;
}

Bitstream::~Bitstream() {
  if(file)
    fclose(file);

  for(int i=0;i<frameInfo.size();i++)
    delete frameInfo[i];
  frameInfo.clear();
}

bool Bitstream::isValid() {
  return fileSize > 0;

  // TODO - must also make sure file is actually a vp9 stream.
}


