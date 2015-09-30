/*******************************************************************************
*  skInfraredCOM.h - ＬＣＤにてデバッグモニターを行う関数のインクルードファイル*
*                                                                              *
* ============================================================================ *
*  VERSION DATE        BY                    CHANGE/COMMENT                    *
* ---------------------------------------------------------------------------- *
*  1.00    2011-07-07  きむ茶工房(きむしげ)  Create                            *
*******************************************************************************/
#ifndef skInfraredCOM_h
#define skInfraredCOM_h

#include "arduino.h"

/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class skInfraredCOM
{
	public:
				 skInfraredCOM(int snd_pin_no,int rcv_pin_no) ;
		int  RemoconSW() ;
		void Send(unsigned char toDeviceNo,unsigned char KeyCode) ;
		int  Recive(unsigned char MyDeviceNo) ;
	private:
		void PalseHigh(int cnt) ;
		int  DataCheck(unsigned char MyDeviceNo,char *dt) ;
		int  SndPinNo ;
		int  RcvPinNo ;
};
#endif
