#include<atomic>
void dmbst();
void dmbsy();

std::atomic<long int> flag0 , flag1 ;
//long int var = 0;
std::atomic<long int> var;
void thread0 (){
      //lock
      /////
      flag0 = 1;
      dmbst();
      while ( flag1 == 1 );
      //CS
      
      var = var + 1;
      //unlock
      dmbsy();
      flag0= 0;
}

void thread1 (){
      while(1) {
            ///
            flag1 = 0;
            dmbst();
            if( flag0 == 1) continue;
            
            flag1 = 1;
            
            if( flag0 == 1) continue;
            else break;
      }
      //CS
      
      var = var + 1;
      //unlock
      dmbsy();
      flag1 = 0;
      //
}
