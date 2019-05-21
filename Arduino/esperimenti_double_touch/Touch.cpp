#include "Touch.h"

void Touch::addIndex(int _index) {
  if( touchStatus == EMPTY) {
    newIdx = _index;
    touchStatus = SINGLETOUCH;   
  }
  else if( touchStatus == SINGLETOUCH  ) {
    // check if the incoming index is adjacent
    if( _index == newIdx-1 || _index == newIdx+1) {
      oldIdx = newIdx;
      newIdx = _index;
      touchStatus = DOUBLETOUCH;
    }
    else {
      newIdx = _index;
      // status remains the same
      touchStatus = SINGLETOUCH; // redundant
    }
  } else if( touchStatus == DOUBLETOUCH) {
    // if we are here it means we have a third consecutive touch without a release.
    
    // check if the incoming index is to the left of minimumum between new and 
    // old indexes or if it is to the right of maximum between the two

    if( _index == min(newIdx, oldIdx)-1 || _index == max(newIdx, oldIdx)+1) {
      // if so, exchange indexes (discard the oldest index, always keep only two indexes)
      oldIdx = newIdx;
      newIdx = _index;
      // status remains the same
      touchStatus = DOUBLETOUCH; // redundant
    } else {
      // if we are here it means that the third touch is elsewhere
      // so we will treat it as a brand new single touch
      newIdx = _index;
      oldIdx = -1;
      touchStatus = SINGLETOUCH;     
    }
  }
}


void Touch::removeIndex(int _index) {
  // it can happen this method to be called even if
  // the index to be removed isn't contained inside
  // the touch object because it only keeps track of single
  // touches or a couple of adjacent ones.
  // So do nothing in that cases.
  if( _index == oldIdx ) {
    oldIdx = -1;
    touchStatus = SINGLETOUCH;
  } else if (_index == newIdx ) {
    newIdx = oldIdx;
    oldIdx = -1;
    touchStatus = SINGLETOUCH;
  } else {
    // do nothing
  }

  // check if we have removed both the collected index
  if( oldIdx == -1 && newIdx == -1 ) {
    touchStatus = EMPTY;
  }

}

int Touch::getInterpolatedIndex() {
  if( touchStatus == SINGLETOUCH ) {
    return newIdx * 2;
  } else if( touchStatus == DOUBLETOUCH ) {
    return newIdx+oldIdx;
  } else if( touchStatus == EMPTY ){
    //Serial.println("EMPTY");
    // this should never happen
    return -1;
  } 
}
