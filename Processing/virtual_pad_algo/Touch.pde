class Touch
{
  final int EMPTY = 0;
  final int SINGLETOUCH = 1;
  final int DOUBLETOUCH = 2;
  final int TRIPLETOUCH = 3;

  int touchStatus = EMPTY;
  int idxL = -1; // left index
  int idxR = -1; // right index
  int idxM = -1; // middle index
  
  Touch() {}

  void addIndex(int _index) {
    if( touchStatus == EMPTY) {
      idxL = _index;
      touchStatus = SINGLETOUCH;
    }
    else if( touchStatus == SINGLETOUCH  ) {
      // the new index is adjacent to the one already touched
      if( _index == idxL-1 ) {
        idxR = idxL;
        idxL = _index;
        touchStatus = DOUBLETOUCH;
      } 
      else if (_index == idxL+1) {
        idxR = _index;
        touchStatus = DOUBLETOUCH;
      }
      else 
      {
        // the new index is not adjacent
        idxL = _index;
        // status remains the same
        touchStatus = SINGLETOUCH; // redundant
      }
    } else if( touchStatus == DOUBLETOUCH ) {
      // if we are here it means we have a third consecutive touch without a release.
      
      // check if the incoming index is to the left of minimumum between 1st and 2nd
      // indexes or if it is to the right of maximum between the two
      if( _index == idxL-1 ) {
        idxM = idxL;
        idxL = _index;
        touchStatus = TRIPLETOUCH;
      } else if ( _index == idxR+1) {
        idxM = idxR;
        idxR = _index;
        touchStatus = TRIPLETOUCH;
      } else {
        // if we are here it means that the third touch is elsewhere
        // so we will treat it as a brand new single touch
        idxL = _index;
        idxR = -1;
        touchStatus = SINGLETOUCH;     
      }
    } else if( touchStatus == TRIPLETOUCH ) {
      if( _index == idxL-1) {
        idxR = idxM;
        idxM = idxL;
        idxL = _index;
        touchStatus = TRIPLETOUCH; // redundant
      }
      else if (_index == idxR+1) {
        // if so, exchange indexes (discard the oldest index, always keep only two indexes)
        idxL = idxM;
        idxM = idxR;
        idxR = _index;
        touchStatus = TRIPLETOUCH; // redundant
      } 
      else 
      {
        // if we are here it means that the third touch is elsewhere
        // so we will treat it as a brand new single touch
        idxL = _index;
        idxR = -1;
        idxM = -1;
        touchStatus = SINGLETOUCH;     
      }
    }
    /*
    } else if( touchStatus == DOUBLETOUCH ) {
      // if we are here it means we have a third consecutive touch without a release.
      
      // check if the incoming index is to the left of minimumum between new and 
      // old indexes or if it is to the right of maximum between the two
  
      if( _index == min(idxL, idxR)-1 || _index == max(idxL, idxR)+1) {
        // if so, exchange indexes (discard the oldest index, always keep only two indexes)
        idxR = idxL;
        idxL = _index;
        // status remains the same
        touchStatus = DOUBLETOUCH; // redundant
      } else {
        // if we are here it means that the third touch is elsewhere
        // so we will treat it as a brand new single touch
        idxL = _index;
        idxR = -1;
        touchStatus = SINGLETOUCH;     
      }
    }
    */
  }
  
  
  void removeIndex(int _index) {
    // it can happen this method to be called even if
    // the index to be removed isn't contained inside
    // the touch object because it only keeps track of single
    // touches or a couple of adjacent ones.
    // So do nothing in that cases.
    if( touchStatus == SINGLETOUCH && _index == idxL)
    {
      idxL = -1;
      touchStatus = EMPTY;
    }
    else if ( touchStatus == DOUBLETOUCH )
    {
      if(_index == idxL) {
        idxL = idxR;
        idxR = -1;
        touchStatus = SINGLETOUCH;
      }
      else if (_index == idxR ) {
        idxR = -1;
        touchStatus = SINGLETOUCH;
      }
    }    
    else if( touchStatus == TRIPLETOUCH ) {
      if (_index == idxL) {
        idxL = idxM;
        idxM = -1;
        touchStatus = DOUBLETOUCH;
      } else if (_index == idxR) {
        idxR = idxM;
        idxM = -1;
        touchStatus = DOUBLETOUCH;
      } else if (_index == idxM) {
        // in a case like this one i deliberately
        // choose to maintain only the left index
        idxM = -1;
        idxR = -1;
        touchStatus = SINGLETOUCH;
      }
    }
  }
  
  int getInterpolatedIndex() {
    if( touchStatus == SINGLETOUCH ) {
      return idxL * 2;
    } 
    else if( touchStatus == DOUBLETOUCH ) {
      return idxL+idxR;
    } 
    else if( touchStatus == TRIPLETOUCH ) {
      return idxM * 2; // same as doing idxL + idxR;
    } 
    else if( touchStatus == EMPTY ){
      //Serial.println("EMPTY");
      // this should never happen
      return -1;
    } 
    return -1;
  }
}
