/*
 * action.c
 *
 *  Created on: 30 de abr. de 2016
 *      Author: miguelsanchez
 */

#include <stdlib.h>  // para el NULL
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include <signal.h>
#include <time.h>

#define BUTTON_START 20 
#define LED_START 16

#define BUTTON_1 19
#define LED_1 26

#define BUTTON_2 6
#define LED_2 13

#define BUTTON_3 11
#define LED_3 5

#define BUTTON_4 27
#define LED_4 22

//#define NBUTTONS 4


#define FLAG_BUTTON_1 0x01
#define FLAG_BUTTON_2 0x02
#define FLAG_BUTTON_3 0x04
#define FLAG_BUTTON_4 0x08
#define FLAG_BUTTON_START 0x16
#define FLAG_TIMER	0x32

#define btn_fail_max 5

/*#define buttonsArray [] = {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, 100};*/
#define ledsArray [] = {LED_1, LED_2, LED_3, LED_4};

long long int penalty_time= 3;


int button_start =0;

int ledOn =0;
long int timeSeg=0;
long int timeNseg=0;
long long int game_time=0;
int TIMEOUT= 5*1000;
int fallos=0;
int round=0;


//digitalWrite (LED_START, 1);

int flags = 0;
void button_1_isr (void) { flags |= FLAG_BUTTON_1; }
void button_2_isr (void) { flags |= FLAG_BUTTON_2; }
void button_3_isr (void) { flags |= FLAG_BUTTON_3; }
void button_4_isr (void) { flags |= FLAG_BUTTON_4; }
void button_start_isr (void) { flags |= FLAG_BUTTON_START; printf("pulse boton start"); fflush(stdout);}


void timer_isr (union sigval value) { flags |= FLAG_TIMER; }
int timeout (fsm_t* this) { return (flags & FLAG_TIMER); }

int button1_pushed (fsm_t* this) { return (flags & FLAG_BUTTON_1); }
int button2_pushed (fsm_t* this) { return (flags & FLAG_BUTTON_2); }
int button3_pushed (fsm_t* this) { return (flags & FLAG_BUTTON_3); }
int button4_pushed (fsm_t* this) { return (flags & FLAG_BUTTON_4); }

int EVENT_BTN_START_END(fsm_t* this) { return (flags & FLAG_BUTTON_START); printf("pulse boton 1"); fflush(stdout);  }


void led_On(fsm_t* this, int led){
	digitalWrite(led,1);
	ledOn = led;
}


void randomLedOn(fsm_t* this){
	int randomN = rand() % 4; // Genero numero aleatorio
	round++;
	if(randomN == 0){    //enciendo el led
		led_On(this, LED_1);
		printf("led1"); fflush(stdout);
	} else if(randomN ==1){
		led_On(this, LED_2);
		printf("led2"); fflush(stdout);
	} else if(randomN ==2){
		led_On(this, LED_3);
		printf("led3"); fflush(stdout);
	} else if(randomN ==3){
		led_On(this, LED_4);
		printf("led4"); fflush(stdout);
	}
	tmr_startms((tmr_t*)(this->user_data), TIMEOUT);
	flags=0;
}

void startGame(fsm_t* this){
	digitalWrite (LED_START, 0);
	game_time=0;
	randomLedOn(this);
	round=0;
	fallos=0;
	flags=0;
	ledOn =0;

}
void gameOver(fsm_t* this,tmr_t* this1){
	digitalWrite (LED_START, 1);
	digitalWrite (LED_1, 0);
	digitalWrite (LED_2, 0);
	digitalWrite (LED_3, 0);
	digitalWrite (LED_4, 0);

}


void turnOff(fsm_t* this){
	flags &= (~FLAG_TIMER);
	digitalWrite (LED_1, 0);
	digitalWrite (LED_2, 0);
	digitalWrite (LED_3, 0);
	digitalWrite (LED_4, 0);
	digitalWrite (LED_START, 0);
}

int time_out (fsm_t* this) { return (flags & FLAG_TIMER); }

int readTime(tmr_t* this){
 	timer_gettime (this->timerid, &(this->spec));
 	game_time= ((TIMEOUT/1000  - this->spec.it_value.tv_sec)*1000000000) +
 			(1000000000 -this->spec.it_value.tv_nsec);
    return game_time;

}

int EVENT_BTN_OK(fsm_t* this,tmr_t* this1){ //METODO QUE INDICA SI SE APRETO EL BOTON CORRECTO
	printf("BNT_OK"); fflush(stdout);
	if(flags & 0x31){  //compruebo si pulse algun boton
	if(ledOn == LED_1){ if(flags & FLAG_BUTTON_1){
		readTime(this1);
		return (flags & FLAG_BUTTON_1);}
	else return (flags & FLAG_BUTTON_1);
	}
	if(ledOn == LED_2){if(flags & FLAG_BUTTON_2){
		readTime(this1);
		return (flags & FLAG_BUTTON_2);}
	else return (flags & FLAG_BUTTON_2);
	}
	if(ledOn == LED_3){if(flags & FLAG_BUTTON_3){
		readTime(this1);
		return (flags & FLAG_BUTTON_3);}
	else return (flags & FLAG_BUTTON_3);
	}
	if(ledOn == LED_4){if(flags & FLAG_BUTTON_4){
		readTime(this1);
		return (flags & FLAG_BUTTON_4);}
	else return (flags & FLAG_BUTTON_4);
	}
	printf("BNT_OK"); fflush(stdout);
	}
	return 0;
}



int EVENT_BTN_FAIL(fsm_t* this,tmr_t* this1){
	fallos++;
	game_time+= penalty_time * 1000000000;
	return !EVENT_BTN_FAIL_OK(this,this1) || time_out(this);


}

int EVENT_END_GAME(fsm_t* this,tmr_t* this1){
	if(round>=5 || fallos >= btn_fail_max){
		return 1;
	}
	return 0;
}

void printData(fsm_t* this, tmr_t* this1){
	printf("tiempo queda = %lld\n", game_time);
	  //  fflush(stdout); // Will now print everything in the stout buffer
}


int main(){
	//Lista de transiciones
	// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
	tmr_t* tmr = tmr_new (timer_isr);
	fsm_trans_t actionList[] = {
			{WAIT_START, EVENT_BTN_START_END, WAIT_PUSH, startGame},
			{WAIT_PUSH, EVENT_BTN_OK, WAIT_PUSH, randomLedOn},
			{WAIT_PUSH, EVENT_BTN_FAIL, WAIT_PUSH, randomLedOn},
			{WAIT_PUSH, EVENT_END_GAME, WAIT_END, gameOver},
			{WAIT_END, EVENT_BTN_START_END, WAIT_START,printData},
			{-1, NULL, -1,NULL},
	};
	fsm_t* machine = fsm_new(WAIT_START,actionList,tmr);
	//unsigned int next;

	 wiringPiSetupGpio();
	 pinMode (LED_START, OUTPUT);
	 digitalWrite(LED_START, 1);
	 pinMode (LED_1, OUTPUT);
	 digitalWrite(LED_1, 0);
	 pinMode (LED_2, OUTPUT);
	 digitalWrite(LED_2, 0);
	 pinMode (LED_2, OUTPUT);
	 digitalWrite(LED_2, 0);
	 pinMode (LED_3, OUTPUT);
	 digitalWrite(LED_3, 0);
	 pinMode (LED_4, OUTPUT);
	 digitalWrite(LED_4, 0);
	 pinMode (BUTTON_START, INPUT);
	 pinMode (BUTTON_1, INPUT);
	 pinMode (BUTTON_2, INPUT);
	 pinMode (BUTTON_3, INPUT);
	 pinMode (BUTTON_4, INPUT);

	 wiringPiISR (BUTTON_1, INT_EDGE_RISING, button_1_isr);
	 wiringPiISR (BUTTON_2, INT_EDGE_RISING, button_2_isr);
	 wiringPiISR (BUTTON_3, INT_EDGE_RISING, button_3_isr);
	 wiringPiISR (BUTTON_4, INT_EDGE_RISING, button_4_isr);
	 wiringPiISR (BUTTON_START, INT_EDGE_RISING, button_start_isr);

//	 apagar (interruptor_tmr_fsm);
	// next = millis();
	 //unsigned int next = millis();
	 while (1) {
	 fsm_fire (machine);
	 }
	 tmr_destroy ((tmr_t*)(machine->user_data));
	 fsm_destroy (machine);
	 return 0;
	}



