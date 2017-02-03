ISR(TIMER1_COMPA_vect){  //timer1A中段時程式碼
  if(powerOnoff == 0){
    digitalWrite(3,0);
  }
  else{
    digitalWrite(3,1);
  }
}

ISR(TIMER1_COMPB_vect){  //timer1B中段時程式碼
  digitalWrite(3,0);
}

void initTimer(){//初始化timer
    cli();
    TCCR1A = 0x00;
    TCCR1B = (1<<WGM12);   //CTC mode
    
    TCCR1B &= ~_BV(CS12);  // Timer1 1/8 
    TCCR1B |= _BV(CS11);
    TCCR1B &= ~_BV(CS10);
    
    /*
    TCCR1B &= ~_BV(CS12);
    TCCR1B &= ~_BV(CS11);
    TCCR1B &= ~_BV(CS10);
    */
//    TIMSK1 &= ~_BV(OCIE1A); //Timer1A關閉
//    TIMSK1 &= ~_BV(OCIE1B); //Timer1B關閉
    TCNT1=0;
    OCR1A = 16667;
    OCR1B = 16670;
    TIMSK1 |= _BV(OCIE1A);
    TIMSK1 |= _BV(OCIE1B);
    sei();
}
