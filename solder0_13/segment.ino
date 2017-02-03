void segmentSetpoint(int input){
  static int buff = 0;
//  input *= 10;
  if(buff != input){
//    lc.setDigit(0,3,input/1000,false);
    lc.setDigit(0,2,(input%1000)/100,false);
    lc.setDigit(0,1,(input%100)/10,false);
    lc.setDigit(0,0,input%10,false);
  }
  buff = input;
}

void segmentTemp(double input){
  static int buff = 0;
  int inputBuff = input*10;
  if(buff != inputBuff){
    lc.setDigit(0,7,inputBuff/1000,false);
    lc.setDigit(0,6,(inputBuff%1000)/100,false);
    lc.setDigit(0,5,(inputBuff%100)/10,true);
    lc.setDigit(0,4,inputBuff%10,false);
  }
  buff = inputBuff;
}
