/*******************************************************************************
*  skInfraredCOM - 赤外線通信を行う関数ライブラリー                            *
*                                                                              *
*    skInfraredCOM - ライブラリ関数を生成する時の初期化処理                    *
*    Send      - 赤外線ＬＥＤにデータを送信する処理                            *
*    Recive    - 赤外線ＬＥＤよりデータを受信する処理                          *
*                                                                              *
* ============================================================================ *
*   VERSION  DATE        BY             CHANGE/COMMENT                         *
* ---------------------------------------------------------------------------- *
*   1.00     2010-06-28  きむ茶工房     Create                                 *
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "arduino.h"
#include "skInfraredCOM.h"


#define ON_USEC    1690
#define OFF_USEC   560
#define STOP_USEC  1000
#define READ_USEC  1000
#define HIGH_USEC  560

int  SndPinNo;
int  RcvPinNo;

/*******************************************************************************
*  skInfraredCOM(snd_pin_no,rcv_pin_no)                                        *
*    赤外線通信を行う関数を生成する時の初期化処理                              *
*                                                                              *
*    snd_pin_no : 赤外線送信用の接続ピン番号を指定                             *
*    rcv_pin_no : 赤外線受信用の接続ピン番号を指定                             *
*                 使用しない機能のピン番号は－１を必ず指定する                 *
*                                                                              *
*******************************************************************************/
skInfraredCOM::skInfraredCOM(int snd_pin_no,int rcv_pin_no)
{

	// 送信用の初期化
	SndPinNo = -1;
	if (snd_pin_no != -1) {
		SndPinNo = snd_pin_no;
	}
	// 受信用の初期化
	RcvPinNo = -1;
	if (rcv_pin_no != -1) {
		RcvPinNo = rcv_pin_no;
		pinMode(RcvPinNo, INPUT);
	}
}
/*******************************************************************************
*  Send(toDeviceNo,KeyCode)                                                    *
*    赤外線ＬＥＤにデータを送信する処理                                        *
*    送信相手のデバイスコードで２５５を指定すると無条件で相手は受信する        *
*                                                                              *
*    toDeviceNo : 送信相手のデバイスコード(1-250:251-255は予約)                *
*    KeyCode    : 送信するキーコード(1-255)                                    *
*******************************************************************************/
void skInfraredCOM::Send(unsigned char toDeviceNo,unsigned char KeyCode)
{
	int i;
	
	// リーダ部を送る
	PalseHigh(500);		// 約5ms
	delayMicroseconds(READ_USEC);
	// 送信相手のデバイスコードを送る
	for (i = 0; i < 8; i++) {
		PalseHigh(56);	// 約560us
		if ((toDeviceNo >> i) & 0x1) delayMicroseconds(ON_USEC);
		else                         delayMicroseconds(OFF_USEC);
	}
	// キーデータ１を送る
	for (i = 0; i < 8; i++) {
		PalseHigh(56);
		if ((KeyCode >> i) & 0x01) delayMicroseconds(ON_USEC);
		else                       delayMicroseconds(OFF_USEC);
	}
	// キーデータ１を反転して送る
	for (i = 0; i < 8; i++) {
		PalseHigh(56);
		if ((KeyCode >> i) & 0x01) delayMicroseconds(OFF_USEC);
		else                       delayMicroseconds(ON_USEC);
	}
	// ストップデータを送る
	PalseHigh(56);
	delayMicroseconds(STOP_USEC);
}
//  PalseHigh() - ＨＩＧＨのパルスを作る
void skInfraredCOM::PalseHigh(int cnt)
{
	int i;

	tone(SndPinNo, 37900);// 37.9KHzのON開始
	for (i = 0; i < cnt; i++ ) {
		delayMicroseconds(10);
	}
	noTone(SndPinNo);     // 37.9KHzの終了
}
/*******************************************************************************
*  ans = Recive(MyDeviceNo)                                                    *
*    赤外線受信モジュールよりデータを受信する処理                              *
*    受信したデータは、送信時のデータをモジュールが反転して出力する            *
*                                                                              *
*    MyDeviceNo : 自分のデバイス番号(1-250)                                    *
*    ans        : 受信データが自分へのﾃﾞｰﾀなら受信キーコード(1-255)を返す      *
*                 受信データが自分以外なら０を返す                             *
*******************************************************************************/
int skInfraredCOM::Recive(unsigned char MyDeviceNo)
{
	char IRbit[26];			// 受信バッファ
	unsigned long t;
	int ans , i;

	ans = 0;
	t   = 0;
		// リーダ部のチェックを行う
	if (digitalRead(RcvPinNo) == LOW) {
		// 現在の時刻(us)を得る
		t = micros();
		while (digitalRead(RcvPinNo) == LOW && micros() - t < 4500) {
			// HIGH(ON)になるまで待つ
		}

		// LOW(OFF)の部分をはかる
		t = micros() - t;
	}
	// リーダ部有りなら処理する(4.5ms以上のLOWにて判断する)
	if (t >= 4500) {
		i = 0;
		while(digitalRead(RcvPinNo) == HIGH) {
			// ここまでがリーダ部(ON部分)読み飛ばす
		}
		// データ部の読み込み
		while (1) {
			t = micros();
			while(digitalRead(RcvPinNo) == LOW && micros() - t < 1000) {
				// OFF部分は読み飛ばす
			}

			t = micros();
			while(digitalRead(RcvPinNo) == HIGH && micros() - t < 1000) {
				// LOW(FF)になるまで待つ
			}

			t = micros() - t;					// HIGH(ON)部分の長さをはかる
			if (t >= 1000) {
				IRbit[i] = (char)0x31;	// ON部分が長い
			}
			else {
				IRbit[i] = (char)0x30;	// ON部分が短い
			}
			i++;
			if (i == 24) {
				break;				// ３バイト読込んだら終了
			}
		}
		// データ有りならチェックを行う
		if (i == 24) {
			ans = DataCheck(MyDeviceNo,IRbit);
		}
	}
	return( ans );
}
//  DataCheck() - 受信データのチェックを行う
//                デバイスコードが255受信ならMyDeviceNoはチェックせず
int skInfraredCOM::DataCheck(unsigned char MyDeviceNo,char *dt)
{
	int x1 , x2 , i;

	// デバイスコードのチェック
	x1 = 0;
	for (i = 0; i < 8; i++) {
		if (*dt++ == 0x31) bitSet(x1,i);
	}
	if ((x1 != 255) && (x1 != MyDeviceNo)) return(0);// 受信デバイスコードが自分のコードではない
	// キーデータ１のチェック
	x1 = 0;
	for (i = 0; i < 8; i++) {
		if (*dt++ == 0x31) bitSet(x1,i);
	}
	// キーデータ２のチェック(キーデータ１の反転データ)
	x2 = 0;
	for (i = 0; i < 8; i++) {
		if (*dt++ == 0x30) bitSet(x2,i);
	}
	if (x1 != x2) return(0);		// キーデータの１と２が異なった
	return(x1);					// チェック正常、キーデータを返す
}
